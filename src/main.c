#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "buffer.h"

#include "minilua.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BACKGROUND_COLOR (Color){28,28,28,255}

#define FONT_SIZE 48

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "uno");
    Font font = LoadFontEx("font/Source-Code-Pro.ttf", FONT_SIZE, NULL, 1568);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    Buffer buffer = create_buffer_from_file("text.txt", font, global_allocator);

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_dofile(L, "script.lua");

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

