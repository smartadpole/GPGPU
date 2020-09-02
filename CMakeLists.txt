cmake_minimum_required(VERSION 3.2.0)

project(opengl_sdk)
find_package(glog REQUIRED)

set(ROOT_PATH ${CMAKE_CURRENT_LIST_DIR}/)
set(SRC_DIR
    ${ROOT_PATH}
    )

function (find_source_file SRC_FILES arg)
    set(file_list "")
    message(STATUS "[*] searching file: ${arg};${ARGN}")

    list(LENGTH ARGV  argv_len)
    set(i 1)
    while( i LESS ${argv_len})
         list(GET ARGV ${i} argv_value)
         foreach(file_path ${SRC_DIR})
             file(GLOB file ${file_path}/${argv_value})
             set(file_list ${file_list} ${file})
         endforeach()

         math(EXPR i "${i} + 1")
    endwhile()

    set(${SRC_FILES} ${file_list} PARENT_SCOPE)
endfunction ()

set(SRC_FILES "gpgpu.cpp")
find_source_file(SRC_FILES  "*.h" "*.cpp" "*.c" "*.cc" "*.hpp")
message(STATUS ${SRC_FILES})
add_executable(gpgpu ${SRC_FILES})

target_include_directories(gpgpu PRIVATE ${GLOG_INCLUDE_DIR})

target_link_libraries(gpgpu -lGLESv2 -lEGL -lGL -lglog -lpthread)
