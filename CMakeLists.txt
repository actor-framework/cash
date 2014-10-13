cmake_minimum_required(VERSION 2.8)
project(caf_cash CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sash/sash/")
  message(FATAL_ERROR "SASH submodule not loaded")
endif()

include_directories(. ${LIBCAF_INCLUDE_DIRS} sash/)

file(GLOB CAF_CASH_HDRS "caf/cash/*.hpp" "sash/sash/*.hpp")

set(CAF_CASH_SRCS
    src/main.cpp
    src/shell.cpp)

find_package(Edit REQUIRED)

add_executable(cash ${CAF_CASH_SRCS} ${CAF_CASH_HDRS})

target_link_libraries(cash
                      ${LD_FLAGS}
                      ${LIBCAF_LIBRARIES}
                      ${PTHREAD_LIBRARIES}
                      ${EDIT_LIBRARIES})

install(PROGRAMS ${EXECUTABLE_OUTPUT_PATH}/cash DESTINATION bin)
