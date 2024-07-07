
find_package(doctest REQUIRED)


# create macro to create test given the source file and libs to link to
macro(create_test test_name test_src test_libs)
    add_executable(${test_name} ${test_src})
    target_compile_definitions(${test_name} PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)
    target_link_libraries(${test_name} PRIVATE ${test_libs} doctest::doctest)
    add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()


# create tests
create_test(test_page test_page.cpp page)
create_test(test_lru test_lru.cpp lru)