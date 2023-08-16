#include "buffer.h"
#include <stdio.h>
#include <assert.h>

Lines init_lines(Allocator allocator) {
    return (Lines){ NULL, 0, 0, allocator };
}

void swap(Cursor *xp, Cursor *yp) {
    Cursor temp = *xp;
    *xp = *yp;
    *yp = temp;
}
  
void selection_sort(Cursor *arr, int n) {
    Cursor i, j, min_idx;
    for (i = 0; i < n - 1; i++) {
        min_idx = i;
        for (j = i + 1; j < n; j++) if (arr[j] < arr[min_idx]) min_idx = j;
        swap(&arr[min_idx], &arr[i]);
    }
}

void add_cursor(Buffer *b, Cursor c) {
    array_push(b->cursors, Cursor, c);
    selection_sort(b->cursors.data, b->cursors.len);
}

Buffer create_buffer_from_file(const char *path, Font default_font, Allocator allocator) {
    Buffer buffer = {0};
    buffer.data = read_entire_file(path, allocator);
    buffer.lines = init_lines(allocator);
    buffer.cursors.allocator = allocator;
    add_cursor(&buffer, 0);
    add_cursor(&buffer, 3);
    add_cursor(&buffer, 6);
    add_cursor(&buffer, 9);
    add_cursor(&buffer, 12);
    add_cursor(&buffer, 15);
    add_cursor(&buffer, 18);
    buffer.font = default_font;
    buffer.font_size = default_font.baseSize;
    buffer.texture = LoadRenderTexture(800, 600);
    buffer.allocator = allocator;
    buffer_refresh(&buffer);
    return buffer;
}

void check_cursors_buffer(Buffer *b) {
    Cursors new_cursors = {NULL, 0, 0, b->allocator };
    int unique_count = 0;
    for (int i = 0; i < b->cursors.len; i++) {
        bool unique = true;
        for (int j = 0; j < unique_count; j++) {
            if (new_cursors.data[j] == b->cursors.data[i]) {
                unique = false;
                break;
            }
        }
        if (unique) {
            array_push(new_cursors, Cursor, b->cursors.data[i]);
            unique_count++;
        }
    }

    array_free(b->cursors);
    b->cursors = new_cursors;
}

void tokenize_buffer(Buffer *b) {
    b->lines.len = 0;
    int begin = 0;
    int end = 0;
    const char *ptr = b->data.data;
    for (int i = 0; i < b->data.len; i++) {
        int next = 0;
        int letter = GetCodepointNext(ptr, &next);
        ptr += next;
        i += next - 1;
        if (letter == '\n') {
            Line line = { begin, end };
            Line *lines = &line;
            array_append(b->lines, lines, 1);
            begin = end + next;
        }
        end += next;
    }
    Line line = { begin, b->data.len };
    array_push(b->lines, Line, line);
}

void prepare_buffer(Buffer *b) {
    BeginTextureMode(b->texture);

    ClearBackground(BLACK);
    
    for (int i = 0; i < b->cursors.len; i++) {
        Cursor c = b->cursors.data[i];
        int y = buffer_get_cursor_line(b, c);
        Line line = b->lines.data[y];
        String str = str_global_allocator();
        char *ptr = b->data.data + line.begin;
        array_append(str, ptr, c - line.begin);
        array_push(str, char, 0);
        int x = MeasureTextEx(b->font, str.data, b->font_size, 0).x;
        Rectangle cursor_rect = {
            .x = x,
            .y = y * b->font_size,
            .width = b->font_size / 2,
            .height = b->font_size,
        };
        DrawRectangleRec(cursor_rect, (Color){0, 210, 30, 50});
    }

    float y = 0;
    for (int j = 0; j < b->lines.len; j++) {
        Line line = b->lines.data[j];
        String_View line_str = {b->data.data + line.begin, line.end - line.begin};

        float x = 0;
        char *ptr = (char *)&line_str.ptr[0];
        for (int i = 0; i < line_str.len; i++) {
            int next = 0;
            int letter = GetCodepointNext(ptr, &next);
            int index = GetGlyphIndex(b->font, letter);
            ptr += next;
            i += next - 1;

            assert(letter != '\n');
            if ((letter != ' ') && (letter != '\t')) {
                DrawTextCodepoint(b->font, letter, (Vector2){ x, y }, b->font_size, WHITE);
            }

            if (b->font.glyphs[index].advanceX == 0) x += (float)b->font.recs[index].width;
            else x += (float)b->font.glyphs[index].advanceX;
        }

        y += b->font_size;
    }

    EndTextureMode();
}

