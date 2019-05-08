cmake_minimum_required(VERSION 3.2)

option(
    HUNTER_STATUS_DEBUG OFF
    )

include("${CMAKE_CURRENT_SOURCE_DIR}/HunterGate.cmake")

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.83.tar.gz"
    SHA1 "12dec078717539eb7b03e6d2a17797cba9be9ba9"
)

include(ExternalProject)

project(CMakeLIEF)

# LIEF as an External Project
# ===========================
set(LIEF_PREFIX       "${CMAKE_CURRENT_BINARY_DIR}/LIEF")
set(LIEF_INSTALL_DIR  "${LIEF_PREFIX}")
set(LIEF_INCLUDE_DIRS "${LIEF_PREFIX}/include")

# LIEF static library
set(LIB_LIEF_STATIC
  "${LIEF_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}LIEF${CMAKE_STATIC_LIBRARY_SUFFIX}")

# URL of the LIEF repo (Can be your fork)
set(LIEF_GIT_URL "https://github.com/lief-project/LIEF.git")

# LIEF's version to be used (can be 'master')
set(LIEF_VERSION 0.9.0)

# LIEF compilation config
set(LIEF_CMAKE_ARGS
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DLIEF_DOC=off
  -DLIEF_PYTHON_API=off
  -DLIEF_EXAMPLES=off
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
)

ExternalProject_Add(LIEF
  PREFIX           "${PACKER_LIEF_PREFIX}"
  GIT_REPOSITORY   ${LIEF_GIT_URL}
  GIT_TAG          ${LIEF_VERSION}
  INSTALL_DIR      ${LIEF_INSTALL_DIR}
  CMAKE_ARGS       ${LIEF_CMAKE_ARGS}
  BUILD_BYPRODUCTS ${LIEF_LIBRARIES}
  UPDATE_COMMAND   ""
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)
set( CMAKE_EXPORT_COMPILE_COMMANDS ON ) # For YCM

# Add our executable
# ==================
set(BINNAME "main.elf")
set(MYSOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RegisterManager.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Packer.cpp"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/Loader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/Utils.cpp"
    )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

link_libraries(stdc++fs)
add_executable("${BINNAME}" ${MYSOURCES})

if (MSVC)
  # Used for the 'and', 'or' ... keywords - See: http://www.cplusplus.com/reference/ciso646/
  target_compile_options("${BINNAME}" PUBLIC /FIiso646.h)
	set_property(TARGET "${BINNAME}" PROPERTY LINK_FLAGS /NODEFAULTLIB:MSVCRT)
endif()

# Setup the LIEF include directory
target_include_directories("${BINNAME}"
  PUBLIC
  ${LIEF_INCLUDE_DIRS}
)

# Enable C++17
set_property(TARGET "${BINNAME}" PROPERTY CXX_STANDARD           17)
set_property(TARGET "${BINNAME}" PROPERTY CXX_STANDARD_REQUIRED  ON)

# Link the executable with LIEF
target_link_libraries("${BINNAME}" PUBLIC ${LIB_LIEF_STATIC})
message(STATUS "Using Lief Version: ${LIEF_VERSION}")
add_dependencies("${BINNAME}" LIEF)

# Dependencies
# ============
hunter_add_package(spdlog)
hunter_add_package(fmt)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries("${BINNAME}" PUBLIC spdlog::spdlog)

