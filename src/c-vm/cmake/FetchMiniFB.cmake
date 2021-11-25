include(FetchContent)

FetchContent_Declare(
        minifb
        GIT_REPOSITORY https://github.com/emoon/minifb.git
        GIT_TAG        f54b94a
        GIT_SHALLOW    1
)

FetchContent_MakeAvailable(minifb)

Set(MINIFB_BUILD_EXAMPLES FALSE CACHE BOOL "Skip build of minifb example programs")
include_directories(${minifb_SOURCE_DIR}/include)

