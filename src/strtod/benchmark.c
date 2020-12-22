/*
 * Copyright (c) 2018 YaoYuan <ibireme@gmail.com>.
 * Released under the MIT license (MIT).
 */

#include "benchmark_helper.h"
#include "yybench.h"
#include <inttypes.h>
#include <fenv.h>

#ifndef BENCHMARK_DATA_PATH
extern const char *benchmark_get_data_path(void);
#define BENCHMARK_DATA_PATH benchmark_get_data_path()
#endif

/*----------------------------------------------------------------------------*/
/**
 A function prototype to convert string to double.
 @param str C-string beginning with the representation of a floating-point number.
 @param len The string's length
 @param endptr Ending pointer after the numerical value, or point to `str` if failed.
 @return The double number, 0.0 if failed, +/-HUGE_VAL if overflow.
 */
typedef double (*strtod_func)(const char *str, size_t len, char **endptr);



/*----------------------------------------------------------------------------*/

/** Structure used to avoid type-based aliasing rule. */
typedef union {
    f64 f;
    u64 u;
} f64_uni;

/** Convert double to raw. */
static yy_inline u64 f64_to_u64_raw(f64 f) {
    f64_uni uni;
    uni.f = f;
    return uni.u;
}

/** Convert raw to double. */
static yy_inline f64 f64_from_u64_raw(u64 u) {
    f64_uni uni;
    uni.u = u;
    return uni.f;
}



/*----------------------------------------------------------------------------*/
static int func_count = 0;
static strtod_func func_arr[128];
static char *func_name_arr[128];
static int func_name_max = 0;


// returns the function name aligned (inner memory)
static char *get_name_aligned(const char *name, bool align_right) {
    static char buf[64];
    int len = (int)strlen(name);
    int space = func_name_max - len;
    if (align_right) {
        for (int i = 0; i < space; i++) buf[i] = ' ';
        memcpy(buf + space, name, len);
    } else {
        memcpy(buf, name, len);
        for (int i = 0; i < space; i++) buf[len + i] = ' ';
    }
    buf[func_name_max] = '\0';
    return buf;
}



static void strtod_func_verify_all(void) {
//    fesetround(FE_TOWARDZERO);
    
    for (int i = 1; i < func_count; i++) { // skip null func
        const char *name = func_name_arr[i];
        strtod_func func = func_arr[i];
        int func_ulp_err = 0;
        int func_pas_err = 0;
        int func_max_ulp = 0;
       
        
        printf("verify %s ", get_name_aligned(name, false));
        
        // read test data dir
        char data_path[YY_MAX_PATH];
        yy_path_combine(data_path, BENCHMARK_DATA_PATH, "data", NULL);
        
        int file_count = 0;
        char **file_names = yy_dir_read(data_path, &file_count);
        if (file_count == 0) {
            printf("cannot read test data!\n");
            return;
        }
        
        // read each test file
        for (int f = 0; f < file_count; f++) {
            char *file_name = file_names[f];
            if (!yy_str_has_prefix(file_name, "real_pass") &&
                !yy_str_has_prefix(file_name, "sint_") &&
                !yy_str_has_prefix(file_name, "uint_")) continue;
            char file_path[YY_MAX_PATH];
            yy_path_combine(file_path, data_path, file_name, NULL);
            yy_dat dat;
            if (!yy_dat_init_with_file(&dat, file_path)) {
                printf("cannot read test file: %s\n", file_path);
                return;
            }
            
            // read each line
            usize line_len;
            char *line;
            while ((line = yy_dat_copy_line(&dat, &line_len))) {
                if (line_len == 0 || line[0] == '#') continue;
                
                // read test string with google
                int processed = 0;
                f64 val1 = google_string_to_double(line, &processed);
                if (val1 == -0.0) val1 = 0.0;
                if (!processed) continue;
                
                // read test number with the func
                char *end;
                f64 val2 = func(line, strlen(line), &end);
                if (val2 == -0.0) val2 = 0.0;
                
                // get ulp diff
                u64 uval1 = f64_to_u64_raw(val1);
                u64 uval2 = f64_to_u64_raw(val2);
                
                i64 ulp = (i64)uval1 - (i64)uval2;
                if (ulp < 0) ulp = -ulp;
                bool suc = (int)(end - line) == processed;
                
                bool show_debug = false;
                if (!suc) {
                    if (show_debug) {
                        printf("    parse failed: %s\n", line);
                    }
                    func_pas_err++;
                    
                } else if (ulp) {
                    if (show_debug) {
                        char buf[32];
                        google_double_to_string(val1, buf);
                        printf("    expect: %s\n", buf);
                        google_double_to_string(val2, buf);
                        printf("    output: %s\n", buf);
                        printf("    ulp error: %d\n", (int)ulp);
                    }
                    
                    func_ulp_err++;
                    if ((int)ulp > func_max_ulp) func_max_ulp = (int)ulp;
                }
                
                free(line);
            }
            yy_dat_release(&dat);
        }
        yy_dir_free(file_names);
        
        if (!func_pas_err && !func_ulp_err) printf(" [OK]\n");
        else {
            if (!func_pas_err) {
                printf(" [OK]");
            } else {
                printf(" [FAIL:%d]", func_pas_err);
            }
            if (func_ulp_err) {
                printf(" [ulp_err_num:%d]", func_ulp_err);
                printf(" [max_ulp_err:%d]", func_max_ulp);
            }
            printf("\n");
        }
    }
}

