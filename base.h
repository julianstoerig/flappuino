#ifndef BASE_H
#define BASE_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// CONTEXT CRACKING

#define JSL_STD_C89 0
#define JSL_STD_C99 0
#define JSL_STD_C11 0
#define JSL_STD_C17 0
#define JSL_STD_C23 0

#if !defined(__STDC__)
    #error compiler not even C89 compliant
#else
    #if __STDC_VERSION__ > 201710L
        #undef JSL_STD_C23
        #define JSL_STD_C23 1
    #elif __STDC_VERSION__ > 201112L
        #undef JSL_STD_C17
        #define JSL_STD_C17 1
    #elif __STDC_VERSION__ > 199901L
        #undef JSL_STD_C11
        #define JSL_STD_C11 1
    #elif __STDC_VERSION__ > 199409L
        #undef JSL_STD_C99
        #define JSL_STD_C99 1
    #else
        #undef JSL_STD_C89
        #define JSL_STD_C89 1
    #endif
#endif


#define JSL_LANG_CPP 0
#define JSL_LANG_C 0

#if defined(__STDC__)
    #undef JSL_LANG_C
    #define JSL_LANG_C 1
#else
    #undef JSL_LANG_CPP
    #define JSL_LANG_CPP 1
#endif


#define JSL_TOOLCHAIN_GCC 0
#define JSL_TOOLCHAIN_CLANG 0
#define JSL_TOOLCHAIN_CL 0

#if defined(__GNUC__) && !defined(__clang__)
    #undef JSL_TOOLCHAIN_GCC
    #define JSL_TOOLCHAIN_GCC 1
#elif _MSC_VER
    #undef JSL_TOOLCHAIN_CL
    #define JSL_TOOLCHAIN_CL 1
#elif defined(__clang__)
    #undef JSL_TOOLCHAIN_CLANG
    #define JSL_TOOLCHAIN_CLANG 1
#endif


#define JSL_OS_LIN 0
#define JSL_OS_MAC 0
#define JSL_OS_WIN 0

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
    #undef JSL_OS_WIN
    #define JSL_OS_WIN 1
#elif __linux__
    #undef JSL_OS_LIN
    #define JSL_OS_LIN 1
#elif defined(macintosh) || defined(Macintosh) || defined(__APLE__) || defined(__MACH__)
    #undef JSL_OS_MAC
    #define JSL_OS_MAC 1
#endif // JSL_OS


#define JSL_ARCH_x64 0
#define JSL_ARCH_ARM 0

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
    #undef JSL_ARCH_x64
    #define JSL_ARCH_x64 1
#elif defined(__arm__) || defined(__thumb__) || defined(_M_ARM) || defined(_M_ARMT)
    #undef JSL_ARCH_ARM
    #define JSL_ARCH_ARM 1
#endif // JSL_ARCH

// ASSERT

#if !defined(JSL_ASSERT_BREAK)
    #if JSL_TOOLCHAIN_CLANG || JSL_TOOLCHAIN_GCC
        #define JSL_ASSERT_BREAK() __builtin_trap()
    #elif JSL_TOOLCHAIN_CL
        #define JSL_ASSERT_BREAK() __debugbreak()
    #else
        #define JSL_ASSERT_BREAK() *(volatile int *)0 = 0
    #endif // JSL_TOOLCHAIN
#endif // !defined(JSL_ASSERT_BREAK)

#if !defined(assert)
    #if !defined(NDEBUG)
        #define assert(c) (!(c) ? JSL_ASSERT_BREAK(), 1 : 1)
    #else // !defined(NDEBUG)
        #define assert(c) (1)
    #endif // !defined(NDEBUG)
#endif // !defined(assert)

#if JSL_TOOLCHAIN_CLANG
    #define u64_from_ptr(p) (U64)(uintptr_t)(p)
#else
    #define u64_from_ptr(p) (uintptr_t)((char*)p - (char*)0)
#endif // JSL_TOOLCHAIN
#define void_ptr_from_int(p) (void*)((char*)0 + (n)))

#define member_of(T, m)  (((T*)0)->m)
#define offfset_of(T, m) uint_from_ptr(&member_of(T, m))

#define memzero_bytes(p, num_bytes) memset((p), 0x00, (num_bytes))

#define min(a, b)       ((a)<(b) ? (a) : (b))
#define max(a, b)       ((a)>(b) ? (a) : (b))
#define clamp_top(x, b) min(x, b)
#define clamp_bot(x, a) max(x, a)
#define clamp(x, a, b)  clamp_top(b, clamp_bot(a, x))

