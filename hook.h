#ifndef KHM_HOOK_H
#define KHM_HOOK_H

#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/ftrace.h>
#include <linux/version.h>
#include "queue.h"
#include "ctor.h"

struct filter {
    const char *fname;
    void *filter;
    void *real;
    TAILQ_ENTRY(filter) list;
    atomic_t ref;
    unsigned int flags;
    void *address;
    struct ftrace_ops ops;
    int error;
};

static TAILQ_HEAD(, filter) flts = TAILQ_HEAD_INITIALIZER(flts);

int filter_disabled(void);
void filter_register(struct filter *flt);

#define START_PROFILE(name)
#define END_PROFILE(name)

#define REAL(name, ...) ((__fptr_ ## name)(__flt_ ## name.real))(__VA_ARGS__)


#define FILTER_FLAGS(name, x)                   \
static void __set_filter_flags_ ## name(void) { \
    __flt_ ## name.flags |= (x);                \
}                                               \
__initcall(__set_filter_flags_ ## name);

#define FILTER_DEFINEx(x, name, type, ...)                              \
static type __wrapfilter_ ## name(__MAP(x, __SC_DECL, __VA_ARGS__));    \
static struct filter __flt_ ## name = {                                 \
    .fname = #name,                                                     \
    .filter = __wrapfilter_ ## name,                                    \
    .ref = ATOMIC_INIT(0),                                         \
};                                                                      \
static void __register_flt_ ## name(void) {                             \
    filter_register(&__flt_ ## name);                                   \
}                                                                       \
__initcall(__register_flt_ ## name);                                    \
typedef type (*__fptr_ ## name)(__MAP(x, __SC_DECL, __VA_ARGS__));      \
typedef type __retype_ ## name;                                         \
static inline type __filter_ ## name(__MAP(x, __SC_DECL, __VA_ARGS__)); \
static type __wrapfilter_ ## name(__MAP(x, __SC_DECL, __VA_ARGS__))     \
{                                                                       \
    type ret;                                                           \
    atomic_inc(&__flt_ ## name.ref);                               \
    START_PROFILE(name);                                                \
    if (0/*filter_disabled()*/) {                                            \
        ret = REAL(name, __MAP(x, __SC_ARGS, __VA_ARGS__));             \
    } else {                                                            \
        ret = __filter_ ## name(__MAP(x, __SC_ARGS, __VA_ARGS__));      \
    }                                                                   \
    END_PROFILE(name);                                                  \
    atomic_dec(&__flt_ ## name.ref);                               \
    return ret;                                                         \
}                                                                       \
static inline type __filter_ ## name(__MAP(x, __SC_DECL, __VA_ARGS__))

#define FILTER_DEFINE0(name, t, ...) FILTER_DEFINE1(name, t, void, )
#define FILTER_DEFINE1(name, t, ...) FILTER_DEFINEx(1, name, t, __VA_ARGS__)
#define FILTER_DEFINE2(name, t, ...) FILTER_DEFINEx(2, name, t, __VA_ARGS__)
#define FILTER_DEFINE3(name, t, ...) FILTER_DEFINEx(3, name, t, __VA_ARGS__)
#define FILTER_DEFINE4(name, t, ...) FILTER_DEFINEx(4, name, t, __VA_ARGS__)
#define FILTER_DEFINE5(name, t, ...) FILTER_DEFINEx(5, name, t, __VA_ARGS__)
#define FILTER_DEFINE6(name, t, ...) FILTER_DEFINEx(6, name, t, __VA_ARGS__)
#define FILTER_DEFINE7(name, t, ...) FILTER_DEFINEx(7, name, t, __VA_ARGS__)
#define FILTER_DEFINE8(name, t, ...) FILTER_DEFINEx(8, name, t, __VA_ARGS__)

#ifndef __MAP7
#define __MAP7(m,t,a,...) m(t,a), __MAP6(m,__VA_ARGS__)
#define __MAP8(m,t,a,...) m(t,a), __MAP7(m,__VA_ARGS__)
#endif


#define PTREGS_NEED_REAL    1

#define FLT_ALREADY_UNINSTALL		(1<<0)

int filter_hook(void);
int filter_unhook(void);


#endif
