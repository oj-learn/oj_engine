# CMake 最低的版本要求
cmake_minimum_required(VERSION 3.13)

#---------------------------------------------------------------------------------------
# project
#---------------------------------------------------------------------------------------
# 定义项目名称变量PROJECT_NAME, 默认值为demo
set(PROJECT_NAME demo)

project(${PROJECT_NAME})
set(oj_dir "${PROJECT_SOURCE_DIR}/..")

message("-------------- project ---------------")
message(STATUS "Build engine: ${engine_VERSION}")
message("  CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message("  CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message("  PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message("  PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
message("----------------------------------")

#---------------------------------------------------------------------------------------
# Compiler config

#---------------------------------------------------------------------------------------
enable_language(C ASM)

# CMAKE_CXX_COMPILER：指定C++编译器

# 调试开关
#set(CMAKE_BUILD_TYPE RELEASE)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "-std=c++2b -fmodules-ts -fPIC -fno-strict-aliasing -pthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function -Wno-unused-local-typedefs -Wno-comment")

# debug的编译选项
#set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g -ggdb -O0")
set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0")
# release的编译选项，不需要调试信息
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")


message("------------ Compiler config  -------------")
message("  CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("  CMAKE_COMMAND: ${CMAKE_COMMAND}")
message("  enable_debugger: ${ENABLE_DEBUGGER}")
message("  build dynamic lib: ${BUILD_DYNAMIC}")
message("  CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message("  CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

message("------------ include -------------")
# include头文件时搜索的所有目录
include_directories (
    ${PROJECT_SOURCE_DIR}
    ${oj_dir}/include
    ${oj_dir}/include/engine
    ${oj_dir}/include/engine/iguana/msgpack/include
)
# 库文件存放的目录，在程序连接库文件的时候要再这些目录下寻找对应的库文件
link_directories (
    ${oj_dir}/lib
)
message("------------ -- -------------")


#---------------------------------------------------------------------------------------
# src
#---------------------------------------------------------------------------------------
message("------------ Src -------------")
#
#set(SRC_LIST ${PROJECT_SOURCE_DIR}/main.cpp)
aux_source_directory(${PROJECT_SOURCE_DIR}/ SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/actorTest SRC_LIST)

message("------------ target -------------")

# target
# 静态库名为 lib${TARGET}.a； 动态库名为lib${TARGET}.so
set(TARGET ${PROJECT_NAME})

# 可执行文件生成
set(TARGET_T "${TARGET}_exe")
add_executable("${TARGET_T}" ${SRC_LIST})
# 属性设置
set_target_properties("${TARGET_T}" 
# 在输出时，不是"TARGET_T"，而是以"${TARGET}"的名字显示!
PROPERTIES OUTPUT_NAME "${TARGET}"
)

set_target_properties("${TARGET_T}" 
# 在构建一个新的target时，会尝试清理掉其他使用这个名字的库,阻止这种行为!
PROPERTIES CLEAN_DIRECT_OUTPUT 1
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

target_link_libraries(
    ${TARGET_T}
    -Wl,--start-group
    libengine.a
    uuid
    dl
    libbreakpad_client.a
    mysqlclient 
    -Wl,--end-group
)

# 设置安装路径
set(CMAKE_INSTALL_PREFIX "${oj_dir}/")
message("  CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

# 安装目标文件, 在 ${CMAKE_INSTALL_PREFIX}/　下
install(TARGETS ${TARGET_T} RUNTIME DESTINATION "bin")

###################################################################################