/*----------------------------------------------------------------------------*/

#define NUM_STR_LEN_MAX 32

typedef void (*fill_func)(char *buf, int count);
typedef void (*fill_len_func)(char *buf, int count, int len);

static yy_inline f64 random_f64(void) {
    while (true) {
        u64 u = yy_random64();
        f64 f = f64_from_u64_raw(u);
        if (isfinite(f)) return f;
    };
}

static void fill_double(char *buf, int count) {
    for (int i = 0; i < count; i++) {
        char *s = buf + i * NUM_STR_LEN_MAX;
        f64 f = random_f64();
        google_double_to_string(f, s);
    }
}

static void fill_double_rnd_len(char *buf, int count) {
    for (int i = 0; i < count; i++) {
        char *s = buf + i * NUM_STR_LEN_MAX;
        f64 f = random_f64();
        google_double_to_string_prec(f, (int)yy_random32_range(1, 17), s);
    }
}

static void fill_double_fix_len(char *buf, int count, int len) {
    len = len < 1 ? 1 : len > 17 ? 17 : len;
    for (int i = 0; i < count; i++) {
        char *s = buf + i * NUM_STR_LEN_MAX;
        f64 f = random_f64();
        google_double_to_string_prec(f, len, s);
    }
}

static void fill_one_double_no_exp_len(char *s, int len) {
    int dot_pos = (int)yy_random32_range(0, len);
    if (dot_pos == 0) {
        *s++ = '0';
        *s++ = '.';
        *s++ = (char)yy_random32_range(1, 9) + '0';
        int l = len - 1;
        while(l--) *s++ = (char)yy_random32_range(0, 9) + '0';
    } else if (dot_pos == len) {
        *s++ = (char)yy_random32_range(1, 9) + '0';
        int l = len - 1;
        while(l--) *s++ = (char)yy_random32_range(0, 9) + '0';
        *s++ = '.';
        *s++ = '0';
    } else {
        *s++ = (char)yy_random32_range(1, 9) + '0';
        int l = len;
        while(l--) *s++ = (char)yy_random32_range(0, 9) + '0';
        *(s - dot_pos - 1) = '.';
    }
    *s = '\0';
}

static void fill_double_no_exp_rnd_len(char *buf, int count) {
    for (int i = 0; i < count; i++) {
        char *s = buf + i * NUM_STR_LEN_MAX;
        fill_one_double_no_exp_len(s, yy_random32_range(1, 17));
    }
}

static void fill_double_no_exp_fix_len(char *buf, int count, int len) {
    len = len < 1 ? 1 : len > 17 ? 17 : len;
    for (int i = 0; i < count; i++) {
        char *s = buf + i * NUM_STR_LEN_MAX;
        fill_one_double_no_exp_len(s, len);
    }
}

static void fill_nomalized(char *buf, int count) {
    for (int i = 0; i < count; i++) {
        char *s = buf + i * NUM_STR_LEN_MAX;
        
        f64 val = (f64)yy_random64() / (f64)UINT64_MAX;
        val = (yy_random32() & 1) ? -val : val;
        google_double_to_string(val, s);
    }
}

static void fill_nomalized_f32(char *buf, int count) {
    for (int i = 0; i < count; i++) {
        char *s = buf + i * NUM_STR_LEN_MAX;
        
        f64 val = (f64)yy_random64() / (f64)UINT64_MAX;
        val = (yy_random32() & 1) ? -val : val;
        google_double_to_string((f32)val, s);
    }
}

