#include <time.h>
#include <stdio.h>

#include "wine/debug.h"

#include "pdh_fake_counters.h"

#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

WINE_DEFAULT_DEBUG_CHANNEL(pdh);

struct cpu_fake_load g_cpu_fake_load;

struct cpu_fake_load_item cpu_fake_loads[] = {
    { .duration_s = 60, .cpu_util = 3.14,   .queue_length = 0.0},
    { .duration_s = 60*30, .cpu_util = 100.0,   .queue_length = 0.0},
    { .duration_s = 60*30, .cpu_util = 0.0,   .queue_length = 0.0},
    { .duration_s = 60*30, .cpu_util = 90.0,   .queue_length = 0.0},
    { .duration_s = 60*30, .cpu_util = 0.0,   .queue_length = 0.0},
    { .duration_s = 60*30, .cpu_util = 100.0,   .queue_length = 6.0},
};

static double _get_fake_counter_by_id(const struct cpu_fake_load_item *item, enum FAKE_COUNTER_ID id)
{
        switch (id) {
            case FAKE_COUNTER_QUEUE_LENGTH:
                return item->queue_length;
            case FAKE_COUNTER_CPU_UTIL:
                return item->cpu_util;
        }

        __builtin_unreachable();
}

#define FAKE_LOAD_WRAP 1

double get_fake_counter(enum FAKE_COUNTER_ID id)
{
    if (g_cpu_fake_load.last_counter_check == 0) {
        g_cpu_fake_load.items = cpu_fake_loads;
        g_cpu_fake_load.item_count = sizeof(cpu_fake_loads) / sizeof(cpu_fake_loads[0]);
        g_cpu_fake_load.last_counter_check = time(NULL);
    }

    const DWORD timenow = time(NULL);
    const DWORD timegap = timenow - g_cpu_fake_load.last_counter_check;

    TRACE("get_fake_counter: timenow = %ld, timegap = %ld, "
            "current_item: %ld\n", timenow, timegap, g_cpu_fake_load.current_item);

    // If we are out of items, just return the last one.
    if (g_cpu_fake_load.current_item >= g_cpu_fake_load.item_count) {

        // This should never happen.
        if (!(g_cpu_fake_load.item_count > 0)) {
            fprintf(stderr, "WARNING: Sanity check failed! No items in fake counters?\n");
            return 0.0;
        }

        if (FAKE_LOAD_WRAP) {
            g_cpu_fake_load.current_item = 0;
        } else {
            const struct cpu_fake_load_item * const lastitem =
                &g_cpu_fake_load.items[g_cpu_fake_load.item_count - 1];

            return _get_fake_counter_by_id(lastitem, id);
        }
    }

    // TODO: TOCTOU? Make sure only one thread reads g_cpu_fake_load.
    const struct cpu_fake_load_item *curitem = &g_cpu_fake_load.items[g_cpu_fake_load.current_item];
    if (timegap >= curitem->duration_s) {

        //
        // Advance to next item and return new value.
        //
        // TODO: implement multiple item skips - we might have been called not
        //       frequent enough and have to skip few items at once.
        ++g_cpu_fake_load.current_item;
        ++curitem;
        g_cpu_fake_load.last_counter_check = timenow;

    }

    return _get_fake_counter_by_id(curitem, id);
}
