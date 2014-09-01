cmake_minimum_required(VERSION 2.8)
project(caf_shell CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sash/sash/")
  message(FATAL_ERROR "SASH submodule not loaded")
endif()

include_directories(. ${LIBCAF_INCLUDE_DIRS} sash/)

file(GLOB CAF_SHELL_HDRS "caf/shell/*.hpp" "sash/sash/*.hpp")

set(CAF_SHELL_SRCS
    src/view_actor.cpp
    src/main.cpp
    src/shell_actor.cpp)

find_package(Editline REQUIRED)

add_executable(shell ${CAF_SHELL_SRCS} ${CAF_SHELL_HDRS})

target_link_libraries(shell
                      ${LD_FLAGS}
                      ${LIBCAF_LIBRARIES}
                      ${PTHREAD_LIBRARIES}
                      ${EDITLINE_LIBRARIES})
