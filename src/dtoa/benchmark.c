/*
 * Copyright (c) 2018 YaoYuan <ibireme@gmail.com>.
 * Released under the MIT license (MIT).
 */

#include "benchmark_helper.h"
#include "yy_test_utils.h"
#include <inttypes.h>

#ifndef BENCHMARK_DATA_PATH
extern const char *benchmark_get_data_path(void);
#define BENCHMARK_DATA_PATH benchmark_get_data_path()
#endif

/*----------------------------------------------------------------------------*/
/**
 A function prototype to convert double to string.
 Null-terminator is required (since we cannot change some lib's source).
 @param val A double number.
 @param buf A string buffer as least 32 bytes
 @return The ending of this string.
 */
typedef char *(*dtoa_func)(double val, char *buf);



/*----------------------------------------------------------------------------*/

typedef union {
    f64 f; u64 u;
} f64_uni;

typedef union {
    f32 f; u32 u;
} f32_uni;

static yy_inline u64 f64_to_u64_raw(f64 f) {
    f64_uni uni;
    uni.f = f;
    return uni.u;
}

static yy_inline f64 f64_from_u64_raw(u64 u) {
    f64_uni uni;
    uni.u = u;
    return uni.f;
}

static yy_inline f32 f32_from_u32_raw(u32 u) {
    f32_uni uni;
    uni.u = u;
    return uni.f;
}

/** Get the number of significant digit from a valid floating number string. */
static yy_inline int f64_str_get_digits(const char *str) {
    const char *cur = str, *dot = NULL, *hdr = NULL, *end = NULL;
    for (; *cur && *cur != 'e' && *cur != 'E' ; cur++) {
        if (*cur == '.') dot = cur;
        else if ('0' < *cur && *cur <= '9') {
            if (!hdr) hdr = cur;
            end = cur;
        }
    }
    if (!hdr) return 0;
    return (int)((end - hdr + 1) - (hdr < dot && dot < end));
}

/*----------------------------------------------------------------------------*/

typedef f64 (*fill_func)(void);
typedef f64 (*fill_len_func)(int len);

/** Get random double. */
static yy_inline f64 rand_f64(void) {
    while (true) {
        u64 u = yy_random64();
        f64 f = f64_from_u64_raw(u);
        if (isfinite(f)) return f;
    };
}

/** Get random float number as double. */
static yy_inline f64 rand_f64_from_f32(void) {
    while (true) {
        u32 u = yy_random32();
        f32 f = f32_from_u32_raw(u);
        if (isfinite(f)) return (f64)f;
    };
}

/** Get random positive double with specified number of digit (in range 1 to 17) */
static yy_inline f64 rand_f64_len(int len) {
    if (len < 1 || len > 17) return 0.0;
    char buf[32];
    int processed = 0;
    while (true) {
        // get random double
        u64 u = yy_random64();
        f64 f = f64_from_u64_raw(u);
        if (!isfinite(f)) continue;
        if (f < 0) f = -f;
        
        // print with precision
        google_double_to_string_prec(f, len, buf);
        f = google_string_to_double(buf, &processed);
        processed = google_double_to_string(f, buf);
        if (processed == 0 || f64_str_get_digits(buf) != len) continue;
        
        // read as double
        f = google_string_to_double(buf, &processed);
        if (!isfinite(f)) continue;
        if (processed == 0) continue;
        
        return f;
    }
}

/** Get random normalized double (in range 0.0 to 1.0). */
static yy_inline f64 rand_f64_normalize(void) {
    char buf[32];
    int processed = 0;
    
    // write a random double number in range 0.0 to 1.0
    buf[0] = '0';
    buf[1] = '.';
    yy_uint_to_string(yy_random64(), buf + 2);
    
    // read as double
    return google_string_to_double(buf, &processed);
}

/** Get random normalized double (in range 0.0 to 1.0)
    with specified number of digit (in range 1 to 17) */
static yy_inline f64 rand_f64_normalize_len(int len) {
    if (len < 1 || len > 17) return 0.0;
    char buf[32];
    int processed = 0;
    while (true) {
        // write a random double number in range 0.0 to 1.0
        buf[0] = '0';
        buf[1] = '.';
        yy_uint_to_string(yy_random64_uniform(100000000000000000ULL), buf + 2);
        buf[2 + len] = '\0';
        
        // read as double
        f64 f = google_string_to_double(buf, &processed);
        if (processed == 0) continue;
        
        // print to shortest string
        processed = google_double_to_string(f, buf);
        if (processed == 0) continue;
        if (f64_str_get_digits(buf) != len) continue;
        
        return f;
    }
}

/** Get random double which can convert to integer exactly. */
static yy_inline f64 rand_f64_integer(void) {
    char buf[32];
    int processed = 0;
    
    // write a random integer number which can fit in double
    yy_uint_to_string(yy_random64_range(1, (u64)1 << 53), buf);
    
    // read as double
    return google_string_to_double(buf, &processed);
}

