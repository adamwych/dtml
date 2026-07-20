cmake_minimum_required(VERSION 4.0)

project(dhtml LANGUAGES CXX)

option(DHTML_BUILD_TESTS "Build dhtml tests" OFF)
option(DHTML_BUILD_BENCHMARKS "Build dhtml benchmarks" OFF)

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

    add_executable(dhtml ${SOURCES})

    set_target_properties(dhtml PROPERTIES SUFFIX ".js")

    target_sources(dhtml PRIVATE src/fetch-emscripten.cc)
    target_compile_definitions(dhtml PRIVATE TARGET_EMSCRIPTEN)

    string(CONCAT DHTML_EXPORTED_FUNCTIONS
        "["
        "\"_DHTML_Template_New\","
        "\"_DHTML_Template_Free\","
        "\"_DHTML_TemplateCache_New\","
        "\"_DHTML_TemplateCache_Free\","
        "\"_DHTML_TemplateCache_Clear\","
        "\"_DHTML_TemplateEvaluationContext_New\","
        "\"_DHTML_TemplateEvaluationContext_Free\","
        "\"_DHTML_TemplateEvaluator_New\","
        "\"_DHTML_TemplateEvaluator_Free\","
        "\"_DHTML_TemplateEvaluator_Evaluate\","
        "\"_DHTML_TemplateEvaluationResult_Free\","
        "\"_DHTML_TemplateEvaluationResult_GetText\","
        "\"_DHTML_TemplateEvaluationResult_IsError\","
        "\"_DHTML_TemplateEvaluationResult_GetErrorLocationLine\","
        "\"_DHTML_TemplateEvaluationResult_GetErrorLocationColumn\""
        "]"
    )

    target_link_options(dhtml PRIVATE
        "SHELL:-sMODULARIZE=1"
        "SHELL:-sEXPORT_NAME=createDHTMLModule"
        "SHELL:-sENVIRONMENT=web,worker"
        "SHELL:-sEXPORTED_FUNCTIONS=${DHTML_EXPORTED_FUNCTIONS}"
        "SHELL:-sEXPORTED_RUNTIME_METHODS=[\"ccall\",\"cwrap\",\"UTF8ToString\"]"
        "SHELL:-sFILESYSTEM=0"
        "SHELL:-sASSERTIONS=0"
        "SHELL:-sSTACK_SIZE=131072"
        "SHELL:-sASYNCIFY=1"
        "SHELL:-sASYNCIFY_STACK_SIZE=131072"
    )
else()
    add_library(dhtml ${SOURCES})
    target_sources(dhtml PRIVATE src/fetch-native.cc)
    target_compile_definitions(dhtml PRIVATE TARGET_NATIVE)
endif()

set_target_properties(dhtml PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    OUTPUT_NAME dhtml
)

target_include_directories(dhtml PUBLIC src)

if((DHTML_BUILD_TESTS OR DHTML_BUILD_BENCHMARKS) AND NOT EMSCRIPTEN)
    include(FetchContent)

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)
endif()

if(DHTML_BUILD_TESTS AND NOT EMSCRIPTEN)
    enable_testing()

    add_executable(tel-json-test tests/tel-json-test.cc)
    target_link_libraries(tel-json-test PRIVATE dhtml Catch2::Catch2WithMain)

    set_target_properties(tel-json-test PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(Catch)
    catch_discover_tests(tel-json-test)
endif()

if(DHTML_BUILD_BENCHMARKS AND NOT EMSCRIPTEN)
    add_executable(tel-json-benchmark bench/tel-json-benchmark.cc)
    target_link_libraries(tel-json-benchmark PRIVATE dhtml Catch2::Catch2WithMain)
    target_compile_definitions(tel-json-benchmark PRIVATE CATCH_CONFIG_ENABLE_BENCHMARKING)

    set_target_properties(tel-json-benchmark PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )
endif()
