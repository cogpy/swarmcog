# Test suite for SwarmCog

# Create a simple test executable
add_executable(swarmcog_test
    test_basic.cpp
)

target_link_libraries(swarmcog_test 
    swarmcog_core
    Threads::Threads
)

# Add the test
add_test(NAME BasicTest COMMAND swarmcog_test)