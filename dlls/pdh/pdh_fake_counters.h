#include "windef.h"

struct cpu_fake_load_item
{
    DWORD duration_s;
    double cpu_util;
    double queue_length;
};

struct cpu_fake_load
{
    struct cpu_fake_load_item *items;
    DWORD item_count;
    DWORD current_item;
    DWORD last_counter_check;
};

extern struct cpu_fake_load g_cpu_fake_load;

enum FAKE_COUNTER_ID {
    FAKE_COUNTER_QUEUE_LENGTH,
    FAKE_COUNTER_CPU_UTIL,
};

double get_fake_counter(enum FAKE_COUNTER_ID id);
