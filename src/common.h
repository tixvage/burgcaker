#ifndef _COMMON_H
#define _COMMON_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef void *(* Alloc_Func)(unsigned long);
typedef void *(* Realloc_Func)(void *, unsigned long);
typedef void  (* Free_Func)(void *);

typedef struct Allocator {
    Alloc_Func alloc_func;
    Realloc_Func realloc_func;
    Free_Func free_func;
} Allocator;

extern Allocator global_allocator;

void *global_alloc(int size);
void *global_realloc(void *ptr, int size);
void global_free(void *ptr);

typedef struct String_View {
    char *ptr;
    int len;
} String_View;

typedef struct String {
    char *data;
    int len;
    int capacity;
    Allocator allocator;
} String;

#define def_array_struct(N, T) \
    typedef struct N {\
        T *data; \
        int len; \
        int capacity; \
        Allocator allocator; \
    } N

#define array_append(a, d, l) \
    do { \
        int __len = l; \
        while (a.capacity <= a.len + __len) a.capacity += 250; \
        if (a.data == NULL) { \
            a.data = a.allocator.alloc_func((a.capacity) * sizeof(a.data[0])); \
            for (int __i = 0; __i < __len; __i++) { a.data[__i] = d[__i]; } \
            a.len += __len; \
        } else { \
            a.data = a.allocator.realloc_func(a.data, a.capacity * sizeof(a.data[0])); \
            for (int __i = 0; __i < __len; __i++) { a.data[a.len + __i] = d[__i]; } \
            a.len += __len; \
        } \
    } while(0)

#define array_push(a, T, d) do {  T ___v = d; T *___p = &___v; array_append(a, ___p, 1); } while(0)

#define array_free(a) do { a.allocator.free_func(a.data); a.data = NULL; a.len = 0; a.capacity = 0; } while(0)

#define array_copy(a, T) ({ T ___na = a; ___na.data = ___na.allocator.alloc_func((___na.capacity) * sizeof(___na.data[0])); memcpy(___na.data, a.data, sizeof(___na.data[0]) * ___na.len); ___na; })

#define str_append_cstr(str, cstr) do { int ___len = strlen(cstr); array_append(str, cstr, ___len); } while(0)

#define str_global_allocator() \
    { \
        .data = NULL, \
        .len = 0, \
        .capacity = 0, \
        .allocator = global_allocator, \
    }

String read_entire_file(const char *filename, Allocator allocator);

#endif