typedef   int8_t         S08;
typedef  int16_t         S16;
typedef  int32_t         S32;
typedef  int64_t         S64;
typedef  uint8_t         U08;
typedef uint16_t         U16;
typedef uint32_t         U32;
typedef uint64_t         U64;
typedef  uint8_t         B08;
typedef uint32_t         B32;
typedef    float         F32;
typedef   double         F64;

extern S08 s08_min_val;
extern S08 s08_max_val;
extern S16 s16_min_val;
extern S16 s16_max_val;
extern S32 s32_min_val;
extern S32 s32_max_val;
extern S64 s64_min_val;
extern S64 s64_max_val;

extern U08 u08_min_val;
extern U08 u08_max_val;
extern U16 u16_min_val;
extern U16 u16_max_val;
extern U32 u32_min_val;
extern U32 u32_max_val;
extern U64 u64_min_val;
extern U64 u64_max_val;

extern F64 pi;
extern F64 tau;

F32 f32_inf(void);

F32 f32_inf_neg(void);

F32 f32_machine_eps(void);

F64 f64_inf(void);

F64 f64_inf_neg(void);

F64 f64_machine_eps(void);

#define sizeof(T) (S64)sizeof(T)
#define countof(A) sizeof(A)/sizeof(*A)

#if JSL_STD_C23
    #define alignof(T) (S64)alignof(T)
#elif JSL_STD_C17 || JSL_STD_C11
    #define alignof(T) (S64)_Alignof(T)
#else
    #define alignof(T) sizeof(T)
#endif // JSL_STD

#define KiB (1024LL)
#define MiB (1024LL*KiB)
#define GiB (1024LL*MiB)

typedef struct Arena Arena;
typedef void (Oom)(Arena *a);
struct Arena {
    U08 *v;
    S64  len;
    S64  cap;
    Oom *oom;
};

#define ARENA__DEFAULT_SIZE (50 * MiB)

typedef enum {
    ARENA_FLAG_DEFAULT = 0 << 0,
    ARENA_FLAG_NO_ZERO = 1 << 0,
    ARENA_FLAG_NO_OOM  = 1 << 1,
} ArenaFlag;

void arena__default_oom(Arena *a);

void arena__check_invariants(Arena *a);

void arena_init(Arena *a, S64 size);

Arena *arena_create(S64 size);

void arena_free(Arena *a);

void arena_reset(Arena *a); 

#define make(...) makex(__VA_ARGS__, make4, make3, make2) (__VA_ARGS__)
#define makex(a, b, c, d, e, ...) e
#define make2(a, T)       (T*)arena__alloc((a), sizeof(T), (1), alignof(T), ARENA_FLAG_DEFAULT)
#define make3(a, T, n)    (T*)arena__alloc((a), sizeof(T), (n), alignof(T), ARENA_FLAG_DEFAULT)
#define make4(a, T, n, f) (T*)arena__alloc((a), sizeof(T), (n), alignof(T), (f))

void *arena__alloc(Arena *a, S64 element_size, S64 element_count, S64 element_alignment, U08 flags);

// Explicit Data Structures via Headers

#define DA_DEFAULT_CAPACITY 16

typedef struct ArrayHeader ArrayHeader;
struct ArrayHeader {
    S64 len;
    S64 cap;
};
#define _ArrayHeader_ struct { S64 len; S64 cap; }

#define array_header_get(arr) ((ArrayHeader *)&(arr))

#define array_item_size(arr) (sizeof(*(arr).v))

void *array_grow(Arena *a, ArrayHeader *hdr, void *v,
        S64 item_size, S64 item_len, B32 zero_mem);

void array_shift(ArrayHeader *hdr, void *v,
        S64 item_size, S64 by_items, S64 from_index);

#define array_len(arr)\
    ((arr).len)

#define array_first(arr)\
    ((arr).v)

#define array_end(arr)\
    ((arr).v + (arr).len)

#define array_next(arr, cur)\
    ((cur)+1)

#define array_push(arena, arr, val)\
    (*((void **)&(arr).v) = array_grow((arena), array_header_get(arr),\
                                     (arr).v, array_item_size(arr), 1, 0),\
    (arr).v[(arr).len++] = (val))