/** Get random double which can convert to integer exactly,
    with specified number of digit (in range 1 to 17) */
static yy_inline f64 rand_f64_integer_len(int len) {
    if (len < 1 || len > 17) return 0.0;
    char buf[32];
    int processed = 0;
    while (true) {
        // write a random integer number with length
        yy_uint_to_string(yy_random64_range(100000000000000000ULL,
                                            999999999999999999ULL), buf);
        buf[len] = '\0';
        
        // read as double
        f64 f = google_string_to_double(buf, &processed);
        if (processed == 0) continue;
        
        // print to shortest string
        processed = google_double_to_string(f, buf);
        if (processed == 0) continue;
        if (f64_str_get_digits(buf) != len) continue;
        
        return f;
    }
}

/** Get random subnormal double. */
static yy_inline f64 rand_f64_subnormal(void) {
    u64 u = yy_random64_range(0x0000000000001ULL,
                              0xFFFFFFFFFFFFFULL);
    f64 f = f64_from_u64_raw(u);
    return f;
}

/*----------------------------------------------------------------------------*/
static int func_count = 0;
static dtoa_func func_arr[128];
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



/*----------------------------------------------------------------------------*/

static void dtoa_func_benchmark_all(const char *output_path) {
    printf("initialize...\n");
    yy_cpu_setup_priority();
    yy_cpu_spin(0.5);
    yy_cpu_measure_freq();
    
    
    int num_per_case = 10000;
    int  meansure_count = 16;
    
    typedef struct {
        const char *name;
        const char *desc;
        void *fill_func;
        bool func_has_len;
        bool use_random_len;
    } dataset_t;
    
    dataset_t dataset_arr[64];
    int dataset_num = 0;
    
    dataset_arr[dataset_num++] = (dataset_t) {
        "random",
        "random double number in all binary range, ignore nan and inf",
        rand_f64, false
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "random (random len)",
        "random double number with random significant digit count",
        rand_f64_len, true, true
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "random (fixed len)",
        "random double number with fixed significant digit count",
        rand_f64_len, true, false
    };
    
    
    dataset_arr[dataset_num++] = (dataset_t) {
        "nomalized",
        "random double number in range 0.0 to 1.0",
        rand_f64_normalize, false
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "nomalized (random len)",
        "random double number in range 0.0 to 1.0, with random significant digit count",
        rand_f64_normalize_len, true, true
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "nomalized (fixed len)",
        "random double number in range 0.0 to 1.0, with fixed significant digit count",
        rand_f64_normalize_len, true, false
    };
    
    dataset_arr[dataset_num++] = (dataset_t) {
        "integer",
        "random double number from integer",
        rand_f64_integer, false
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "integer (random len)",
        "random double number from integer, with random digit count",
        rand_f64_integer_len, true, true
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "integer (fixed len)",
        "random double number from integer, with fixed digit count",
        rand_f64_integer_len, true, false
    };
    
    dataset_arr[dataset_num++] = (dataset_t) {
        "subnormal",
        "random subnormal double number",
        rand_f64_subnormal, false
    };
    dataset_arr[dataset_num++] = (dataset_t) {
        "float",
        "random float number",
        rand_f64_from_f32, false
    };
    
    char buf[64];
    f64 *vals = malloc(num_per_case * sizeof(f64));
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
        
        if (dataset.func_has_len && !dataset.use_random_len) {
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
                dtoa_func func = func_arr[f];
                yy_chart_item_begin(chart, func_name);
                
                for (int len = 1; len <= 17; len++) {
                    yy_random_reset();
                    for (int i = 0; i < num_per_case; i++) {
                        vals[i] = ((fill_len_func)dataset.fill_func)(len);
                    }
                    
                    u64 ticks_min = UINT64_MAX;
                    for (int r = 0; r < meansure_count; r++) {
                        u64 t1 = yy_time_get_ticks();
                        for (int v = 0; v < num_per_case; v++) {
                            f64 val = vals[v];
                            func(val, buf);
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
            
        } else { // bar chart
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
            if (dataset.func_has_len) {
                for (int i = 0; i < num_per_case; i++) {
                    vals[i] = ((fill_len_func)dataset.fill_func)((int)yy_random32_range(1, 17));
                }
            } else {
                for (int i = 0; i < num_per_case; i++) {
                    vals[i] = ((fill_func)dataset.fill_func)();
                }
            }
            
            for (int f = 0; f < func_count; f++) {
                const char *func_name = func_name_arr[f];
                dtoa_func func = func_arr[f];
                u64 ticks_min = UINT64_MAX;
                for (int r = 0; r < meansure_count; r++) {
                    u64 t1 = yy_time_get_ticks();
                    for (int v = 0; v < num_per_case; v++) {
                        f64 val = vals[v];
                        func(val, buf);
                    }
                    u64 t2 = yy_time_get_ticks();
                    u64 t = t2 - t1;
                    if (t < ticks_min) ticks_min = t;
                }
                f64 cycle = (f64)ticks_min / (f64)num_per_case * yy_cpu_get_cycle_per_tick();
                yy_chart_item_with_float(chart, func_name, (f32)cycle);
                
            }
        }
        yy_chart_sort_items_with_value(chart, false);
        yy_chart_set_options(chart, &op);
        yy_report_add_chart(report, chart);
        yy_chart_free(chart);
        
        printf("[OK]\n");
    }
    
    // export report to html
    bool suc = yy_report_write_html_file(report, output_path);
    if (!suc) {
        printf("write report file failed: %s\n", output_path);
    }
    yy_report_free(report);
    free(vals);
}

static void dtoa_func_verify_all(void) {
    for (int i = 1; i < func_count; i++) { // skip null func
        const char *name = func_name_arr[i];
        dtoa_func func = func_arr[i];
        int func_ulp_err = 0;
        int func_len_err = 0;
        
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
                char str[64]; // some func (such as schubfach) need more space
                
                // read test string with google
                int processed = 0;
                f64 val1 = google_string_to_double(line, &processed);
                if (!processed) continue;
                if (!isfinite(val1)) continue;
                if (val1 == -0.0) val1 = 0.0; /* some func may write -0.0 as 0.0 */
                
                // write double to string with google to get shortest digit num
                google_double_to_string(val1, str);
                int min_dig_num1 = f64_str_get_digits(str);
                
                // write test number with dtoa func
                usize write_len = func(val1, str) - str;
                int min_dig_num2 = f64_str_get_digits(str);
                
                // read output string with google
                f64 val2 = google_string_to_double(str, &processed);
                
                // ensure the double value is same
                u64 uval1 = f64_to_u64_raw(val1);
                u64 uval2 = f64_to_u64_raw(val2);
                bool is_same = (uval1 == uval2) && ((int)write_len == processed);
                
                // ensure the string is shortest
                bool is_shortest = (min_dig_num1 == min_dig_num2);
                
                bool print_err = false;
                if (!is_same) {
                    if (print_err) {
                        printf("  func %s error:\n", name);
                        printf("    output: %s\n", str);
                        google_double_to_string(val1, str);
                        printf("    expect: %s\n", str);
                    }
                    func_ulp_err++;
                } else if (!is_shortest) {
                    
                    if (print_err) {
                        printf("  func %s error:\n", name);
                        printf("    output: %s\n", str);
                        google_double_to_string(val1, str);
                        printf("    expect: %s\n", str);
                    }
                    func_len_err++;
                }
                
                free(line);
            }
            yy_dat_release(&dat);
        }
        yy_dir_free(file_names);
        
        if (!func_len_err && !func_ulp_err) printf(" [OK]\n");
        else {
            if (func_len_err) printf(" [not_shortest:%d]", func_len_err);
            if (func_ulp_err) printf(" [not_match:%d]", func_ulp_err);
            printf("\n");
        }
    }
}

