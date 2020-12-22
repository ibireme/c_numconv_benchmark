/*
 * Copyright (c) 2018 YaoYuan <ibireme@gmail.com>.
 * Released under the MIT license (MIT).
 */
#include "benchmark.h"
#include "yybench.h"
#include <inttypes.h>

#ifndef BENCHMARK_DATA_PATH
extern const char *benchmark_get_data_path(void);
#define BENCHMARK_DATA_PATH benchmark_get_data_path()
#endif

// use odd length to emulate unaligned input string
#define u32_max_buf_len 13
#define i32_max_buf_len 13
#define u64_max_buf_len 23
#define i64_max_buf_len 23


/*----------------------------------------------------------------------------*/

typedef struct {
    atoi_u32_func u32_func;
    atoi_i32_func i32_func;
    atoi_u64_func u64_func;
    atoi_i64_func i64_func;
    const char *name;
} atoi_group;

static int atoi_group_num = 0;
static atoi_group atoi_group_arr[128];
static int atoi_group_name_max = 0;

static char *atoi_group_get_name_aligned(atoi_group group, bool align_right) {
    static char buf[64];
    int len, space, i;
    len = (int)strlen(group.name);
    space = atoi_group_name_max - len;
    if (align_right) {
        for (i = 0; i < space; i++) buf[i] = ' ';
        memcpy(buf + space, group.name, len);
        buf[atoi_group_name_max] = '\0';
    } else {
        memcpy(buf, group.name, len);
        for (i = 0; i < space; i++) buf[len + i] = ' ';
        buf[atoi_group_name_max] = '\0';
    }
    return buf;
}

/*----------------------------------------------------------------------------*/