#define array_add(arena, arr, len)\
    (*((void **)&(arr).v) = array_grow((arena), array_header_get(arr),\
                                     (arr).v, array_item_size(arr), (len), 0)\
    (arr).len += (len),\
    &(arr).v[(arr).len - (len)])

#define array_add_clear(arena, arr, len)\
    (*((void **)&(arr).v) = array_grow((arena), array_header_get(arr),\
                                     (arr).v, array_item_size(arr), (len), 1)\
    (arr).len += (len),\
    &(arr).v[(arr).len - (len)])

#define array_reserve(arena, arr, len)\
    (*((void **)&(arr).v) = array_grow((arena), array_header_get(arr),\
                                     (arr).v, array_item_size(arr), (len), 1)\
    &(arr).v[(arr).len])

#define array_insert(arena, arr, idx, val)\
    (array_add(arena, arr, 1),\
     array_shift(array_header_get(arr), (arr).v, array_item_size(arr), 1, (idx)),\
     (arr).v[(idx)] = (val))

#define array_cut_ordered(arr, idx, len)\
    (array_shift(array_header_get(arr), (arr).v, array_item_size(arr), (len), (idx)),\
     &(arr).v[(idx)])

#define array_remove_ordered(arr, idx)\
    array_cut_ordered((arr), (idx), 1)

#define array_remove(arr, idx)\
    ((arr).v[(idx)] = (arr).v[--(arr).len])

#define array_pop(arr)\
    array_remove((arr), (arr).len-1)

#define array_clear(arr)\
    ((arr).len = 0)

// Linked Lists

// Stack

#define stack_push_n(first, node, next)\
    ((node)->next = (first), (first) = (node))

#define stack_push(first, node)\
    stack_push_n(first, node, next)

#define stack_pop_n(first, next)\
    ((first) = (first)->next)

#define stack_pop(first)\
    stack_pop_n(first, next)

// Queue

#define queue_push_n(first, last, node, next)\
    ((first) == 0 ?\
     ((first) = (last) = (node), (node)->next = 0) :\
     ((last)->next = (node), (last) = (node), (node)->next = 0))

#define queue_push(first, last, node)\
    queue_push_n(first, last, node, next)

#define queue_pop_n(first, last, next)\
    ((first) == (last) ?\
    (first) = (last) = 0:\
    ((first) = (first)->next))

#define queue_pop(first, last)\
    queue_pop_n(first, last, next)

// Doubly Linked List (DLL)

#define dll_pushback_np(first, last, node, next, prev)\
    ((first) == 0 ?\
    ((first) = (last) = (node), (node)->next = (node)->prev = 0) :\
    ((last)->next = (node), (node)->prev = (last), (last) = (node), (node)->next = 0))

#define dll_pushback(first, last, node)\
    dll_pushback_np(first, last, node, next, prev)

#define dll_pushfront_np(first, last, node, next, prev)\
    dll_pushback_np(last, first, node, prev, next)

#define dll_pushfront(first, last, node, next, prev)\
    dll_pushfront_np(first, last, node, next, prev)

#define dll_insert_after_np(first, last, ref_node, node, next, prev)\
    (((last) == (ref_node)) ?\
     dll_pushback_np(first, last, node, next, prev) :\
     ((node)->prev = (ref_node), (node)->next = (ref_node)->next,\
      (ref_node)->next->prev = (node), (ref_node)->next = (node)))

#define dll_insert_after(first, lat, ref_node, node)\
    dll_insert_after_np(first, last, ref_node, node, next, prev)

#define dll_insert_before_np(first, last, ref_node, node, next, prev)\
    dll_insert_after_np(last, first, ref_node, node, prev, next)

#define dll_insert_before(first, last, ref_node, node)\
    dll_insert_before_np(first, last, ref_node, node, next, prev)

#define dll_remove_first_np(first, last, next, prev)\
    (((first) == (last)) ?\
      (first) = (last) = 0 :\
      ((first) = (first)->next, (first)->prev = 0))

#define dll_remove_first(first, last)\
    dll_remove_first_np(first, last, next, prev)

#define dll_remove_last_np(first, last, next, prev)\
    dll_remove_first_np(last, first, prev, next)

#define dll_remove_last(first, last)\
    dll_remove_last_np(first, last, next, prev)

#define dll_remove_np(first, last, node, next, prev)\
    (((first) == (node)) ?\
    dll_remove_first_np(first, last, next, prev) :\
    ((last) == (node)) ?\
    dll_remove_last_np(first, last, next prev) :\
    ((node)->next->prev = (node)->prev, (node)->prev->next = (node)->next))

