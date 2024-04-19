set(PROJECT_SOURCES
    fileio.cpp
    schedule.cpp
    Data.cpp
    Operation.cpp
)

add_library(hlsyn_lib ${PROJECT_SOURCES})
add_executable(hlsyn hlsyn.cpp)
target_link_libraries(hlsyn hlsyn_lib)