#define atoi_group_verify_type(type) \
static bool atoi_group_verify_##type(atoi_group group, const char *str,         \
                                     size_t _len, type _val, atoi_result _res) {\
    atoi_##type##_func func = group. type##_func;                              \
    type val;                                                                   \
    char *end;                                                                  \
    atoi_result res;                                                            \
                                                                                \
    val = func(str, strlen(str), &end, &res);                                   \
    if (res != _res) {                                                          \
        printf("atoi_%s_%s not match, input:%s, expect:%s, return:%s\n",        \
               #type, group.name, str, atoi_result_desc(_res), atoi_result_desc(res)); \
        return false;                                                           \
    }                                                                           \
    if (res != atoi_result_suc) return true;                                    \
    if (val != _val) {                                                          \
        printf("atoi_%s_%s not match, input:%s, expect:%" PRI##type ", return:%" PRI##type "\n", \
               #type, group.name, str, _val, val);                              \
        return false;                                                           \
    }                                                                           \
    if (str + _len != end) {                                                    \
        printf("atoi_%s_%s not match, input:%s, expect_len:%d, return_len:%d\n",\
               #type, group.name, str, (int)_len, (int)(end - str));            \
        return false;                                                           \
    }                                                                           \
    return true;                                                                \
}
atoi_group_verify_type(u32)
atoi_group_verify_type(i32)
atoi_group_verify_type(u64)
atoi_group_verify_type(i64)



static void atoi_group_verify_all(void) {
    for (int g = 1; g < atoi_group_num; g++) {
        atoi_group group = atoi_group_arr[g];
        bool suc = true;
        char buf[32];
        const char *str;
        size_t len;
        
#define verify_itype(_type, _str, _num, _ret)\
        str = _str; \
        len = strlen(str); \
        memcpy(buf, str, len + 1); \
        suc &= atoi_group_verify_##_type(group, buf, len, _num, atoi_result_##_ret); \
        buf[len + 1] = 'A'; \
        buf[len + 2] = '\0'; \
        suc &= atoi_group_verify_##_type(group, buf, len, _num, atoi_result_##_ret); \
        
#define verify_utype(_type, _str, _num, _ret) \
        str = _str; \
        len = strlen(str); \
        memcpy(buf, str, len + 1); \
        suc &= atoi_group_verify_##_type(group, buf, len, _num, atoi_result_##_ret); \
        buf[len + 1] = 'A'; \
        buf[len + 2] = '\0'; \
        suc &= atoi_group_verify_##_type(group, buf, len, _num, atoi_result_##_ret); \
        buf[0] = '-'; \
        memcpy(buf + 1, str, len + 1); \
        suc &= atoi_group_verify_##_type(group, buf, 0, 0, atoi_result_fail); \
        buf[len + 1] = 'A'; \
        buf[len + 2] = '\0'; \
        suc &= atoi_group_verify_##_type(group, buf, 0, 0, atoi_result_fail); \
        
        //  4294967295
        //  2147483647
        // -2147483648
        //  18446744073709551615
        //  9223372036854775807
        // -9223372036854775808
        
        verify_itype(i32, "0", 0, suc)
        verify_itype(i32, "-0", -0, suc)
        verify_itype(i32, "1", 1, suc)
        verify_itype(i32, "-1", -1, suc)
        verify_itype(i32, "999999999", 999999999, suc)
        verify_itype(i32, "-999999999", -999999999, suc)
        verify_itype(i32, "2147483647", 2147483647, suc)
        verify_itype(i32, "-2147483647", -2147483647, suc)
        verify_itype(i32, "2147483648", 0, overflow)
        verify_itype(i32, "-2147483648", -2147483648, suc)
        verify_itype(i32, "2147483649", 0, overflow)
        verify_itype(i32, "-2147483649", 0, overflow)
        verify_itype(i32, "21474836480", 0, overflow)
        verify_itype(i32, "-21474836480", 0, overflow)
        verify_itype(i32, "21474836470", 0, overflow)
        verify_itype(i32, "-21474836470", 0, overflow)
        verify_itype(i32, "10000000000", 0, overflow)
        verify_itype(i32, "-10000000000", 0, overflow)
        
        verify_itype(u32, "0", 0u, suc)
        verify_itype(u32, "1", 1u, suc)
        verify_itype(u32, "999999999", 999999999u, suc)
        verify_itype(u32, "4294967294", 4294967294u, suc)
        verify_itype(u32, "4294967295", 4294967295u, suc)
        verify_itype(u32, "4294967296", 0, overflow)
        verify_itype(u32, "42949672950", 0, overflow)
        verify_itype(u32, "42949672940", 0, overflow)
        verify_itype(u32, "10000000000", 0, overflow)
        
        verify_itype(i64, "0", 0, suc)
        verify_itype(i64, "-0", -0, suc)
        verify_itype(i64, "1", 1, suc)
        verify_itype(i64, "-1", -1, suc)
        verify_itype(i64, "999999999999999999", 999999999999999999, suc)
        verify_itype(i64, "-999999999999999999", -999999999999999999, suc)
        verify_itype(i64, "9223372036854775807", 9223372036854775807ll, suc)
        verify_itype(i64, "-9223372036854775807", -9223372036854775807ll, suc)
        verify_itype(i64, "9223372036854775808", 0, overflow)
        verify_itype(i64, "-9223372036854775807", -9223372036854775807ll, suc)
        verify_itype(i64, "9223372036854775809", 0, overflow)
        verify_itype(i64, "-9223372036854775809", 0, overflow)
        verify_itype(i64, "92233720368547758060", 0, overflow)
        verify_itype(i64, "-92233720368547758060", 0, overflow)
        verify_itype(i64, "92233720368547758070", 0, overflow)
        verify_itype(i64, "-92233720368547758070", 0, overflow)
        verify_itype(i64, "10000000000000000000", 0, overflow)
        verify_itype(i64, "-10000000000000000000", 0, overflow)
        
        verify_itype(u64, "0", 0, suc)
        verify_itype(u64, "1", 1, suc)
        verify_itype(u64, "9999999999999999999", 9999999999999999999ull, suc)
        verify_itype(u64, "18446744073709551615", 18446744073709551615ull, suc)
        verify_itype(u64, "18446744073709551616", 0, overflow)
        verify_itype(u64, "18446744073709551620", 0, overflow)
        verify_itype(u64, "28446744073709551615", 0, overflow)
        verify_itype(u64, "100000000000000000000", 0, overflow)
                
        suc &= atoi_group_verify_i32(group, "A", 0, 0, atoi_result_fail);
        suc &= atoi_group_verify_u32(group, "A", 0, 0, atoi_result_fail);
        suc &= atoi_group_verify_i64(group, "A", 0, 0, atoi_result_fail);
        suc &= atoi_group_verify_u64(group, "A", 0, 0, atoi_result_fail);

        suc &= atoi_group_verify_i32(group, "-", 0, 0, atoi_result_fail);
        suc &= atoi_group_verify_u32(group, "-", 0, 0, atoi_result_fail);
        suc &= atoi_group_verify_i64(group, "-", 0, 0, atoi_result_fail);
        suc &= atoi_group_verify_u64(group, "-", 0, 0, atoi_result_fail);
        
        suc &= atoi_group_verify_i32(group, "-A", 0, 0, atoi_result_fail);
        suc &= atoi_group_verify_u32(group, "-A", 0, 0, atoi_result_fail);
        suc &= atoi_group_verify_i64(group, "-A", 0, 0, atoi_result_fail);
        suc &= atoi_group_verify_u64(group, "-A", 0, 0, atoi_result_fail);
        
        suc &= atoi_group_verify_u32(group, "-1", 0, 0, atoi_result_fail);
        suc &= atoi_group_verify_u64(group, "-1", 0, 0, atoi_result_fail);
        
        if (suc) printf("    %s [OK]\n", atoi_group_get_name_aligned(group, false));
    }
}

/*----------------------------------------------------------------------------*/

static const u64 pow10_table[] = {
    1ULL,
    10ULL,
    100ULL,
    1000ULL,
    10000ULL,
    100000ULL,
    1000000ULL,
    10000000ULL,
    100000000ULL,
    1000000000ULL,
    10000000000ULL,
    100000000000ULL,
    1000000000000ULL,
    10000000000000ULL,
    100000000000000ULL,
    1000000000000000ULL,
    10000000000000000ULL,
    100000000000000000ULL,
    1000000000000000000ULL,
    10000000000000000000ULL
};

// len: [1, 10], out: [0, 4294967295]
static u32 rand_u32_len(u32 len) {
    len = len > 10 ? 10 : len < 1 ? 1 : len;
    u32 min = len == 1 ? 0 : (u32)pow10_table[len - 1];
    u32 max = len == 10 ? UINT32_MAX : (u32)pow10_table[len] - 1;
    return yy_random32_range(min, max);
}

// len: [1, 20], out: [0, 18446744073709551615]
static u64 rand_u64_len(u32 len) {
    len = len > 20 ? 20 : len < 1 ? 1 : len;
    u64 min = len == 1 ? 0 : pow10_table[len - 1];
    u64 max = len == 20 ? UINT64_MAX : pow10_table[len] - 1;
    return yy_random64_range(min, max);
}

// len: [1, 10], out: [0, ±2147483647]
static i32 rand_i32_len(u32 len, bool negative) {
    len = len > 10 ? 10 : len < 1 ? 1 : len;
    u32 min = len == 1 ? 0 : (u32)pow10_table[len - 1];
    u32 max = len == 10 ? INT32_MAX : (u32)pow10_table[len] - 1;
    return yy_random32_range(min, max) * (negative ? -1 : 1);
}

// len: [1, 19], out: [0, ±9223372036854775807]
static i64 rand_i64_len(u32 len, bool negative) {
    len = len > 19 ? 19 : len < 1 ? 1 : len;
    u64 min = len == 1 ? 0 : pow10_table[len - 1];
    u64 max = len == 19 ? INT64_MAX : pow10_table[len] - 1;
    return yy_random64_range(min, max) * (negative ? -1 : 1);
}

static void fill_rand_u32(char *buf, u32 count) {
    for (u32 i = 0; i < count; i++) {
        u32 v = rand_u32_len(yy_random32_range(1, 10));
        char *s = buf + i * u32_max_buf_len;
        snprintf(s, u32_max_buf_len, "%" PRIu32 "%c", v, '\0');
    }
}

static void fill_rand_u64(char *buf, u32 count) {
    for (u32 i = 0; i < count; i++) {
        u64 v = rand_u64_len(yy_random32_range(1, 20));
        char *s = buf + i * u32_max_buf_len;
        snprintf(s, u64_max_buf_len, "%" PRIu64 "%c", v, '\0');
    }
}

static void fill_rand_i32(char *buf, u32 count) {
    for (u32 i = 0; i < count; i++) {
        i32 v = rand_i32_len(yy_random32_range(1, 10), yy_random32() % 2);
        char *s = buf + i * i32_max_buf_len;
        snprintf(s, i32_max_buf_len, "%" PRIi32 "%c", v, '\0');
    }
}

static void fill_rand_i64(char *buf, u32 count) {
    for (u32 i = 0; i < count; i++) {
        i64 v = rand_i64_len(yy_random32_range(1, 19), yy_random32() % 2);
        char *s = buf + i * i64_max_buf_len;
        snprintf(s, i64_max_buf_len, "%" PRIi64 "%c", v, '\0');
    }
}

static void fill_rand_u32_len(char *buf, u32 count, u32 len) {
    len = len > 10 ? 10 : len;
    u32 min = (u32)pow10_table[len - 1];
    u32 max = len == 10 ? UINT32_MAX : (u32)pow10_table[len] - 1;
    double step = (max - min) / (double)count;
    for (u32 i = 0; i < count; i++) {
        u32 v = (u32)(min + i * step);
        char *s = buf + i * u32_max_buf_len;
        snprintf(s, u32_max_buf_len, "%" PRIu32 "%c", v, '\0');
    }
}

static void fill_rand_u64_len(char *buf, u32 count, u32 len) {
    len = len > 20 ? 20 : len;
    u64 min = pow10_table[len - 1];
    u64 max = len == 20 ? UINT64_MAX : pow10_table[len] - 1;
    double step = (max - min) / (double)count;
    for (u32 i = 0; i < count; i++) {
        u64 v = (u64)(min + i * step);
        char *s = buf + i * u64_max_buf_len;
        snprintf(s, u64_max_buf_len, "%" PRIu64 "%c", v, '\0');
    }
}

static void fill_rand_i32_len(char *buf, u32 count, u32 len) {
    len = len > 10 ? 10 : len;
    u32 min = (u32)pow10_table[len - 1];
    u32 max = len == 10 ? INT32_MAX : (u32)pow10_table[len] - 1;
    u32 half = count / 2;
    double step = (max - min) / (double)half;
    for (u32 i = 0; i < half; i++) {
        i32 v = (i32)(min + i * step);
        char *s = buf + i * i32_max_buf_len;
        snprintf(s, i32_max_buf_len, "%" PRIi32 "%c", v, '\0');
    }
    for (u32 i = half; i < count; i++) {
        i32 v = -(i32)(min + i * step);
        char *s = buf + i * i32_max_buf_len;
        snprintf(s, i32_max_buf_len, "%" PRIi32 "%c", v, '\0');
    }
}

static void fill_rand_i64_len(char *buf, u32 count, u32 len) {
    len = len > 19 ? 19 : len;
    u64 min = pow10_table[len - 1];
    u64 max = len == 19 ? INT64_MAX : (u64)pow10_table[len] - 1;
    u64 half = count / 2;
    double step = (max - min) / (double)half;
    for (u64 i = 0; i < half; i++) {
        i64 v = (i64)(min + i * step);
        char *s = buf + i * i64_max_buf_len;
        snprintf(s, i64_max_buf_len, "%" PRIi64 "%c", v, '\0');
    }
    for (u64 i = half; i < (u64)count; i++) {
        i64 v = -(i64)(min + i * step);
        char *s = buf + i * i64_max_buf_len;
        snprintf(s, i64_max_buf_len, "%" PRIi64 "%c", v, '\0');
    }
}



static void atoi_group_benchmark_all(const char *output_path) {
    
    static const int sample_count = 10000;
    static const int repeat_count = 16;
    
    yy_chart *charts[8], *chart;
    int chart_count = 8;
    u64 tsc_begin, tsc_end, tsc;
    f64 tsc_avg, tsc_avg_min, tsc_avg_sum, cycles;
    atoi_group group;
    u32 u32_out;
    i32 i32_out;
    u64 u64_out;
    i64 i64_out;
    atoi_result res;
    char *endptr;
    
    printf("prepare...\n");
    yy_cpu_setup_priority();
    yy_cpu_spin(0.5);
    yy_cpu_measure_freq();
    
    char *buf = malloc(sample_count * u64_max_buf_len);
    if (!buf) {
        printf("memory allocation failed.\n");
        return;
    }
    
    yy_report *report = yy_report_new();
    yy_report_add_env_info(report);
    
    // create charts
    for (int i = 0; i < chart_count; i++) {
        charts[i] = yy_chart_new();
        yy_chart_options op;
        yy_chart_options_init(&op);
        switch (i) {
            case 0: op.title = "atoi u32 (fixed length)"; break;
            case 1: op.title = "atoi u32 (random length)"; break;
            case 2: op.title = "atoi u64 (fixed length)"; break;
            case 3: op.title = "atoi u64 (random length)"; break;
            case 4: op.title = "atoi i32 (fixed length)"; break;
            case 5: op.title = "atoi i32 (random length)"; break;
            case 6: op.title = "atoi i64 (fixed length)"; break;
            case 7: op.title = "atoi i64 (random length)"; break;
            default: break;
        }
        if ((i % 2) == 0) { /* sequence (line chart) */
            op.type = YY_CHART_LINE;
            op.v_axis.title = "CPU cycles";
            op.v_axis.logarithmic = true;
            op.h_axis.title = "digit count";
            op.h_axis.tick_interval = 1;
            op.plot.point_start = 1;
            op.tooltip.value_decimals = 2;
            op.tooltip.shared = true;
            op.tooltip.crosshairs = true;
            op.width = 640;
            op.height = 400;
        } else { /* random (bar chart) */
            op.type = YY_CHART_BAR;
            op.h_axis.title = "average CPU cycles";
            op.plot.value_labels_enabled = true;
            op.plot.value_labels_decimals = 2;
            op.plot.color_by_point = true;
            op.plot.group_padding = 0.0f;
            op.plot.point_padding = 0.1f;
            op.plot.border_width = 0.0f;
            op.legend.enabled = false;
            op.tooltip.value_decimals = 2;
            op.width = 640;
            op.height = 300;
        }
        yy_chart_set_options(charts[i], &op);
    }
    
    
#define BENCHMARK_SEQUENTIAL(type, max_len, chart_idx)                          \
    printf("run sequential %s\n", #type);                                       \
    chart = charts[chart_idx];                                                  \
    for (int i = 0; i < atoi_group_num; i++) {                                  \
        atoi_##type##_func func;                                                \
        group = atoi_group_arr[i];                                              \
        func = group.type ## _func;                                             \
        tsc_avg_sum = 0;                                                        \
        if (!func) continue;                                                    \
                                                                                \
        yy_chart_item_begin(chart, group.name);                                 \
        for (int len = 1; len <= max_len; len++) {                              \
            /* fill buffer with fix-length numbers */                           \
            yy_random_reset();                                                  \
            fill_rand_##type##_len(buf, sample_count, len);                     \
                                                                                \
            /* run benchmark */                                                 \
            tsc_avg_min = HUGE_VAL;                                             \
            for (int j = 0; j < repeat_count; j++) {                            \
                tsc_begin = yy_time_get_ticks();                                \
                for (int s = 0; s < sample_count; s++) {                        \
                    type##_out = func(buf + s * type##_max_buf_len, type##_max_buf_len, &endptr, &res); \
                }                                                               \
                tsc_end = yy_time_get_ticks();                                  \
                tsc = tsc_end - tsc_begin;                                      \
                tsc_avg = (double)tsc / sample_count;                           \
                if (tsc_avg < tsc_avg_min) tsc_avg_min = tsc_avg;               \
            }                                                                   \
            tsc_avg_sum += tsc_avg_min;                                         \
            cycles = tsc_avg_min * yy_cpu_get_cycle_per_tick();                 \
            yy_chart_item_add_float(chart, (float)cycles);                      \
        }                                                                       \
        yy_chart_item_end(chart);                                               \
        tsc_avg_min = tsc_avg_sum / max_len;                                    \
        cycles = tsc_avg_min * yy_cpu_get_cycle_per_tick();                     \
    }
    
    
#define BENCHMARK_RANDOM(type, chart_idx)                                       \
    printf("run random %s\n", #type);                                           \
    /* fill buffer with unpredictable numbers */                                \
    yy_random_reset();                                                          \
    fill_rand_ ## type(buf, sample_count);                                      \
                                                                                \
    for (int i = 0; i < atoi_group_num; i++) {                                  \
        atoi_##type##_func func;                                                \
        group = atoi_group_arr[i];                                              \
        func = group.type ## _func;                                             \
        chart = charts[chart_idx];                                              \
        if (!func) continue;                                                    \
                                                                                \
        /* run benchmark */                                                     \
        tsc_avg_min = HUGE_VAL;                                                 \
        for (int j = 0; j < repeat_count; j++) {                                \
            tsc_begin = yy_time_get_ticks();                                    \
            for (int s = 0; s < sample_count; s++) {                            \
                type##_out = func(buf + s * type##_max_buf_len, type##_max_buf_len, &endptr, &res); \
            }                                                                   \
            tsc_end = yy_time_get_ticks();                                      \
            tsc = tsc_end - tsc_begin;                                          \
            tsc_avg = (double)tsc / sample_count;                               \
            if (tsc_avg < tsc_avg_min) tsc_avg_min = tsc_avg;                   \
        }                                                                       \
        cycles = tsc_avg_min * yy_cpu_get_cycle_per_tick();                     \
        yy_chart_item_with_float(chart, group.name, (float)cycles);             \
    }
    
    BENCHMARK_SEQUENTIAL(u32, 10, 0)
    BENCHMARK_RANDOM(u32, 1)
    BENCHMARK_SEQUENTIAL(u64, 20, 2)
    BENCHMARK_RANDOM(u64, 3)
    BENCHMARK_SEQUENTIAL(i32, 10, 4)
    BENCHMARK_RANDOM(i32, 5)
    BENCHMARK_SEQUENTIAL(i64, 19, 6)
    BENCHMARK_RANDOM(i64, 7)
    
    for (int i = 0; i < chart_count; i++) {
        yy_chart_sort_items_with_value(charts[i], false);
    }
    for (int i = 0; i < chart_count; i++) {
        yy_report_add_chart(report, charts[i]);
    }
    
    if (!yy_report_write_html_file(report, output_path)) {
        printf("[ERROR] report write failed: %s\n", output_path);
    } else {
        printf("report generated: %s\n", output_path);
    }
    
    for (int i = 0; i < chart_count; i++) yy_chart_free(charts[i]);
    
    yy_report_free(report);
    free(buf);
}


/*----------------------------------------------------------------------------*/

static void atoi_group_cleanup(void) {
    atoi_group_num = 0;
    atoi_group_name_max = 0;
}

static void atoi_group_register_all(void) {
#define register_group_name(_name) \
    extern u32 atoi_u32_##_name(const char *str, size_t len, char **endptr, atoi_result *res); \
    extern i32 atoi_i32_##_name(const char *str, size_t len, char **endptr, atoi_result *res); \
    extern u64 atoi_u64_##_name(const char *str, size_t len, char **endptr, atoi_result *res); \
    extern i64 atoi_i64_##_name(const char *str, size_t len, char **endptr, atoi_result *res); \
    atoi_group_arr[atoi_group_num].name = #_name; \
    atoi_group_arr[atoi_group_num].u32_func = atoi_u32_##_name; \
    atoi_group_arr[atoi_group_num].i32_func = atoi_i32_##_name; \
    atoi_group_arr[atoi_group_num].u64_func = atoi_u64_##_name; \
    atoi_group_arr[atoi_group_num].i64_func = atoi_i64_##_name; \
    if (atoi_group_name_max < (int)strlen(#_name)) atoi_group_name_max = (int)strlen(#_name); \
    atoi_group_num++;
    
    register_group_name(null); /* used to meansure the benchmark overhead */
    register_group_name(libc);
    register_group_name(libcpp);
    register_group_name(lemire);
    register_group_name(yy);
}


void benchmark(const char *output_path) {
    atoi_group_register_all();
    printf("------[verify]---------\n");
    atoi_group_verify_all();
    printf("------[benchmark]------\n");
    atoi_group_benchmark_all(output_path);
    printf("------[finish]---------\n");
    atoi_group_cleanup();
    return;
    
}
