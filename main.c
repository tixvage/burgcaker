#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "buffer.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BACKGROUND_COLOR (Color){28,28,28,255}

#define FONT_SIZE 48

void draw_text(Font font, String text, Vector2 position, float font_size, Color color) {
    char *ptr = (char *)&text.data[0];

    float x = position.x;
    float y = position.y;

    for (int i = 0; i < text.len; i++) {
        int next = 0;
        int letter = GetCodepointNext(ptr, &next);
        int index = GetGlyphIndex(font, letter);
        ptr += next;
        i += next - 1;

        if (letter == '\n') {
            y += font_size;
            x = position.x;
            continue;
        }

        if ((letter != ' ') && (letter != '\t')) {
            DrawTextCodepoint(font, letter, (Vector2){ x, y }, font_size, color);
        }

        if (font.glyphs[index].advanceX == 0) x += (float)font.recs[index].width;
        else x += (float)font.glyphs[index].advanceX;
    }
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "uno");
    Font font = LoadFontEx("font.ttf", FONT_SIZE, NULL, 1568);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    Buffer buffer = create_buffer_from_file("text.txt", font, global_allocator);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
    
        int input_char = GetCharPressed();
        if (input_char != 0) {
            int len = 0;
            const char *bytes = CodepointToUTF8(input_char, &len);
            buffer_insert(&buffer, bytes, len);
        }

        int input_key = GetKeyPressed();

        if (input_key == KEY_ENTER) {
            buffer_insert(&buffer, "\n", 1);
        }
        if (input_key == KEY_BACKSPACE) {
            buffer_backspace(&buffer);
        }
        if (input_key == KEY_LEFT) {
            buffer_left(&buffer);
        }
        if (input_key == KEY_RIGHT) {
            buffer_right(&buffer);
        }
        buffer_draw(&buffer, (Vector2){20, 20});
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

