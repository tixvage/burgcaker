#include <raylib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "buffer.h"
#include "minilua.h"
#include "config.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

/*
 * IDEA
 * lua scripts gonna work like addons for editor
 * c side will contain:
 * cursor system
 * openning saving files
 * window system (splitting buffers)
 * different types of buffers:
 * default buffer (normal text files)
 * mini buffer (ESC ':' in vim & M-x in emacs)
 * popup buffer (like fzf in vim)
 * 'main addon of lua' will contain:
 * file navigation
 * switching between buffers
 * etc...
 */


int inc(lua_State *L) {
    float rtrn = lua_tonumber(L, -1);
    lua_pushnumber(L, rtrn + 1);
    return 1;
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "uno");
    Font font = LoadFontEx("font/Source-Code-Pro.ttf", FONT_SIZE, NULL, 1568);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    Buffer buffer1 = create_buffer_from_file("text.txt", font, global_allocator);

    Buffer *buffer = &buffer1;

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    lua_register(L, "inc", inc);  
    luaL_dofile(L, "script.lua");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
    
        int input_char = GetCharPressed();
        if (input_char != 0) {
            int len = 0;
            const char *bytes = CodepointToUTF8(input_char, &len);
            buffer_insert(buffer, bytes, len);
        }

        int input_key = GetKeyPressed();

        if (input_key == KEY_ENTER) {
            buffer_insert(buffer, "\n", 1);
        }
        if (input_key == KEY_BACKSPACE) {
            buffer_backspace(buffer);
        }
        if (input_key == KEY_LEFT) {
            buffer_left(buffer);
        }
        if (input_key == KEY_RIGHT) {
            buffer_right(buffer);
        }
        buffer_draw(&buffer1, (Vector2){0, 0}, 1, 2);
        buffer_draw(&buffer1, (Vector2){0, 1}, 1, 2);
        buffer_draw(&buffer1, (Vector2){1, 1}, 2, 2);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

