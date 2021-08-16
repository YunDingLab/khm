#include "ctor.h"

extern my_ctor_fn_t ctor_start[], ctor_end[];
extern my_dtor_fn_t dtor_start[], dtor_end[];

void ctor_init(void)
{
    my_ctor_fn_t *start = ctor_start;
    while (start < ctor_end) {
        if (*start)
            (*start)();
        ++start;
    }
}

void dtor_exit(void)
{
    my_dtor_fn_t *start = dtor_start;
    while (start < dtor_end) {
        if (*start)
            (*start)();
        ++start;
    }
}
