#define NOBUILD_IMPLEMENTATION
#include "./nobuild.h"

#define COMMON_CFLAGS "-Wall", "-Wextra", "-Werror", "-Wno-sign-compare", "-pedantic", "-std=c11", "-ggdb", "-O0", "-I."
#define CC "clang"
#define SRCS "main.c", "buffer.c", "common.c"

void build(void) {
    MKDIRS("build");
    CMD(CC, COMMON_CFLAGS, "-o", "./build/uno", SRCS, "-lm", "-lraylib");
}

void run(void) {
    CMD("./build/uno");
}

void usage(const char *program) {
    INFO("Usage: %s [<subcommand>]", program);
    INFO("Subcommands:");
    INFO("    build");
    INFO("        Build program");
    INFO("    run");
    INFO("        Run program");
}

int main(int argc, char **argv) {
    GO_REBUILD_URSELF(argc, argv);

    const char *program = shift_args(&argc, &argv);
    if (argc > 0) {
        const char *subcmd = shift_args(&argc, &argv);
        if (strcmp(subcmd, "build") == 0) {
            build();
        } else {
            usage(program);
            PANIC("Unknown command `%s`", subcmd);
        }
    } else {
        build();
        run();
    }

    return 0;
}
