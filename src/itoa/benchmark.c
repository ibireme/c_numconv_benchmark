/*
 * Copyright (c) 2018 YaoYuan <ibireme@gmail.com>.
 * Released under the MIT license (MIT).
 */

#include "yybench.h"

#ifndef BENCHMARK_DATA_PATH
extern const char *benchmark_get_data_path(void);
#define BENCHMARK_DATA_PATH benchmark_get_data_path()
#endif

/*----------------------------------------------------------------------------*/

/*
 Function prototype:
 The buffer should be large enough to hold any possible result.
 The return value point to ending position.
 Null-terminator is not required.
 */
typedef char *(*itoa_u32_func)(u32, char *);
typedef char *(*itoa_i32_func)(i32, char *);
typedef char *(*itoa_u64_func)(u64, char *);
typedef char *(*itoa_i64_func)(i64, char *);

typedef struct {
    itoa_u32_func u32_func;
    itoa_i32_func i32_func;
    itoa_u64_func u64_func;
    itoa_i64_func i64_func;
    const char *name;
    bool need_verify;
    bool need_benchmark;
} itoa_group;

static itoa_group itoa_group_expect; /* used to verify other groups */
static itoa_group itoa_group_array[64] = {0}; /* all groups */
static u32 itoa_group_count = 0;
static u32 itoa_group_name_maxlen = 0;


/*----------------------------------------------------------------------------*/

// register a function group
static void itoa_group_register(itoa_group group) {
    size_t namelen = strlen(group.name);
    if (namelen > itoa_group_name_maxlen) itoa_group_name_maxlen = (int)namelen;
    itoa_group_array[itoa_group_count] = group;
    itoa_group_count++;
}

#define GROUP_REGISTER_ARGS(group_name, verify, benchmark) do { \
    extern int itoa_ ## group_name ## _available_32; \
    extern int itoa_ ## group_name ## _available_64; \
    extern char *itoa_u32_ ## group_name(u32, char *); \
    extern char *itoa_i32_ ## group_name(i32, char *); \
    extern char *itoa_u64_ ## group_name(u64, char *); \
    extern char *itoa_i64_ ## group_name(i64, char *); \
    itoa_group group = { 0 }; \
    group.name = # group_name; \
    group.need_verify = verify; \
    group.need_benchmark = benchmark; \
    if (itoa_ ## group_name ## _available_32) { \
        group.u32_func = itoa_u32_ ## group_name; \
        group.i32_func = itoa_i32_ ## group_name; \
    } \
    if (itoa_ ## group_name ## _available_64) { \
        group.u64_func = itoa_u64_ ## group_name; \
        group.i64_func = itoa_i64_ ## group_name; \
    } \
    if (group.u32_func || group.u64_func) { \
        itoa_group_register(group); \
    } \
} while(0)

#define GROUP_REGISTER(group_name) \
    GROUP_REGISTER_ARGS(group_name, true, true)

#define GROUP_REGISTER_EMPTY(group_name) \
    GROUP_REGISTER_ARGS(group_name, false, true)

#define GROUP_REGISTER_EXPECT(group_name) do { \
    extern char *itoa_u32_ ## group_name(u32, char *); \
    extern char *itoa_i32_ ## group_name(i32, char *); \
    extern char *itoa_u64_ ## group_name(u64, char *); \
    extern char *itoa_i64_ ## group_name(i64, char *); \
    itoa_group group; \
    group.name = # group_name; \
    group.u32_func = itoa_u32_ ## group_name; \
    group.i32_func = itoa_i32_ ## group_name; \
    group.u64_func = itoa_u64_ ## group_name; \
    group.i64_func = itoa_i64_ ## group_name; \
    itoa_group_expect = group; \
} while(0)


// returns the function name aligned (inner memory)
static char *itoa_group_get_name_aligned(itoa_group group, bool align_right) {
    static char buf[64];
    int len = (int)strlen(group.name);
    int space = itoa_group_name_maxlen - len;
    if (align_right) {
        for (int i = 0; i < space; i++) buf[i] = ' ';
        memcpy(buf + space, group.name, len);
        buf[itoa_group_name_maxlen] = '\0';
    } else {
        memcpy(buf, group.name, len);
        for (int i = 0; i < space; i++) buf[len + i] = ' ';
        buf[itoa_group_name_maxlen] = '\0';
    }
    return buf;
}



