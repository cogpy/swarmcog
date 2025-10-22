cmake_minimum_required(VERSION 3.16)
project(SwarmCog VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build test suite" ON)
option(BUILD_PYTHON_BINDINGS "Build Python bindings" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(include)

# Core library
set(SWARMCOG_CORE_SOURCES
    src/agentspace.cpp
    src/microkernel.cpp
    src/cognitive_agent.cpp
    src/swarmcog.cpp
    src/utils.cpp
)

set(SWARMCOG_CORE_HEADERS
    include/swarmcog/agentspace.h
    include/swarmcog/microkernel.h
    include/swarmcog/cognitive_agent.h
    include/swarmcog/swarmcog.h
    include/swarmcog/types.h
    include/swarmcog/utils.h
)

# Create core library
add_library(swarmcog_core ${SWARMCOG_CORE_SOURCES} ${SWARMCOG_CORE_HEADERS})
target_link_libraries(swarmcog_core Threads::Threads)

# Set library properties
set_target_properties(swarmcog_core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Install targets
install(TARGETS swarmcog_core
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/swarmcog/
    DESTINATION include/swarmcog
    FILES_MATCHING PATTERN "*.h"
)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Python bindings
if(BUILD_PYTHON_BINDINGS)
    add_subdirectory(bindings)
endif()