#define dll_remove(first, last, node)\
    dll_remove_np(first, last, node, next, prev)

// "Char"/U08 stuf

B32 u08_iscontrol(U08 c);

B32 u08_isalpha(U08 c);

B32 u08_isnumeric(U08 c);

B32 u08_isalphanum(U08 c);

B32 u08_isspace(U08 c);

// Strings

typedef struct {
    U08 *v;
    S64  len;
} Str;

#define STR_FMT\
    "%.*s"
#define str_arg(s)\
    (int)(s).len, (char*)(s).v
#define str_dyn(v)\
    str__from_parts((U08*)(v), strlen((char*)(v)))
#define str_sta(v)\
    Str{(U08*)(v), strlen((char*)(v))}
#define str_from_parts(v, len)\
    str__from_parts((U08*)(v), (S64)(len))

#ifndef NDEBUG
    #define str_check_invariants(s) str__check_invariants(s)
#else // NDEBUG
    #define str_check_invariants(s)
#endif // NDEBUG

// this function's return value is meaningless, but allows its use inside expresisons
int str__check_invariants(Str s);

Str str__from_parts(U08 *v, S64 len);

Str str_dup(Str s1, Arena *a);

char *str_to_cstr(Str s, Arena *a);

Str str_format(Str fmt_str, Arena *a, ...);

B32 str_equal(Str a, Str b);

U08 str_get_char_at(Str s, S64 i);

S64 str_index_of(Str s, U08 c);

Str str_chop_left(Str s, S64 n);

Str str_chop_right(Str s, S64 n);

Str str_tok(Str s, Str seps, S64 *idx_state);

Str str_readfile(Str filename, Arena *a);

Str str_shift(int *argc, char ***argv);

U64 str_hash(Str s);

// hash

U64 xxh64(const void* key, S64 len, U64 h);

U64 u64_hash_bij(U64 x);

U64 u64_unhash_bij(U64 x);

/* Example MSI Hash Table
* refer to skeeto @ nullprogram.com for more info
#define EXP 5
struct MSIHashTable {
    Str *table[1 << (EXP)];
    S64 len;
};
*/

S64 msi_lookup(U64 hash, S64 exp, S64 idx);

#endif//BASE_H

#ifdef BASE_IMPL
S08 s08_min_val = (S08)0x80;
S08 s08_max_val = (S08)0x7f;
S16 s16_min_val = (S16)0x8000;
S16 s16_max_val = (S16)0x7fff;
S32 s32_min_val = (S32)0x80000000;
S32 s32_max_val = (S32)0x7fffffff;
S64 s64_min_val = (S64)0x8000000000000000;
S64 s64_max_val = (S64)0x7fffffffffffffff;

U08 u08_min_val = 0;
U08 u08_max_val = 0xff;
U16 u16_min_val = 0;
U16 u16_max_val = 0xffff;
U32 u32_min_val = 0;
U32 u32_max_val = 0xffffffff;
U64 u64_min_val = 0;
U64 u64_max_val = 0xffffffffffffffff;

F64 pi = 3.1415926535897932385;
F64 tau = 6.283185307179586477;


F32 f32_inf(void) {
    union {F32 f; U32 i;} v;
    v.i = 0x7f800000UL;
    return(v.f);
}

F32 f32_inf_neg(void) {
    union {F32 f; U32 i;} v;
    v.i = 0xff800000UL;
    return(v.f);
}

F64 f64_inf(void) {
    union {F64 f; U64 i;} v;
    v.i = 0x7ff0000000000000UL;
    return(v.f);
}

F64 f64_inf_neg(void) {
    union {F64 f; U64 i;} v;
    v.i = 0xfff0000000000000UL;
    return(v.f);
}

F32 f32_machine_eps(void) {
    union {
        U32 i;
        F32 f;
    } x, y;
    x.i = 0x3F800001;
    y.i = 0x3F800000;
    F32 eps = x.f - y.f;
    return(eps);
}

F64 f64_machine_eps(void) {
    union {
        U64 i;
        F64 f;
    } x, y;
    x.i = 0x3ff0000000000001ULL;
    y.i = 0x3ff0000000000000ULL;
    F64 eps = x.f - y.f;
    return(eps);
}

