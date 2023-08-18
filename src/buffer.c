#include "buffer.h"
#include <stdio.h>
#include <assert.h>
#include "config.h"

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
    buffer.font = default_font;
    buffer.font_size = default_font.baseSize;

    int monitor = GetCurrentMonitor();
    buffer.texture = LoadRenderTexture(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
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

int get_digit_count(int n) {
    int count = 0;
    
    while(n>0){
        count++;
        n = n/10;
    }

    return count;
}

void prepare_buffer(Buffer *b) {
    BeginTextureMode(b->texture);

    ClearBackground(BACKGROUND_COLOR);
    float offset_x = 0;
    {
        int total_lines = b->lines.len;
        int max_digit = get_digit_count(total_lines);
        char *curr_line_number = b->allocator.alloc_func(max_digit + 1);
        memset(curr_line_number, 0, max_digit + 1);
        float y = 0;
        for (int i = 1; i <= total_lines; i++) {
            int digit = get_digit_count(i);
            for (int j = 0; j < max_digit - digit; j++) {
                curr_line_number[j] = '0';
            }
            snprintf(&curr_line_number[max_digit - digit], max_digit + 1, "%d", i);

            float x = 0;
            for (int j = 0; j < max_digit; j++) {
                int c = curr_line_number[j];
                int index = GetGlyphIndex(b->font, c);
                DrawTextCodepoint(b->font, c, (Vector2){ x, y }, b->font_size, (Color){ 0x68, 0x68, 0x68, 0xff });
                if (b->font.glyphs[index].advanceX == 0) x += (float)b->font.recs[index].width;
                else x += (float)b->font.glyphs[index].advanceX;
            }
            offset_x = x + b->font_size / 2;
            y += b->font_size;

            memset(curr_line_number, 0, max_digit + 1);
        }
        b->allocator.free_func(curr_line_number);
    }

    for (int i = 0; i < b->cursors.len; i++) {
        Cursor c = b->cursors.data[i];
        int y = buffer_get_cursor_line(b, c);
        Line line = b->lines.data[y];
        String str = str_global_allocator();
        char *ptr = b->data.data + line.begin;
        array_append(str, ptr, c - line.begin);
        array_push(str, char, 0);
        int x = MeasureTextEx(b->font, str.data, b->font_size, 0).x;
        array_free(str);
        Rectangle cursor_rect = {
            .x = x + offset_x,
            .y = y * b->font_size,
            //TODO: this must be something better
            .width = b->font_size / 2,
            .height = b->font_size,
        };
        DrawRectangleRec(cursor_rect, (Color){ 0, 210, 30, 50 });
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
                DrawTextCodepoint(b->font, letter, (Vector2){ x + offset_x, y }, b->font_size, (Color){ 0xbc, 0x7c, 0x2c, 0xff });
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

void buffer_draw(Buffer *b, Vector2 split_position, float w_f, float h_f) {
    Texture2D texture = b->texture.texture;
    float s_w = GetScreenWidth();
    float s_h = GetScreenHeight();
    float height = s_h / h_f;
    //TODO: we need to calculate y according to cursor position
    Rectangle source = { 0, texture.height - height, s_w / w_f, -height};
    Vector2 position = { 0 };
    if (split_position.x != 0) {
        position.x = s_w / w_f;
    }
    if (split_position.y != 0) {
        position.y = s_h / h_f;
    }
    Rectangle destination = { position.x, position.y, source.width, -source.height };
    DrawTexturePro(texture, source, destination, (Vector2){ 0 }, 0, WHITE);
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
        //TODO: dos files won't like that
        return c - 1;
    }

    return 0;
}

int buffer_find_next_char_loc(Buffer *b, Cursor c) {
    int column = buffer_get_cursor_line(b, c);
    Line line = b->lines.data[column];
    String_View line_str = {b->data.data + c, line.end - c};
    if (line_str.len != 0) {
        char *ptr = (char *)&line_str.ptr[0];
        int next = 0;
        GetCodepointNext(ptr, &next);

        return c + next;
    } else if (column != b->lines.len - 1) {
        //TODO: dos files won't like that
        return c + 1;
    }

    return 0;
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
