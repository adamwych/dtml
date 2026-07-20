cmake_minimum_required(VERSION 4.0)

project(dtml LANGUAGES CXX)

option(dtml_BUILD_TESTS "Build dtml tests" OFF)
option(dtml_BUILD_BENCHMARKS "Build dtml benchmarks" OFF)

set(SOURCES
    src/bindings.cc
    src/tel.cc
    src/tel-expr-parser.cc
    src/tel-json.cc
    src/tpl.cc
    src/tpl-cursor.cc
    src/tpl-printer.cc
)

if(EMSCRIPTEN)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)

    add_executable(dtml ${SOURCES})

    set_target_properties(dtml PROPERTIES SUFFIX ".js")

    target_sources(dtml PRIVATE src/fetch-emscripten.cc)
    target_compile_definitions(dtml PRIVATE TARGET_EMSCRIPTEN)

    string(CONCAT dtml_EXPORTED_FUNCTIONS
        "["
        "\"_dtml_Template_New\","
        "\"_dtml_Template_Free\","
        "\"_dtml_TemplateCache_New\","
        "\"_dtml_TemplateCache_Free\","
        "\"_dtml_TemplateCache_Clear\","
        "\"_dtml_TemplateEvaluationContext_New\","
        "\"_dtml_TemplateEvaluationContext_Free\","
        "\"_dtml_TemplateEvaluator_New\","
        "\"_dtml_TemplateEvaluator_Free\","
        "\"_dtml_TemplateEvaluator_Evaluate\","
        "\"_dtml_TemplateEvaluationResult_Free\","
        "\"_dtml_TemplateEvaluationResult_GetText\","
        "\"_dtml_TemplateEvaluationResult_IsError\","
        "\"_dtml_TemplateEvaluationResult_GetErrorLocationLine\","
        "\"_dtml_TemplateEvaluationResult_GetErrorLocationColumn\""
        "]"
    )

    target_link_options(dtml PRIVATE
        "SHELL:-sMODULARIZE=1"
        "SHELL:-sEXPORT_NAME=createdtmlModule"
        "SHELL:-sENVIRONMENT=web,worker"
        "SHELL:-sEXPORTED_FUNCTIONS=${dtml_EXPORTED_FUNCTIONS}"
        "SHELL:-sEXPORTED_RUNTIME_METHODS=[\"ccall\",\"cwrap\",\"UTF8ToString\"]"
        "SHELL:-sFILESYSTEM=0"
        "SHELL:-sASSERTIONS=0"
        "SHELL:-sSTACK_SIZE=131072"
        "SHELL:-sASYNCIFY=1"
        "SHELL:-sASYNCIFY_STACK_SIZE=131072"
    )
else()
    add_library(dtml ${SOURCES})
    target_sources(dtml PRIVATE src/fetch-native.cc)
    target_compile_definitions(dtml PRIVATE TARGET_NATIVE)
endif()

set_target_properties(dtml PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME dtml
)

target_include_directories(dtml PUBLIC src)

if((dtml_BUILD_TESTS OR dtml_BUILD_BENCHMARKS) AND NOT EMSCRIPTEN)
    include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)
endif()

if(dtml_BUILD_TESTS AND NOT EMSCRIPTEN)
    enable_testing()

    add_executable(tel-json-test tests/tel-json-test.cc)
    target_link_libraries(tel-json-test PRIVATE dtml Catch2::Catch2WithMain)

    set_target_properties(tel-json-test PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)
    catch_discover_tests(tel-json-test)
endif()

if(dtml_BUILD_BENCHMARKS AND NOT EMSCRIPTEN)
    add_executable(tel-json-benchmark bench/tel-json-benchmark.cc)
    target_link_libraries(tel-json-benchmark PRIVATE dtml Catch2::Catch2WithMain)
    target_compile_definitions(tel-json-benchmark PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)

    set_target_properties(tel-json-benchmark PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endif()