/*----------------------------------------------------------------------------*/

#define ITOA_FUNC_VERIFY(type) \
static bool itoa_func_verify_ ## type(itoa_##type##_func t, \
                                      itoa_##type##_func e, \
                                      type v) { \
    char buf_t[64], buf_e[64]; \
    char *end_t, *end_e; \
    int len_t, len_e; \
    \
    memset(buf_t, 0xFF, sizeof(buf_t)); \
    memset(buf_e, 0xFF, sizeof(buf_e)); \
    end_t = t(v, buf_t); \
    end_e = e(v, buf_e); \
    *end_t = '\0'; \
    *end_e = '\0'; \
    len_t = (int)(end_t - buf_t); \
    len_e = (int)(end_e - buf_e); \
    if (len_t != len_e || strcmp(buf_t, buf_e) != 0) { \
        printf("\n    %s error: %s -> %s", # type,  buf_e, buf_t); \
        if (len_t != len_e) printf(" (length %d -> %d)", len_e, len_t); \
        return false; \
    } \
    return true; \
}

ITOA_FUNC_VERIFY(u32)
ITOA_FUNC_VERIFY(i32)
ITOA_FUNC_VERIFY(u64)
ITOA_FUNC_VERIFY(i64)

static bool itoa_group_verify(itoa_group t, itoa_group e) {
#define CALL_VERIFY_FUNC(type, val) \
    itoa_func_verify_ ## type (t.type ## _func, e.type ## _func, (type)(val))
#define CALL_VERIFY_GROUP(val) do {\
    if (t.u32_func) if (!CALL_VERIFY_FUNC(u32, val)) suc = false; \
    if (t.i32_func) if (!CALL_VERIFY_FUNC(i32, val)) suc = false; \
    if (t.u64_func) if (!CALL_VERIFY_FUNC(u64, val)) suc = false; \
    if (t.i64_func) if (!CALL_VERIFY_FUNC(i64, val)) suc = false; \
} while(0)
    
    bool suc = true;
    u32 power;
    u64 i, last;
    
    CALL_VERIFY_GROUP(0);
    CALL_VERIFY_GROUP(UINT32_MAX);
    CALL_VERIFY_GROUP(UINT32_MAX - 1);
    CALL_VERIFY_GROUP(UINT64_MAX);
    CALL_VERIFY_GROUP(UINT64_MAX - 1);
    CALL_VERIFY_GROUP(INT32_MAX);
    CALL_VERIFY_GROUP(INT32_MAX - 1);
    CALL_VERIFY_GROUP(INT64_MAX);
    CALL_VERIFY_GROUP(INT64_MAX - 1);
    CALL_VERIFY_GROUP(INT32_MIN);
    CALL_VERIFY_GROUP(INT32_MIN + 1);
    CALL_VERIFY_GROUP(INT64_MIN);
    CALL_VERIFY_GROUP(INT64_MIN + 1);
    
    // some test cases from Milo Yip:
    // https://github.com/miloyip/itoa-benchmark/blob/master/src/main.cpp
    for (power = 2; power <= 10; power += 1) {
        i = 1;
        do {
            CALL_VERIFY_GROUP(i);
            CALL_VERIFY_GROUP(i - 1);
            CALL_VERIFY_GROUP(-(i32)i);
            CALL_VERIFY_GROUP(-(i32)i - 1);
            CALL_VERIFY_GROUP(-(i64)i);
            CALL_VERIFY_GROUP(-(i64)i - 1);
            last = i;
            i *= power;
        } while (last < i);
    }
    return suc;
    
#undef CALL_VERIFY_GROUP
#undef CALL_VERIFY_FUNC
}

