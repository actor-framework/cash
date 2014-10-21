cmake_minimum_required(VERSION 2.8)
project(caf_cash CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# check whether SASH submodule is available
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sash/sash/")
  message(STATUS "SASH submodule not loaded, disable CASH")
  set(DISABLE_CASH yes)
endif()

# check whether libedit is available
if(NOT DISABLE_CASH)
  find_package(Libedit)
  if(NOT LIBEDIT_FOUND)
    message(STATUS "Libedit not found, disable CASH")
    set(DISABLE_CASH yes)
  endif()
endif()

# add targets to CMake
if(NOT DISABLE_CASH)
  include_directories(. ${LIBCAF_INCLUDE_DIRS} sash/)
  file(GLOB CAF_CASH_HDRS "caf/cash/*.hpp" "sash/sash/*.hpp")
  set(CAF_CASH_SRCS
      src/main.cpp
      src/shell.cpp)
  add_executable(cash ${CAF_CASH_SRCS} ${CAF_CASH_HDRS})
  target_link_libraries(cash
                        ${LD_FLAGS}
                        ${LIBCAF_LIBRARIES}
                        ${PTHREAD_LIBRARIES}
                        ${LIBEDIT_LIBRARIES})
  install(PROGRAMS ${EXECUTABLE_OUTPUT_PATH}/cash DESTINATION bin)
endif()
