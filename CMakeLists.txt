cmake_minimum_required(VERSION 2.8)
project(cppa C CXX)

# extract version from VERSION file
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION LIMIT_COUNT 1)
string(REPLACE "." " " version_numbers ${VERSION})
separate_arguments(version_numbers)
list(GET version_numbers 0 ACTORSHELL_VERSION_MAJOR)
list(GET version_numbers 1 ACTORSHELL_VERSION_MINOR)
list(GET version_numbers 2 ACTORSHELL_VERSION_PATCH)

# prohibit in-source builds
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please use "
                        "./configure to choose a build directory and "
                        "initialize the build configuration.")
endif ()

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# set binary output path
if ("${EXECUTABLE_OUTPUT_PATH}" STREQUAL "")
  set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
endif ()

# set library output path
if ("${LIBRARY_OUTPUT_PATH}" STREQUAL "")
  set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
endif ()

# set library output path to binary output path for Xcode projects
if ("${CMAKE_GENERATOR}" STREQUAL "Xcode")
  set(LIBRARY_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}")
endif ()

# check for g++ >= 4.7 or clang++ > = 3.2
try_run(ProgramResult
        CompilationSucceeded
  ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src/get_compiler_version.cpp
        RUN_OUTPUT_VARIABLE CompilerVersion)
if (NOT CompilationSucceeded OR NOT ProgramResult EQUAL 0)
  message(FATAL_ERROR "Cannot determine compiler version")
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if (CompilerVersion VERSION_GREATER 4.6)
    message(STATUS "Found g++ version ${CompilerVersion}")
  else ()
    message(FATAL_ERROR "g++ >= 4.7 required (found: ${CompilerVersion}.")
  endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  if (CompilerVersion VERSION_GREATER 3.1)
    message(STATUS "Found clang++ version ${CompilerVersion}")
  else ()
    message(FATAL_ERROR "clang++ >= 3.2 required (found: ${CompilerVersion}.")
  endif ()
  if (NOT CXXFLAGS_PROVIDED)
    message(STATUS "NOTE: Automatically added -stdlib=libc++ flag, "
                   "you can override this by defining CMAKE_CXX_FLAGS "
                   "(see 'configure --help')")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif ()
else ()
  message(FATAL_ERROR "Your C++ compiler does not support C++11 "
                      "or is not supported")
endif ()

# check if the user provided CXXFLAGS on the command line
if (CMAKE_CXX_FLAGS)
  set(CXXFLAGS_PROVIDED true)
  set(CMAKE_CXX_FLAGS_DEBUG          "")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "")
  set(CMAKE_CXX_FLAGS_RELEASE        "")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "")
else (CMAKE_CXX_FLAGS)
  set(CXXFLAGS_PROVIDED false)
  if (MORE_CLANG_WARNINGS AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -Weverything -Wno-c++98-compat -Wno-padded -Wno-documentation-unknown-command -Wno-exit-time-destructors -Wno-global-constructors -Wno-missing-prototypes -Wno-c++98-compat-pedantic -Wno-unused-member-function -Wno-unused-const-variable")
  else ()
    set(CMAKE_CXX_FLAGS "-std=c++11 -Wextra -Wall -pedantic")
  endif ()
  if (MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN32")
    include (GenerateExportHeader)
  endif(MINGW)
  set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
  set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os")
  set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
  set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
endif (CMAKE_CXX_FLAGS)

if (CPPA_LOG_LEVEL)
  add_definitions(-DCPPA_LOG_LEVEL=${CPPA_LOG_LEVEL})
endif(CPPA_LOG_LEVEL)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# set build default build type if not set
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif ("${CMAKE_BUILD_TYPE}" STREQUAL "")

# get header files; only needed by CMake generators,
# e.g., for creating proper Xcode projects
file(GLOB ACTORSHELL_HDRS "actorshell/*.hpp")

# list cpp files including platform-dependent files
set(ACTORSHELL_SRC    
    src/main.cpp)

find_package(LibCPPA REQUIRED)
find_package(Editline REQUIRED)

include_directories(. .. ${CPPA_INCLUDE} ${EDITLINE_INCLUDE_DIR} sash)

set(INCLUDE_DIRS .)
set(LD_DIRS)
set(LD_FLAGS ${CMAKE_LD_LIBS})

add_executable(actorshell ${ACTORSHELL_SRC})
target_link_libraries(actorshell ${CMAKE_DL_LIBS} ${CPPA_LIBRARY} ${LD_FLAGS} ${EDITLINE_LIBRARIES})

if (APPLE)
  # -pthread is ignored on MacOSX
elseif (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
elseif (MINGW)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
  set(LD_FLAGS "ws2_32 -liphlpapi")
endif ()

link_directories(${LD_DIRS})
include_directories(${INCLUDE_DIRS})

# install includes
install(DIRECTORY cppa/ DESTINATION include/cppa FILES_MATCHING PATTERN "*.hpp")

# process cmake_uninstall.cmake.in
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)

# add uninstall target
add_custom_target(uninstall
                  COMMAND ${CMAKE_COMMAND} -P
                  ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# set variables for summary
set(LOG_LEVEL_STR "none")
if (CPPA_LOG_LEVEL)
  if (${CPPA_LOG_LEVEL} EQUAL 0)
    set(LOG_LEVEL_STR "ERROR")
  elseif (${CPPA_LOG_LEVEL} EQUAL 1)
    set(LOG_LEVEL_STR "WARNING")
  elseif (${CPPA_LOG_LEVEL} EQUAL 2)
    set(LOG_LEVEL_STR "INFO")
  elseif (${CPPA_LOG_LEVEL} EQUAL 3)
    set(LOG_LEVEL_STR "DEBUG")
  elseif (${CPPA_LOG_LEVEL} EQUAL 4)
    set(LOG_LEVEL_STR "TRACE")
  else ()
    set(LOG_LEVEL_STR "invalid")
  endif ()
endif (CPPA_LOG_LEVEL)

macro (toYesNo in out)
  if (${in})
    set(${out} "yes")
  else ()
    set(${out} "no")
  endif ()
endmacro ()

toYesNo(ENABLE_DEBUG DEBUG_MODE_STR)

# done (print summary)
message("\n====================|  Build Summary  |===================="
        "\n"
        "\nLibcppa version:   ${LIBRARY_VERSION}"
        "\n"
        "\nBuild type:        ${CMAKE_BUILD_TYPE}"
        "\nDebug mode:        ${DEBUG_MODE_STR}"
        "\nLog level:         ${LOG_LEVEL_STR}"
        "\n"
        "\nCXX:               ${CMAKE_CXX_COMPILER}"
        "\nCXXFLAGS:          ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${build_type}}"
        "\nLD_DIRS:           ${LD_DIRS}"
        "\nLIBRARIES:         ${LD_FLAGS}"
        "\n"
        "\nSource directory:  ${CMAKE_SOURCE_DIR}"
        "\nBuild directory:   ${CMAKE_BINARY_DIR}"
        "\nExecutable path:   ${EXECUTABLE_OUTPUT_PATH}"
        "\nLibrary path:      ${LIBRARY_OUTPUT_PATH}"
        "\nInstall prefix:    ${CMAKE_INSTALL_PREFIX}"
        "\n"
        "\nEditline:          ${EDITLINE_LIBRARIES}"
        "\n"
        "\n===========================================================\n")