void buffer_refresh(Buffer *b) {
    check_cursors_buffer(b);
    tokenize_buffer(b);
    prepare_buffer(b);
}

void buffer_draw(Buffer *b, Vector2 position) {
    Texture2D texture = b->texture.texture;
    Rectangle source = { 0, 0, texture.width, -texture.height };
    DrawTextureRec(texture, source, position, WHITE);
}

int buffer_get_cursor_line(Buffer *b, Cursor c) {
    for (int i = 0; i < b->lines.len; i++) {
        Line line = b->lines.data[i];
        if (line.begin <= c && line.end >= c) {
            return i;
        }
    }
    return 0;
}

Vector2 buffer_get_cursor_position(Buffer *b, Cursor c) {
    int y = buffer_get_cursor_line(b, c);
    Line line = b->lines.data[y];

    String_View line_str = {b->data.data + line.begin, c - line.begin};
    char *ptr = (char *)&line_str.ptr[0];
    int x = 0;
    for (int i = 0; i < line_str.len; i++) {
        int next = 0;
        GetCodepointNext(ptr, &next);
        ptr += next;
        i += next - 1;
        x++;
    }

    return (Vector2){x, y};
}

int buffer_find_previous_char_loc(Buffer *b, Cursor c) {
    int column = buffer_get_cursor_line(b, c);
    Line line = b->lines.data[column];
    String_View line_str = {b->data.data + line.begin, c - line.begin};

    if (line_str.len != 0) {
        char *ptr = (char *)&line_str.ptr[0];
        int last_one = 0;
        for (int i = 0; i < line_str.len; i++) {
            int next = 0;
            GetCodepointNext(ptr, &next);
            ptr += next;
            i += next - 1;
            last_one = i - next + 1;
        }

        return line.begin + last_one;
    } else if (column != 0) {
        return c - 1;
    }

    return 0;
}

int buffer_find_next_char_loc(Buffer *b, Cursor c) {
    int column = buffer_get_cursor_line(b, c);
    Line line = b->lines.data[column];
    String_View line_str = {b->data.data + c, c - line.end};
    char *ptr = (char *)&line_str.ptr[0];
    int next = 0;
    GetCodepointNext(ptr, &next);

    return c + next;
}

void buffer_insert(Buffer *b, const char *d, int len) {
    int total = 0;
    for (int i = 0; i < b->cursors.len; i++) {
        b->cursors.data[i] += total;
        Cursor c = b->cursors.data[i];
        for (int j = 0; j < len; j++) array_push(b->data, char, '\0');
        memmove(
            &b->data.data[c + len],
            &b->data.data[c],
            b->data.len - c - len
        );
        memcpy(&b->data.data[c], d, len);
        b->cursors.data[i] += len;
        total += len;

        tokenize_buffer(b);
    }
    check_cursors_buffer(b);
    prepare_buffer(b);
}

void buffer_backspace(Buffer *b) {
    int total = 0;
    for (int i = 0; i < b->cursors.len; i++) {
        Cursor *c = &b->cursors.data[i];
        *c -= total;
        if (*c > b->data.len) {
            *c = b->data.len;
        }
        if (*c == 0) continue;

        int p_c = buffer_find_previous_char_loc(b, *c);

        memmove(
            &b->data.data[p_c],
            &b->data.data[*c],
            b->data.len - *c
        );
        b->data.len -= *c - p_c;
        total += *c - p_c;
        *c = p_c;

        tokenize_buffer(b);
    }
    check_cursors_buffer(b);
    prepare_buffer(b);
}

void buffer_left(Buffer *b) {
    for (int i = 0; i < b->cursors.len; i++) {
        Cursor *c = &b->cursors.data[i];
        if (*c > 0) *c = buffer_find_previous_char_loc(b, *c);
    }
    check_cursors_buffer(b);
    prepare_buffer(b);
}

void buffer_right(Buffer *b) {
    for (int i = 0; i < b->cursors.len; i++) {
        Cursor *c = &b->cursors.data[i];
        if (*c < b->data.len) *c = buffer_find_next_char_loc(b, *c);
    }
    check_cursors_buffer(b);
    prepare_buffer(b);
}