# Test source files
set(TEST_SOURCES
    test_bit_value.c
    test_bit_stream.c
    test_bit_stream_reader_writer.c
)

# Create test executables
foreach(test_source ${TEST_SOURCES})
    # Get the test name from the source file (without extension)
    get_filename_component(test_name ${test_source} NAME_WE)
    
    # Add executable
    add_executable(${test_name} ${test_source})
    
    # Link with the library and Unity
    target_link_libraries(${test_name} bit_stream unity)
    
    # Add test
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()