static void fill_nomalized_rnd_len(char *buf, int count) {
    for (int i = 0; i < count; i++) {
        char *s = buf + i * NUM_STR_LEN_MAX;
        char *c = s;
        *c++ = '0';
        *c++ = '.';
        int l = (int)yy_random32_range(1, 17) - 1;
        while(l--) *c++ = (char)yy_random32_range(0, 9) + '0';
        *c++ = (char)yy_random32_range(1, 9) + '0';
        *c = '\0';
    }
}


static void fill_nomalized_fix_len(char *buf, int count, int len) {
    for (int i = 0; i < count; i++) {
        char *s = buf + i * NUM_STR_LEN_MAX;
        char *c = s;
        *c++ = '0';
        *c++ = '.';
        int l = len - 1;
        while(l--) *c++ = (char)yy_random32_range(0, 9) + '0';
        *c++ = (char)yy_random32_range(1, 9) + '0';
        *c = '\0';
    }
}

static void fill_integer_rnd_len(char *buf, int count) {
    for (int i = 0; i < count; i++) {
        char *s = buf + i * NUM_STR_LEN_MAX;
        char *c = s;
        int l = (int)yy_random32_range(1, 17) - 1;
        *c++ = (char)yy_random32_range(1, 9) + '0';
        while(l--) *c++ = (char)yy_random32_range(0, 9) + '0';
        *c++ = '.';
        *c++ = '0';
        *c = '\0';
    }
}

static void fill_integer_fix_len(char *buf, int count, int len) {
    for (int i = 0; i < count; i++) {
        char *s = buf + i * NUM_STR_LEN_MAX;
        char *c = s;
        int l = len - 1;
        *c++ = (char)yy_random32_range(1, 9) + '0';
        while(l--) *c++ = (char)yy_random32_range(0, 9) + '0';
        *c++ = '.';
        *c++ = '0';
        *c = '\0';
    }
}


