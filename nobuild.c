#define NOBUILD_IMPLEMENTATION
#include "./nobuild.h"

#define COMMON_CFLAGS "-Wall", "-Wextra", "-Werror", "-Wno-sign-compare", "-Wno-gnu-label-as-value", "-pedantic", "-std=c11", "-ggdb", "-O0", "-Isrc/", "-Ilibs/raylib/src/", "-Ilibs/minilua/"
#define CC "clang"
#define SRCS "src/main.c", "src/buffer.c", "src/common.c"

const char *program = NULL;

void usage(void) {
    INFO("Usage: %s [<subcommand>]", program);
    INFO("Subcommands:");
    INFO("    build");
    INFO("        Build program");
    INFO("    build-raylib");
    INFO("        Build raylib");
    INFO("    build-minilua");
    INFO("        Build minilua");
}

void build(void) {
    if (!path_exists("libs/raylib/src/libraylib.a")) {
        usage();
        PANIC("First of all go and build raylib");
    }
    if (!path_exists("libs/minilua/minilua.o")) {
        usage();
        PANIC("First of all go and build minilua");
    }
    MKDIRS("build");
    CMD(CC, COMMON_CFLAGS, "-o", "./build/uno", SRCS, "libs/minilua/minilua.o", "-lm", "-Llibs/raylib/src/", "-lraylib");
}

void build_raylib(void) {
    CMD("make", "-C", "libs/raylib/src");
}

void build_minilua(void) {
    CMD(CC, "-c", "libs/minilua/minilua.c", "-Ilibs/minilua/", "-o", "libs/minilua/minilua.o");
}

void run(void) {
    CMD("./build/uno");
}

int main(int argc, char **argv) {
    GO_REBUILD_URSELF(argc, argv);

    program = shift_args(&argc, &argv);
    if (argc > 0) {
        const char *subcmd = shift_args(&argc, &argv);
        if (strcmp(subcmd, "build") == 0) {
            build();
        } else if (strcmp(subcmd, "build-raylib") == 0) {
            build_raylib();
        } else if (strcmp(subcmd, "build-minilua") == 0) {
            build_minilua();
        } else {
            usage();
            PANIC("Unknown command `%s`", subcmd);
        }
    } else {
        build();
        run();
    }

    return 0;
}
