cmake_minimum_required(VERSION 3.2)
project(InterOp)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)
enable_testing()
include(${PROJECT_SOURCE_DIR}/cmake/Modules/UseGitVersion.cmake)
set(ARCHIVE_VERSION "v1.0.16-src")

find_package(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()


if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

# TODO: https://help.github.com/articles/creating-project-pages-manually/

# TODO: Handle Windows Shared libs: https://cmake.org/Wiki/BuildingWinDLL

set(INTEROP_TESTS interop_tests)
set(INTEROP_LIB interop_lib)
set(INTEROP_DL_LIB interop_fpic_lib)

option(ENABLE_BACKWARDS_COMPATIBILITY "Compile code for c++98" ON)
option(ENABLE_DOCS "Build documentation with Doxygen" ON)
option(ENABLE_SWIG "Build third-party language bindings, e.g. C#" ON)
option(ENABLE_TEST "Build unit tests (depends on Boost)" ON)
option(ENABLE_APPS "Build command line programs" ON)
option(ENABLE_EXAMPLES "Build example programs" ON)
option(ENABLE_STATIC "Build static libraries instead of dynamic" ON)
option(FORCE_X86 "Force 32-bit libraries instead of platform default (Does nothing for Visual Studio)" OFF)
option(FORCE_SHARED_CRT "Used the shared (DLL) run time lib (must be ON if compiling for C#)" ON)
option(ENABLE_CSHARP "Build C# language bindings" ON)
option(ENABLE_PYTHON "Build Python language bindings" ON)

include(${PROJECT_SOURCE_DIR}/cmake/InternalUtils.cmake)
interop_config_compiler_and_linker()

# Options to control integration builds
set(PACKAGE_SUFFIX "" CACHE STRING "Suffix added to the output packages")
set(BUILD_NUMBER "" CACHE STRING "Build number used for select packing scripts")
mark_as_advanced(PACKAGE_SUFFIX BUILD_NUMBER)

include_directories(.)
add_version_target(version ${CMAKE_SOURCE_DIR}/interop/version.h INTEROP_VERSION ${ARCHIVE_VERSION})

if(INTEROP_VERSION)
    string(REGEX REPLACE "[^v]*(v.*)" "\\1" VERSION ${INTEROP_VERSION})
    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${VERSION}")
    if(VERSION_SHA1)
        string(REGEX REPLACE "[^0-9]*([0-9]*)-.*" "\\1" VERSION_DEV "${VERSION_SHA1}")
        if(VERSION_DEV)
            set(VERSION_DEV ".dev${VERSION_DEV}")
        endif()
    endif()
    set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
    string(REPLACE "." "," VERSION_LIST "${VERSION_SHORT}")  # Used to configure the version info file
    message(STATUS "InterOp v${VERSION_SHORT}")
    if(WIN32)
        set(CPACK_GENERATOR "ZIP")
    else()
        set(CPACK_GENERATOR "TGZ")
    endif()
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Illumina InterOp - ${VERSION_SHORT}")
    set(CPACK_PACKAGE_VENDOR "Illumina, Inc.")
    set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
    set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
    set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
    set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}${PACKAGE_SUFFIX}")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "interop_${VERSION_SHORT}")
    if(NOT PACKAGE_OUTPUT_FILE_PREFIX)
        set(PACKAGE_OUTPUT_FILE_PREFIX ".")
    else()
        get_filename_component(PACKAGE_OUTPUT_FILE_PREFIX ${PACKAGE_OUTPUT_FILE_PREFIX} ABSOLUTE)
    endif()
    set(CPACK_OUTPUT_FILE_PREFIX "${PACKAGE_OUTPUT_FILE_PREFIX}/interop/${VERSION}")
endif()

#Adds the target "package"
include(CPack)

if(ENABLE_STATIC)
    set(LIBRARY_TYPE STATIC)
    set(BUILD_SHARED_LIBS OFF)
else()
    set(LIBRARY_TYPE SHARED)
    set(BUILD_SHARED_LIBS ON)
endif()

if(WIN32 AND ${LIBRARY_TYPE} STREQUAL "SHARED")
    message(FATAL_ERROR "Shared libraries on Windows are not supported")
endif()

add_subdirectory("src")
if(ENABLE_DOCS)
    add_subdirectory("docs")
endif()
install(DIRECTORY interop DESTINATION include)
install(FILES README.md docs/src/changes.md DESTINATION .)

find_package(Git)
if(GIT_FOUND)
    string(REGEX REPLACE "[^v]*(v[0-9]+.[0-9]+.[0-9]+)-.*" "\\1" TAG ${INTEROP_VERSION})
    add_custom_target(history
        COMMAND ${GIT_EXECUTABLE};log;${TAG}..HEAD;-m;--first-parent;--date=short;--format='%ad | %b'
        COMMENT "List all commits from last tag - ${TAG}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()
