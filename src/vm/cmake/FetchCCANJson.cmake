include(FetchContent)

FetchContent_Declare(
        ccan_json
        URL         http://ccodearchive.net/tarballs/json.tar.bz2
)

FetchContent_MakeAvailable(ccan_json)

include_directories(${ccan_json_SOURCE_DIR})
set(CCAN_JSON ${ccan_json_SOURCE_DIR}/ccan/json/json.c ${ccan_json_SOURCE_DIR}/ccan/json/json.h)