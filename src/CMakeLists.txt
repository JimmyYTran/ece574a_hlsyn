set(PROJECT_SOURCES
    fileio.cpp
    graph.cpp
    Data.cpp
    Operation.cpp
    # parser.cpp
    parser2.cpp
)

add_library(hlsyn_lib ${PROJECT_SOURCES} "include/writeVerilog.h")
add_executable(hlsyn hlsyn.cpp "include/writeVerilog.h")
target_link_libraries(hlsyn hlsyn_lib)