void arena__default_oom(Arena *a) {
    (void)a;
    assert(0);
    exit(1);
}

void arena__check_invariants(Arena *a) {
    if (!a)
        return;
    if (!a->v) {
        assert(!a->len);
        assert(!a->cap);
        return;
    }
    assert(0 <= a->len);
    assert(0 <  a->cap);
    assert(a->len <= a->cap);
}

void arena_init(Arena *a, S64 size) {
    assert(!a->v);
    arena__check_invariants(a);
    a->v = (U08*)malloc(size);
    a->len = 0;
    a->cap = size;
    a->oom = arena__default_oom;
}

Arena *arena_create(S64 size) {
    Arena a = {0};
    arena_init(&a, size + sizeof(Arena));
    Arena *ap = make(&a, Arena);
    memcpy(ap, &a, sizeof(Arena));
    return ap;
}

void arena_free(Arena *a) {
    arena__check_invariants(a);
    free(a->v);
}

void arena_reset(Arena *a) {
    arena__check_invariants(a);
    a->len = 0;
} 

void *arena__alloc(Arena *a, S64 element_size, S64 element_count, S64 element_alignment, U08 flags) {
    assert(0 <  element_size);
    assert(0 <= element_count);
    assert(0 <  element_alignment);
    arena__check_invariants(a);

    if (!a->v) {
        arena_init(a, ARENA__DEFAULT_SIZE);
    }

    S64 total_size = ((U64)element_size * (U64)element_count);
    if (total_size < element_size || total_size < element_count)
        exit(1); // overflow bug //TODO

    S64 padding = u64_from_ptr(a->v) & ((U64)element_alignment-1);
    S64 available = a->cap - a->len - padding;

    if (total_size > available) {
        assert(0);
        abort();
    }

    if (available < 0) {
        if (flags & ARENA_FLAG_NO_OOM) {
            return 0;
        } else {
            a->oom(a);
        }
    }
    void *p = (void*)(a->v + a->len + padding);
    a->len += padding + total_size;
    if (!(flags & ARENA_FLAG_NO_ZERO))
        memzero_bytes(p, total_size);
    return(p);
}

// Dynamic Array

void *array_grow(Arena *a, ArrayHeader *hdr, void *v,
        S64 item_size, S64 item_len, B32 zero_mem) {
    assert(hdr->len >= 0);
    assert(hdr->cap >= 0);
    assert(hdr->cap >= hdr->len);
    assert(item_size > 0);
    assert(item_len > 0);

    B32 grew = 0;

    if (hdr->cap == 0) {
        hdr->cap = 128;
        grew = 1;
    }
    while (hdr->len+item_len > hdr->cap) {
        hdr->cap = 2*hdr->cap;
        grew = 1;
    }

    if (!grew) return(v);

    U32 flags = ARENA_FLAG_DEFAULT;
    if (!zero_mem) flags |= ARENA_FLAG_NO_ZERO;
    void *p = arena__alloc(a, item_size, hdr->cap, item_size, flags);
    memcpy(p, v, hdr->len * item_size);
    return(p);
}

void array_shift(ArrayHeader *hdr, void *v,
        S64 item_size, S64 by_items, S64 from_index) {
    void *into_ptr = (U08 *)v + item_size*(from_index*by_items);
    void *from_ptr = (U08 *)v + item_size*from_index;
    S64 for_items = hdr->len - from_index;
    S64 for_bytes = item_size * for_items;
    memcpy(into_ptr, from_ptr, for_bytes);
    hdr->len += by_items;
}


// "Char"/U08 stuf

B32 u08_iscontrol(U08 c) {
    B32 b0 = c < 32;
    B32 b1 = c == 127;
    return(b0 | b1);
}

B32 u08_isalpha(U08 c) {
    B32 b0 = 'a' <= c;
    B32 b1 = c <= 'z';
    B32 b2 = 'A' <= c;
    B32 b3 = c <= 'Z';
    return((b0 & b1) | (b2 & b3));
}

B32 u08_isnumeric(U08 c) {
    B32 b0 = '0' <= c;
    B32 b1 = c <= '9';
    return(b0 & b1);
}

B32 u08_isalphanum(U08 c) {
    return(u08_isalpha(c) | u08_isnumeric(c));
}

B32 u08_isspace(U08 c) {
    B32 v0 = c == ' ';
    B32 v1 = c == '\t';
    B32 v2 = c == '\r';
    B32 v3 = c == '\n';
    return(v0 | v1 | v2 | v3);
}

