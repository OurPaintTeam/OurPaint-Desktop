include(GoogleTest)

add_custom_target(ourpaint_tests)

function(add_ourpaint_test name)
    add_executable(${name} ${ARGN})
    target_compile_features(${name} PRIVATE cxx_std_20)
    target_link_libraries(${name} PRIVATE GTest::gtest_main)
    add_dependencies(ourpaint_tests ${name})
    gtest_discover_tests(${name}
        DISCOVERY_MODE PRE_TEST
        TEST_PREFIX "${name}."
        PROPERTIES LABELS ourpaint TIMEOUT 120
    )
endfunction()