static void itoa_group_verify_all(void) {
    for (u32 i = 0; i < itoa_group_count; i++) {
        itoa_group t = itoa_group_array[i];
        if (!t.need_verify) continue;
        printf("verify %s ", itoa_group_get_name_aligned(t, false));
        u32 suc = itoa_group_verify(t, itoa_group_expect);
        printf("%s\n", suc ? "[OK]" : "");
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

static void fill_rand_u32(u32 *buf, u32 count) {
    for (u32 i = 0; i < count; i++) buf[i] = rand_u32_len(yy_random32_range(1, 10));
}

static void fill_rand_u64(u64 *buf, u32 count) {
    for (u32 i = 0; i < count; i++) buf[i] = rand_u64_len(yy_random32_range(1, 20));
}

static void fill_rand_i32(i32 *buf, u32 count) {
    for (u32 i = 0; i < count; i++) buf[i] = rand_i32_len(yy_random32_range(1, 10), yy_random32() % 2);
}

static void fill_rand_i64(i64 *buf, u32 count) {
    for (u32 i = 0; i < count; i++) buf[i] = rand_i64_len(yy_random32_range(1, 19), yy_random32() % 2);
}

static void fill_rand_u32_len(u32 *buf, u32 count, u32 len) {
    len = len > 10 ? 10 : len;
    u32 min = (u32)pow10_table[len - 1];
    u32 max = len == 10 ? UINT32_MAX : (u32)pow10_table[len] - 1;
    double step = (max - min) / (double)count;
    for (u32 i = 0; i < count; i++) buf[i] = (u32)(min + i * step);
}

static void fill_rand_u64_len(u64 *buf, u32 count, u32 len) {
    len = len > 20 ? 20 : len;
    u64 min = pow10_table[len - 1];
    u64 max = len == 20 ? UINT64_MAX : pow10_table[len] - 1;
    double step = (max - min) / (double)count;
    for (u32 i = 0; i < count; i++) buf[i] = (u64)(min + i * step);
}

static void fill_rand_i32_len(i32 *buf, u32 count, u32 len) {
    len = len > 10 ? 10 : len;
    u32 min = (u32)pow10_table[len - 1];
    u32 max = len == 10 ? INT32_MAX : (u32)pow10_table[len] - 1;
    u32 half = count / 2;
    double step = (max - min) / (double)half;
    for (u32 i = 0; i < half; i++) buf[i] = (i32)(min + i * step);
    for (u32 i = half; i < count; i++) buf[i] = -(i32)(min + i * step);
}

static void fill_rand_i64_len(i64 *buf, u32 count, u32 len) {
    len = len > 19 ? 19 : len;
    u64 min = pow10_table[len - 1];
    u64 max = len == 19 ? INT64_MAX : (u64)pow10_table[len] - 1;
    u64 half = count / 2;
    double step = (max - min) / (double)half;
    for (u64 i = 0; i < half; i++) buf[i] = (i64)(min + i * step);
    for (u64 i = half; i < (u64)count; i++) buf[i] = -(i64)(min + i * step);
}



/*----------------------------------------------------------------------------*/

static void itoa_group_benchmark_all(const char *report_file_path) {
    static const u32 chart_count = 8;
    yy_chart *charts[8], *chart;
    u32 i, j, s, len;
    u64 tsc_begin, tsc_end, tsc;
    f64 tsc_avg, tsc_avg_min, tsc_avg_sum, cycles;
    itoa_group group;
    char *in_buf, *out_buf, *out_cur;
    
    static const u32 sample_count = 10000;
    static const u32 repeat_count = 16;
    
    printf("prepare...\n");
    yy_cpu_setup_priority();
    yy_cpu_spin(0.5);
    yy_cpu_measure_freq();
    
    // create report
    yy_report *report = yy_report_new();
    yy_report_add_env_info(report);
    
    // create charts
    for (i = 0; i < chart_count; i++) {
        charts[i] = yy_chart_new();
        yy_chart_options op;
        yy_chart_options_init(&op);
        switch (i) {
            case 0: op.title = "itoa u32 (fixed length)"; break;
            case 1: op.title = "itoa u32 (random length)"; break;
            case 2: op.title = "itoa u64 (fixed length)"; break;
            case 3: op.title = "itoa u64 (random length)"; break;
            case 4: op.title = "itoa i32 (fixed length)"; break;
            case 5: op.title = "itoa i32 (random length)"; break;
            case 6: op.title = "itoa i64 (fixed length)"; break;
            case 7: op.title = "itoa i64 (random length)"; break;
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
            op.width = 800;
            op.height = 540;
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
            op.height = 420;
        }
        yy_chart_set_options(charts[i], &op);
    }
    
    /* create buffers */
    in_buf = (void *)malloc((size_t)(sample_count * 8)); /* array<uint64_t> */
    out_buf = (void *)malloc((size_t)(sample_count * 22)); /* array<char> */
    if (!in_buf || !out_buf) {
        printf("[ERROR] buffer creation failed\n");
        return;
    }
    
    
#define BENCHMARK_SEQUENTIAL(type, max_len, chart_idx)                          \
    printf("run sequential %s\n", #type);                                       \
    chart = charts[chart_idx];                                                  \
    for (i = 0; i < itoa_group_count; i++) {                                    \
        itoa_##type##_func func;                                                \
        group = itoa_group_array[i];                                            \
        func = group.type ## _func;                                             \
        tsc_avg_sum = 0;                                                        \
        if (!func || !group.need_benchmark) continue;                           \
                                                                                \
        yy_chart_item_begin(chart, group.name);                                 \
        for (len = 1; len <= max_len; len++) {                                  \
            /* fill buffer with fix-length numbers */                           \
            yy_random_reset();                                                  \
            fill_rand_##type##_len((type *)in_buf, sample_count, len);          \
                                                                                \
            /* run benchmark */                                                 \
            tsc_avg_min = HUGE_VAL;                                             \
            for (j = 0; j < repeat_count; j++) {                                \
                out_cur = out_buf;                                              \
                tsc_begin = yy_time_get_ticks();                                \
                for (s = 0; s < sample_count; s++) {                            \
                    out_cur = func(((type *)in_buf)[s], out_cur);               \
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
    fill_rand_ ## type((type *)in_buf, sample_count);                           \
                                                                                \
    for (i = 0; i < itoa_group_count; i++) {                                    \
        itoa_##type##_func func;                                                \
        group = itoa_group_array[i];                                            \
        func = group.type ## _func;                                             \
        chart = charts[chart_idx];                                              \
        if (!func || !group.need_benchmark) continue;                           \
                                                                                \
        /* run benchmark */                                                     \
        tsc_avg_min = HUGE_VAL;                                                 \
        for (j = 0; j < repeat_count; j++) {                                    \
            out_cur = out_buf;                                                  \
            tsc_begin = yy_time_get_ticks();                                    \
            for (s = 0; s < sample_count; s++) {                                \
                out_cur = func(((type *)in_buf)[s], out_cur);                   \
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
    
    for (i = 0; i < chart_count; i++) {
        yy_chart_sort_items_with_value(charts[i], false);
    }
    for (i = 0; i < chart_count; i++) {
        yy_report_add_chart(report, charts[i]);
    }
    
    if (!yy_report_write_html_file(report, report_file_path)) {
        printf("[ERROR] report write failed: %s\n", report_file_path);
    } else {
        printf("report generated: %s\n", report_file_path);
    }
    
    for (i = 0; i < chart_count; i++) yy_chart_free(charts[i]);
    yy_report_free(report);
    free(in_buf);
    free(out_buf);
}

static void itoa_group_register_all(void) {
    if (itoa_group_count > 0) return;
    GROUP_REGISTER_EXPECT(sprintf);     /* used to verify other groups */
    GROUP_REGISTER_EMPTY(null);         /* used to meansure the benchmark overhead */
    
    GROUP_REGISTER(yy);
    GROUP_REGISTER(yy_largelut);
    GROUP_REGISTER(lut);
    GROUP_REGISTER(count);
    GROUP_REGISTER(countlut);
    GROUP_REGISTER(branchlut);
    GROUP_REGISTER(branchlut2);
    GROUP_REGISTER(unrolledlut);
    GROUP_REGISTER(naive);
    GROUP_REGISTER(unnamed);
    GROUP_REGISTER(jeaiii);
    GROUP_REGISTER(protobuf);
    GROUP_REGISTER(fmtlib);
    GROUP_REGISTER(jiaendu);        /* no u64toa/i64toa implementation */
    GROUP_REGISTER(sse2);           /* require x86 cpu with SSE2 */
    GROUP_REGISTER(an);             /* require __builtin_ctz and __uint128_t */
    GROUP_REGISTER(tmueller);       /* require __builtin_clzll for 64-bit integers */
    GROUP_REGISTER(amartin);        /* require C++14 and __uint128_t */
    GROUP_REGISTER(amartin_ljust);  /* require C++98 */
    GROUP_REGISTER(amartin_ljust2); /* require C++11 */
}

void benchmark(const char *output_file_path) {
    itoa_group_register_all();
    printf("------[verify]---------\n");
    itoa_group_verify_all();
    printf("------[benchmark]------\n");
    itoa_group_benchmark_all(output_file_path);
    printf("------[finish]---------\n");
}