static void strtod_func_benchmark_all(const char *output_path) {
    printf("initialize...\n");
    yy_cpu_setup_priority();
    yy_cpu_spin(0.5);
    yy_cpu_measure_freq();
    
    int num_per_case = 5000;
    int meansure_count = 16;
    
    typedef struct {
        const char *name;
        const char *desc;
        void *fill_func;
        bool has_len;
        int len_lo, len_hi;
    } dataset_t;
    
    dataset_t dataset_arr[64];
    int dataset_num = 0;
    
    dataset_arr[dataset_num++] = (dataset_t) {
        "random",
        "random double number in all binary range, ignore nan and inf",
        fill_double, false
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "random length",
        "random significant digit count (1-17)",
        fill_double_rnd_len, false
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "fixed length",
        "fixed significant digit count",
        fill_double_fix_len, true, 1, 17
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "random length without exponent",
        "random significant digit count (1-17)",
        fill_double_no_exp_rnd_len, false
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "fixed length without exponent",
        "fixed significant digit count",
        fill_double_no_exp_fix_len, true, 1, 17
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "random normalized",
        "random double number in range 0.0-1.0",
        fill_nomalized, false
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "random normalized (float)",
        "random float number in range 0.0-1.0",
        fill_nomalized_f32, false
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "random length normalized",
        "random significant digit count in range 0.0-1.0",
        fill_nomalized_rnd_len, false
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "fixed length normalized",
        "fixed significant digit count",
        fill_nomalized_fix_len, true, 1, 17
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "integer (random len)",
        "random significant digit count (1-17)",
        fill_integer_rnd_len, false
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "integer (fixed len)",
        "fixed significant digit count",
        fill_integer_fix_len, true, 1, 17
    };
    
    
    char *buf = malloc(num_per_case * NUM_STR_LEN_MAX);
    yy_report *report = yy_report_new();
    yy_report_add_env_info(report);
    
    for (int d = 0; d < dataset_num; d++) {
        dataset_t dataset = dataset_arr[d];
        
        printf("run benchmark %s...", dataset.name);
        
        yy_chart *chart = yy_chart_new();
        yy_chart_options op;
        yy_chart_options_init(&op);
        op.title = dataset.name;
        op.subtitle = dataset.desc;
        
        if (dataset.has_len) {
            op.type = YY_CHART_LINE;
            op.v_axis.title = "average CPU cycles";
            op.v_axis.logarithmic = true;
            op.h_axis.title = "digit count";
            op.h_axis.tick_interval = 1;
            op.plot.point_start = 1;
            op.tooltip.value_decimals = 2;
            op.tooltip.shared = true;
            op.tooltip.crosshairs = true;
            op.width = 800;
            op.height = 540;
            
            for (int f = 0; f < func_count; f++) {
                const char *func_name = func_name_arr[f];
                
                strtod_func func = func_arr[f];
                yy_chart_item_begin(chart, func_name);
                
                for (int len = dataset.len_lo; len <= dataset.len_hi; len++) {
                    yy_random_reset();
                    ((fill_len_func)dataset.fill_func)(buf, num_per_case, len);
                    
                    u64 ticks_min = UINT64_MAX;
                    for (int r = 0; r < meansure_count; r++) {
                        u64 t1 = yy_time_get_ticks();
                        for (int v = 0; v < num_per_case; v++) {
                            char *str = buf + v * NUM_STR_LEN_MAX;
                            func(str, NUM_STR_LEN_MAX, &str);
                        }
                        u64 t2 = yy_time_get_ticks();
                        u64 t = t2 - t1;
                        if (t < ticks_min) ticks_min = t;
                    }
                    f64 cycle = (f64)ticks_min / (f64)num_per_case * yy_cpu_get_cycle_per_tick();
                    yy_chart_item_add_float(chart, (f32)cycle);
                }
                
                yy_chart_item_end(chart);
            }
            
        } else {
            op.type = YY_CHART_BAR;
            op.h_axis.title = "CPU cycles";
            op.plot.value_labels_enabled = true;
            op.plot.value_labels_decimals = 2;
            op.plot.color_by_point = true;
            op.plot.group_padding = 0.0f;
            op.plot.point_padding = 0.1f;
            op.plot.border_width = 0.0f;
            op.legend.enabled = false;
            op.tooltip.value_decimals = 2;
            op.width = 640;
            op.height = 420;
            
            yy_random_reset();
            ((fill_func)dataset.fill_func)(buf, num_per_case);
            
            for (int f = 0; f < func_count; f++) {
                strtod_func func = func_arr[f];
                u64 ticks_min = UINT64_MAX;
                for (int r = 0; r < meansure_count; r++) {
                    u64 t1 = yy_time_get_ticks();
                    for (int v = 0; v < num_per_case; v++) {
                        char *str = buf + v * NUM_STR_LEN_MAX;
                        func(str, NUM_STR_LEN_MAX, &str);
                    }
                    u64 t2 = yy_time_get_ticks();
                    u64 t = t2 - t1;
                    if (t < ticks_min) ticks_min = t;
                }
                f64 cycle = (f64)ticks_min / (f64)num_per_case * yy_cpu_get_cycle_per_tick();
                yy_chart_item_with_float(chart, func_name_arr[f], (f32)cycle);
                
            }
        }
        yy_chart_sort_items_with_value(chart, false);
        yy_chart_set_options(chart, &op);
        yy_report_add_chart(report, chart);
        yy_chart_free(chart);
        
        printf("[OK]\n");
    }
    
    bool suc = yy_report_write_html_file(report, output_path);
    if (!suc) {
        printf("write report file failed: %s\n", output_path);
    }
    
    yy_report_free(report);
    free(buf);
}

static void strtod_func_cleanup(void) {
    func_count = 0;
    func_name_max = 0;
}

static void strtod_func_register_all(void) {
#define strtod_func_register(name) \
    extern double strtod_##name(const char *str, size_t len, char **endptr); \
    func_arr[func_count] = strtod_##name; \
    func_name_arr[func_count] = #name; \
    func_count++; \
    if ((int)strlen(#name) > func_name_max) func_name_max = (int)strlen(#name);
    
    strtod_func_register(null) /* used to meansure the benchmark overhead */
    strtod_func_register(libc)
    strtod_func_register(david_gay)
    strtod_func_register(google)
    strtod_func_register(abseil)
    strtod_func_register(lemire)
#ifndef _MSC_VER
    strtod_func_register(ryu)
#endif
    strtod_func_register(yy)
    strtod_func_register(yy_fast)
}

void benchmark(const char *output_path) {
    strtod_func_register_all();
    printf("------[verify]---------\n");
    strtod_func_verify_all();
    printf("------[benchmark]------\n");
    strtod_func_benchmark_all(output_path);
    printf("------[finish]---------\n");
    strtod_func_cleanup();
    return;
}
