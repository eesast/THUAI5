# 所需cmake的最小版本
cmake_minimum_required(VERSION 3.10)

# 项目名称
project(THUAI5_CAPI VERSION 1.0)

# C++ 标准
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED True) 
set(CMAKE_CXX_FLAGS -pthread)
message(STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message(STATUS "PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}")

##################################################
# 生成C++通信组件
##################################################

# 生成静态库的源文件
add_library(communication STATIC
    Communication/src/Communication.cpp
)

# 需要包含的头文件
target_include_directories(communication
    PUBLIC
        ${PROJECT_SOURCE_DIR}/Communication/include
        ${PROJECT_SOURCE_DIR}/proto
        ${PROJECT_SOURCE_DIR}/HPSocket/linux
        ${PROJECT_SOURCE_DIR}/protobuf/src/google/protobuf
        ${PROJECT_SOURCE_DIR}/thuai/include
)

##################################################
# 生成C++可执行文件
##################################################
aux_source_directory(./API/src CPP_LIST)
aux_source_directory(./proto PROTO_CPP_LIST)
find_package(Protobuf REQUIRED)

if (Protobuf_FOUND)
    include_directories(${PROTOBUF_INCLUDE_DIR})
else (Protobuf_FOUND)
    message(FATAL_ERROR "Protobuf Not Found")
endif(Protobuf_FOUND)

add_executable(capi ${CPP_LIST} ${PROTO_CPP_LIST})

target_include_directories(capi
    PUBLIC
        ${PROJECT_SOURCE_DIR}/Communication/include
        ${PROJECT_SOURCE_DIR}/proto
        ${PROJECT_SOURCE_DIR}/HPSocket/linux
        # ${PROJECT_SOURCE_DIR}/protobuf/src
        # ${PROJECT_SOURCE_DIR}/protobuf/third_party/googletest/googletest/include
        # ${PROJECT_SOURCE_DIR}/protobuf/third_party/googletest/googletest
        # ${PROJECT_SOURCE_DIR}/protobuf/third_party/googletest/googlemock/include
        # ${PROJECT_SOURCE_DIR}/protobuf/third_party/googletest/googlemock
        ${PROJECT_SOURCE_DIR}/thuai/include
        ${PROJECT_SOURCE_DIR}/API/include
        ${PROJECT_SOURCE_DIR}/tclap/include
)

target_link_libraries(capi
    PUBLIC
        communication
        ${PROTOBUF_LIBRARY} 
        ${PROJECT_SOURCE_DIR}/lib/so/libhpsocket_d.so.5
        ${PROJECT_SOURCE_DIR}/lib/so/libhpsocket.so.5
        ${PROJECT_SOURCE_DIR}/lib/a/libhpsocket_d.a
        ${PROJECT_SOURCE_DIR}/lib/a/libhpsocket.a
)