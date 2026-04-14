cmake_minimum_required (VERSION 3.24)

project(tyche
  VERSION 0.0.1
  DESCRIPTION "A Lua/Go-like programing language"
  LANGUAGES C CXX ASM)

#
# project options / configuration
#

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23 CACHE STRING "C++ Standard")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
set_property(GLOBAL PROPERTY LINK_WHAT_YOU_USE TRUE)

# warnings / flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(warnings -Wall -Wextra -Wformat-nonliteral -Wundef -Wshadow -Wwrite-strings -Wfloat-equal -Wswitch-default -Wmissing-format-attribute -Wswitch-enum -Wmissing-noreturn -Wno-unused-parameter -Wno-unused)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(warnings ${warnings} -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=malloc -Wsuggest-attribute=format -Wsuggest-attribute=cold)
    endif()
endif()

# try to use ccache, if available
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# ignore warnings in imported files
set_source_files_properties(${IMGUI_SRC} PROPERTIES COMPILE_FLAGS "-w")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-ggdb -O0)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(DEF B_PRODUCTION_MODE=ON)
    add_compile_options(-Ofast -flto)
endif()

#
# libraries
#

include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
FetchContent_MakeAvailable(googletest)

#
# library
#

add_library(lib${PROJECT_NAME} STATIC
	lib/vm.hh
	lib/vm.cc)

target_compile_options(lib${PROJECT_NAME} PRIVATE ${warnings})

#
# tests
#

add_executable(${PROJECT_NAME}-test tests.cc)
target_link_libraries(${PROJECT_NAME}-test lib${PROJECT_NAME} gtest_main)
add_test(NAME tyche_test COMMAND ${PROJECT_NAME}-test)

#
# check for leaks
#

add_custom_target(leaks)
add_custom_command(TARGET leaks
    POST_BUILD
    COMMENT "Check for leaks using valgrind."
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS ${PROJECT_NAME}
    COMMAND valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=${CMAKE_SOURCE_DIR}/valgrind.supp ./${PROJECT_NAME}
)

#
# installation
#

install(TARGETS lib${CMAKE_PROJECT_NAME} RUNTIME DESTINATION lib)
