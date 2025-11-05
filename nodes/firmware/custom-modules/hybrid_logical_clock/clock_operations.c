#include <assert.h>

#include "hybrid_logical_clock.h"
#include "clock_operations.h"

int _hlc_compare_physical(const hlc_physical_t *p1, const hlc_physical_t *p2)
{
    assert(p1 != NULL);
    assert(p2 != NULL);

    if (*p1 < *p2) {
        return -1;
    } else if (*p1 > *p2) {
        return 1;
    } else {
        return 0;
    }
}

int _hlc_compare_logical(const hlc_logical_t *l1, const hlc_logical_t *l2)
{
    assert(l1 != NULL);
    assert(l2 != NULL);

    if (*l1 < *l2) {
        return -1;
    } else if (*l1 > *l2) {
        return 1;
    } else {
        return 0;
    }
}

void _hlc_max_physical(const hlc_physical_t *p1, const hlc_physical_t *p2, hlc_physical_t *max_p)
{
    assert(p1 != NULL);
    assert(p2 != NULL);
    assert(max_p != NULL);

    *max_p = (*p1 > *p2) ? *p1 : *p2;
}

void _hlc_max_logical(const hlc_logical_t *l1, const hlc_logical_t *l2, hlc_logical_t *max_l)
{
    assert(l1 != NULL);
    assert(l2 != NULL);
    assert(max_l != NULL);

    *max_l = (*l1 > *l2) ? *l1 : *l2;
}
