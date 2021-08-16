#ifndef KHM_CTOR_H
#define KHM_CTOR_H

typedef void (*my_ctor_fn_t)(void);
typedef void (*my_dtor_fn_t)(void);

#define __initcall(fn)                      \
    static my_ctor_fn_t __init_ ## fn __used   \
    __attribute__((section(".ctor"))) = fn;
#define __exitcall(fn)                      \
    static my_dtor_fn_t __exit_ ## fn __used   \
    __attribute__((section(".dtor"))) = fn;

void ctor_init(void);
void dtor_exit(void);

#endif