static void dtoa_func_cleanup(void) {
    func_count = 0;
    func_name_max = 0;
}



/*
 This benchmark is somewhat unfair, because different algorithm use different
 format to print floating point number.
 
 For example:
 david_gay:     1.1e-06     11
 google:        0.0000011   11
 swift:         1.1e-06     11.0
 fmt:           1.1e-06     11.0
 ryu:           1.1E-6      1.1E1
 dragonbox:     1.1E-6      1.1E1
 
 Maybe we should benchmark the "binary to decimal" and "decimal to string"
 separately for some algorithms.
 */
static void dtoa_func_register_all(void) {
#define dtoa_func_register(name) \
    extern char *dtoa_##name(double val, char *buf); \
    func_arr[func_count] = dtoa_##name; \
    func_name_arr[func_count] = #name; \
    func_count++; \
    if ((int)strlen(#name) > func_name_max) func_name_max = (int)strlen(#name);
    
    dtoa_func_register(null) /* used to meansure the benchmark overhead */
    dtoa_func_register(david_gay)
    dtoa_func_register(google)
    dtoa_func_register(swift)
    dtoa_func_register(fmtlib)
    dtoa_func_register(fpconv)
#ifndef _MSC_VER
    dtoa_func_register(milo)
    dtoa_func_register(emyg)
    dtoa_func_register(ryu)
#endif
    dtoa_func_register(ryu_yy)
    dtoa_func_register(grisu3)
    dtoa_func_register(schubfach)
    dtoa_func_register(erthink)
    dtoa_func_register(grisu_exact)
    dtoa_func_register(dragonbox)
    // dtoa_func_register(printf) // not shortest, too slow
}

void benchmark(const char *output_file_path) {
    dtoa_func_register_all();
    printf("------[verify]---------\n");
    dtoa_func_verify_all();
    printf("------[benchmark]------\n");
    dtoa_func_benchmark_all(output_file_path);
    printf("------[finish]---------\n");
    dtoa_func_cleanup();
    return;
}