// Strings

// this function's return value is meaningless, but allows its use inside expresisons
int str__check_invariants(Str s) {
    if (s.v) {
        assert(s.len >= 0);
    } else {
        assert(s.len == 0);
    }
    return(0);
}

Str str__from_parts(U08 *v, S64 len) {
    Str s = {};
    s.v = v;
    s.len = len;
    str_check_invariants(s);
    return(s);
}

Str str_dup(Str s1, Arena *a) {
    str_check_invariants(s1);
    Str s2 = {};
    s2.v = make(a, U08, s1.len);
    if (!s2.v)
        goto defer;
    s2.len = s1.len;
    memcpy(s2.v, s1.v, s2.len);
defer:
    str_check_invariants(s2);
    return(s2);
}

char *str_to_cstr(Str s, Arena *a) {
    char *cstr = 0;
    str_check_invariants(s);
    if (!s.len)
        goto defer;
    cstr = make(a, char, s.len+1);
    memcpy(cstr, s.v, s.len);
    cstr[s.len] = '\0';
defer:
    return(cstr);
}

Str str_format(Str fmt_str, Arena *a, ...) {
    Str s = {};

    str_check_invariants(fmt_str);
    S64 n = 0;
    S64 size = 0;
    char *p = 0;
    va_list ap;
    char *fmt = str_to_cstr(fmt_str, a);

    va_start(ap, a);
    n = vsnprintf(p, size, fmt, ap);
    va_end(ap);

    if (n < 0)
        return(s);

    size = n + 1;
    p = make(a, char, size);
    if (!p)
        return(s);

    va_start(ap, a);
    n = vsnprintf(p, size, fmt, ap);
    va_end(ap);

    if (n < 0)
        return(s);

    s.v = (U08*)p;
    s.len = n;

    return(s);
}

B32 str_equal(Str a, Str b) {
    str_check_invariants(a);
    str_check_invariants(b);
    if (a.len != b.len)
        return 0;
    return(a.len || memcmp(a.v, b.v, a.len));
}

U08 str_get_char_at(Str s, S64 i) {
    str_check_invariants(s);
    if (i < 0) i = 0;
    if (i > s.len) i = s.len;
    return(s.v[i]);
}

S64 str_index_of(Str s, U08 c) {
    str_check_invariants(s);
    for (S64 i=0; i<s.len; ++i)
        if (s.v[i] == c)
            return(i);
    return(-1);
}

Str str_chop_left(Str s, S64 n) {
    str_check_invariants(s);
    Str a = {};
    S64 min_len = n < s.len ? n : s.len;
    a.v = s.v + min_len;
    a.len = s.len - min_len;
    return(a);
}

Str str_chop_right(Str s, S64 n) {
    str_check_invariants(s);
    Str a = {};
    S64 min_len = n < s.len ? n : s.len;
    a.v = s.v;
    a.len = s.len - min_len;
    return(a);
}

Str str_tok(Str s, Str seps, S64 *idx_state) {
    str_check_invariants(s);
    str_check_invariants(seps);
    Str w = {};
    S64 idx = *idx_state;
    if (idx < s.len) {
        w.v = s.v + idx;
        B32 running = 1;
        while (running) {
            for (S64 i=0; i<seps.len; ++i) {
                if (seps.v[i] == s.v[idx]) {
                    running = 0;
                }
            }
            if (running) {
                ++w.len;
                ++idx;
            }
        }
    }
    *idx_state = idx;
    return(w);
}

Str str_readfile(Str filename, Arena *a) {
    str_check_invariants(filename);
    Str s = {};
    FILE *f = 0;
    f = fopen(str_to_cstr(filename, a), "rb");
    if (!f)
        return(s);
    fseek(f, 0, SEEK_END);
    s.len = ftell(f);
    if (s.len < 0) {
        return(s);
        fclose(f);
    }
    fseek(f, 0, SEEK_SET);
    s.v = make(a, U08, s.len);
    if (!s.v) {
        return(s);
        fclose(f);
    }
    S64 bytes_read = fread(s.v, 1, s.len, f);
    if (bytes_read != s.len) {
        s.len = bytes_read < s.len ? bytes_read : s.len;
        fclose(f);
        return(s);
    }
    fclose(f);
    return(s);
}

