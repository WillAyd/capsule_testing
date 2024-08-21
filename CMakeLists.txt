cmake_minimum_required(VERSION 3.20)
project(${SKBUILD_PROJECT_NAME} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)

# Detect the installed nanobind package and import it into CMake
execute_process(
  COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
  OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE NB_DIR)
list(APPEND CMAKE_PREFIX_PATH "${NB_DIR}")
find_package(nanobind CONFIG REQUIRED)

include(FetchContent)
FetchContent_Declare(nanoarrow-project
  GIT_REPOSITORY https://github.com/apache/arrow-nanoarrow.git
  GIT_TAG 67476e9993fc86748b0230b7b1322799fdf757bb 
)
FetchContent_MakeAvailable(nanoarrow-project)

nanobind_add_module(
  capsule_bug
  NOMINSIZE
  src/capsule_bug/capsule_bug.cc
)
target_link_libraries(capsule_bug
  PRIVATE nanoarrow
)
set_target_properties(nanoarrow
                      PROPERTIES POSITION_INDEPENDENT_CODE
                      ON)

install(TARGETS capsule_bug
  LIBRARY DESTINATION ${SKBUILD_PROJECT_NAME}/)
