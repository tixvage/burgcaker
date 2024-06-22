#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void *g_malloc(unsigned long size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        assert("Allocation failed" && 0);
    }
    return ptr;
}

void *g_realloc(void *ptr, unsigned long size) {
    void *ptr_ = realloc(ptr, size);
    if (ptr_ == NULL) {
        assert("Reallocation failed" && 0);
    }
    return ptr_;
}

void g_free(void *ptr) {
    free(ptr);
}

Allocator global_allocator = { g_malloc, g_realloc, g_free };

void *global_alloc(int size) {
    return global_allocator.alloc(size);
}

void *global_realloc(void *ptr, int size) {
    return global_allocator.realloc(ptr, size);
}

void global_free(void *ptr) {
    global_allocator.free(ptr);
}

String read_entire_file(const char *filename, Allocator allocator) {
    FILE* f;
    char* text = NULL;
    long len;

    f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "error: failed to open file at `%s`\n", filename);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    assert(len > 0);
    fseek(f, 0, SEEK_SET);
    text = allocator.alloc(len + 1);
    assert(text != NULL);
    fread(text, 1, len, f);
    text[len] = 0;
    assert(strlen(text) > 0);
    fclose(f);

    String str = { NULL, 0, 0, allocator };
    str_append_cstr(str, text);

    allocator.free(text);

    return str;
}
