-- BUILD SYSTEM API
function array_concat(...) 
    local t = {}
    for n = 1,select("#",...) do
        local arg = select(n,...)
        if type(arg)=="table" then
            for _,v in ipairs(arg) do
                t[#t+1] = v
            end
        else
            t[#t+1] = arg
        end
    end
    return t
end

local program = "./lua build.lua"

local function get_os()
    if package.config:sub(0, 1) == '/' then return "unix" else return "win" end
end

local function path_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

local function info(f, ...)
    f = "[INFO] " .. f .. "\n"
    io.write(f:format(...))
end

local function panic(f, ...)
    f = "[PANIC] " .. f .. "\n"
    io.write(f:format(...))
    os.exit(1)
end

local function path(p)
    if get_os() == "win" then return p:gsub("\\", "/") else return p end
end

local function exec(args)
    local as_string = table.concat(array_concat(table.unpack(args)), " ")
    info("CMD: %s", as_string)
    local success, _ = os.execute(as_string)
    if success == nil then panic("Command failed") end
end

local function mkdir(p)
    exec({"mkdir", "-p", p})
end

local function run()
    exec({path("build/burgcaker")});
end

-- BUILD SCRIPT
local CC = "clang"
local CFLAGS = {"-Wall", "-Wextra", "-Werror", "-Wno-sign-compare", "-Wno-gnu-label-as-value", "-pedantic", "-std=c11", "-ggdb", "-O0", "-Isrc/", "-Ilibs/raylib/src/", "-Ilibs/minilua/"}
local LIBS = {path("libs/minilua/minilua.o"), "-lm", "-Llibs/raylib/src/", "-lraylib"}
local SRCS = {path("src/main.c"), path("src/buffer.c"), path("src/common.c")}

local function usage()
    info("Usage: %s [<subcommand>]", program)
    info("Subcommands:")
    info("    build")
    info("        Build program")
    info("    build-raylib")
    info("        Build raylib")
    info("    build-minilua")
    info("        Build minilua")
end

local function build()
    if not path_exists(path("libs/raylib/src/libraylib.a")) then
        usage()
        panic("First of all go and build raylib")
    end
    if not path_exists(path("libs/minilua/minilua.o")) then
        usage()
        panic("First of all go and build minilua")
    end
    mkdir("build")
    exec({CC, CFLAGS, "-o", path("build/burgcaker"), SRCS, LIBS})
end

local function build_raylib()
    exec({"make", "-C", path("libs/raylib/src")})
end

local function build_minilua()
    mkdir("build");
    exec({CC, "-c", path("libs/minilua/minilua.c"), "-Ilibs/minilua/", "-o", path("libs/minilua/minilua.o")});
end

local function main()
    subcmd = arg[1]
    if subcmd ~= nil then
        if subcmd == "build" then
            build()
        elseif subcmd == "build-raylib" then
            build_raylib()
        elseif subcmd == "build-minilua" then
            build_minilua()
        else usage() end
    else
        build()
        run()
    end
end

main()