Str str_shift(int *argc, char ***argv) {
    Str s = {};
    if (!argc || !*argv)
        return(s);
    s.v = **(U08***)argv;
    s.len = strlen(**argv);
    --(*argc);
    ++(*argv);
    str_check_invariants(s);
    return(s);
}

U64 str_hash(Str s) {
    return(xxh64(s.v, s.len, 0));
}

// hash

// impl stolen from
// https://github.com/demetri/scribbles/blob/master/hashing/

U64 xxh64(const void *key, S64 len, U64 h) {
  // primes used in mul-rot updates
  U64 p1 = 0x9e3779b185ebca87, p2 = 0xc2b2ae3d27d4eb4f,
    p3 = 0x165667b19e3779f9, p4 =0x85ebca77c2b2ae63, p5 = 0x27d4eb2f165667c5;

  // inital 32-byte (4x8) wide hash state
  U64 s[4] = {h+p1+p2, h+p2, h, h-p1};

  // bulk work: process all 32 byte blocks 
  for (S64 i=0; i < (len/32); i++) {
    U64 b[4];
    memcpy(b, key + 4*i, sizeof(b));

    for (S64 j=0;j<4;j++) b[j] = b[j]*p2+s[j];
    for (S64 j=0;j<4;j++) s[j] = ((b[j] << 31) | (b[j] >> 33))*p1;
  }

  // mix 32-byte state down to 8-byte state, initalize to value for short keys
  U64 s64 = (s[2] + p5);
  if (len >= 32) {
    s64  = ((s[0] << 1)  | (s[0] >> 63)) + ((s[1] << 7)  | (s[1] >> 57)) +
           ((s[2] << 12) | (s[2] >> 52)) + ((s[3] << 18) | (s[3] >> 46));
    for (S64 i=0; i<4;i++) {
      U64 ps = (((s[i]*p2) << 31) | ((s[i]*p2) >> 33))*p1;
      s64 = (s64 ^ ps)*p1 + p4;
    }
  }
  s64 += len;

  // up to 31 bytes remain, process 0-3 8 byte blocks
  U08 *tail = (U08 *) (key + (len/32)*32);
  for (S64 i=0;i < (len & 31) / 8; i++,tail+=8) {  
    U64 b;
    memcpy(&b, tail, sizeof(U64));

    b *= p2;
    b = (((b << 31)| (b >> 33))*p1) ^ s64;
    s64 = ((b << 27) | (b >> 37))*p1 + p4;
  }

  // up to 7 bytes remain, process 0-1 4 byte block
  for (S64 i=0;i< (len & 7) / 4; i++, tail +=4) {
    U64 b;
    memcpy(&b, tail, sizeof(b));

    b = (s64 ^ b) * p1;
    s64 = ((b << 23) | (b >> 41))*p2 + p3;
  }

  // up to 3 bytes remain, process 0-3 1 byte blocks
  for (S64 i=0;i<(len & 3); i++,tail++) {
    U64 b = s64 ^ (*tail)*p5;
    s64 = ((b << 11) | (b >> 53))*p1;
  }

  // finalization mix
  s64 =  (s64 ^ (s64 >> 33))*p2;
  s64 =  (s64 ^ (s64 >> 29))*p3;
  return (s64 ^ (s64 >> 32)); 
}

// bijective hash function

U64 u64_hash_bij(U64 x) {
    x = (x ^ (x >> 30ULL)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27ULL)) * 0x94d049bb133111ebULL;
    x = x ^ (x >> 31ULL);
    return(x);
}

U64 u64_unhash_bij(U64 x) {
    x = (x ^ (x >> 31ULL) ^ (x >> 62ULL)) * 0x319642b2d24d8ec3ULL;
    x = (x ^ (x >> 27ULL) ^ (x >> 54ULL)) * 0x96de1b173f119089ULL;
    x =  x ^ (x >> 30ULL) ^ (x >> 60ULL);
    return(x);
}

/* Example MSI Hash Table
* refer to skeeto @ nullprogram.com for more info
#define EXP 5
struct MSIHashTable {
    Str *table[1 << (EXP)];
    S64 len;
};
*/

S64 msi_lookup(U64 hash, S64 exp, S64 idx) {
    assert(0 <= exp);
    assert(0 <= idx);
    assert(idx < (1 << exp));
    U32 mask = ((U32)1 << exp) - 1;
    U32 step = (hash << (64 - exp)) | 1;
    S64 next = (idx + step) & mask;
    return(next);
}

#endif//BASE_IMPL
