#define NOBUILD_IMPLEMENTATION
#include "./nobuild.h"

#define COMMON_CFLAGS "-Wall", "-Wextra", "-Werror", "-Wno-sign-compare", "-pedantic", "-std=c11", "-ggdb", "-O0", "-I.", "-Ilibs/raylib/src"
#define CC "clang"
#define SRCS "main.c", "buffer.c", "common.c"

const char *program = NULL;

void usage(void) {
    INFO("Usage: %s [<subcommand>]", program);
    INFO("Subcommands:");
    INFO("    build");
    INFO("        Build program");
    INFO("    build-raylib");
    INFO("        Build raylib");
}

void build(void) {
    if (!path_exists("libs/raylib/src/libraylib.a")) {
        usage();
        PANIC("First of all go and build raylib");
    }
    MKDIRS("build");
    CMD(CC, COMMON_CFLAGS, "-o", "./build/uno", SRCS, "-lm", "-Llibs/raylib/src/", "-lraylib");
}

void build_raylib(void) {
    MKDIRS("build");
    CMD("cd", "libs/raylib/src");
    CMD("make", "-C", "libs/raylib/src");
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
        } else {
            usage();
            PANIC("Unknown command `%s`", subcmd);
        }
    } else {
        build_raylib();
        build();
        run();
    }

    return 0;
}
