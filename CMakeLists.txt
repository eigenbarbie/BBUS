cmake_minimum_required(VERSION 3.25)

project(
  belief_update
  VERSION 0.1.0
  DESCRIPTION "Deterministic Bayesian belief updates"
  LANGUAGES CXX
)

include(CTest)

option(BELIEF_UPDATE_BUILD_PYTHON "Build the Python extension" ON)
option(BELIEF_UPDATE_BUILD_DEMOS "Build demonstration programs" OFF)
option(BELIEF_UPDATE_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(BELIEF_UPDATE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ProjectOptions)

add_subdirectory(src)

if(BELIEF_UPDATE_BUILD_DEMOS)
  add_subdirectory(demo)
endif()

if(BELIEF_UPDATE_BUILD_PYTHON)
  find_package(Python 3.9 REQUIRED COMPONENTS Interpreter Development.Module)
  set(PYBIND11_FINDPYTHON ON)
  find_package(pybind11 3.1.0 EXACT CONFIG QUIET)

  if(NOT pybind11_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      pybind11
      URL https://github.com/pybind/pybind11/archive/refs/tags/v3.1.0.tar.gz
      URL_HASH SHA256=ef712655692a2e9bf7bb7874c022564a45f91d847ddee987e720cd9e28849665
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(pybind11)
  endif()

  add_subdirectory(bindings/python)
endif()

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
