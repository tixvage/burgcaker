#ifndef _BUFFER_H
#define _BUFFER_H

#include <raylib.h>
#include "common.h"

typedef struct Line {
    int begin;
    int end;
} Line;
def_array_struct(Lines, Line);

Lines init_lines(Allocator allocator);

typedef size_t Cursor;
def_array_struct(Cursors, Cursor);

typedef struct Buffer {
    String data;
    Lines lines;
    Cursors cursors;
    RenderTexture2D texture;
    Font font;
    int font_size;
    Allocator allocator;
} Buffer;

Buffer create_buffer_from_file(const char *path, Font default_font, Allocator allocator);
void buffer_refresh(Buffer *b);
void buffer_draw(Buffer *b, Vector2 position);

int buffer_get_cursor_line(Buffer *b, Cursor c);
Vector2 buffer_get_cursor_position(Buffer *b, Cursor c);
int buffer_find_previous_char_loc(Buffer *b, Cursor c);
int buffer_find_next_char_loc(Buffer *b, Cursor c);

void buffer_insert(Buffer *b, const char *d, int len);
void buffer_backspace(Buffer *b);
void buffer_left(Buffer *b);
void buffer_right(Buffer *b);

#endif
