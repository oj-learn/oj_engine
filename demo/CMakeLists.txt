# CMake 最低的版本要求
cmake_minimum_required(VERSION 2.8)

#---------------------------------------------------------------------------------------
# clang
# 设置编译选项的几种方法
# CMAKE_CXX_COMPILER：指定C++编译器
# CMAKE_CXX_FLAGS：指定编译C++文件时编译选项，也可以通过add_definitions命令添加编译选项

# 设置编译选项（配置编译器）有如下三种方法：
# add_compile_options命令: add_compile_options(-std=c++2a -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes)
# set命令修改CMAKE_CXX_FLAGS：set(CMAKE_CXX_FLAGS "-std=c++2a -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes)
# ADD_DEFINITIONS("-std=c++2a -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes)
#---------------------------------------------------------------------------------------
#set(CMAKE_SYSTEM_NAME Linux )

#SET (CMAKE_C_COMPILER             "/usr/local/bin/clang")
#SET (CMAKE_C_FLAGS                "-Wall -std=c99")
#SET (CMAKE_C_FLAGS_DEBUG          "-g")
#SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
#SET (CMAKE_C_FLAGS_RELEASE        "-O4 -DNDEBUG")
#SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
 
#SET (CMAKE_CXX_COMPILER             "/usr/local/bin/clang++")
#SET (CMAKE_CXX_FLAGS                "-Wall")
#SET (CMAKE_CXX_FLAGS_DEBUG          "-g")
#SET (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
#SET (CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
#SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
 
#SET (CMAKE_AR      "/usr/local/bin/llvm-ar")
#SET (CMAKE_LINKER  "/usr/local/bin/llvm-ld")
#SET (CMAKE_NM      "/usr/local/bin/llvm-nm")
#SET (CMAKE_OBJDUMP "/usr/local/bin/llvm-objdump")
#SET (CMAKE_RANLIB  "/usr/local/bin/llvm-ranlib")

#---------------------------------------------------------------------------------------
# build tips
#---------------------------------------------------------------------------------------
# mkdir build
# cd build
# cmake ..
# make debug
# sudo make install

#---------------------------------------------------------------------------------------
# project
#---------------------------------------------------------------------------------------
# 定义项目名称变量PROJECT_NAME, 默认值为demo
set(PROJECT_NAME demo)

# 项目名称以及版本
# 项目最终编译生成的可执行文件并不一定是这个项目名称，而是由另一条命令确定的
# 这个项目名称还是必要的，在cmake中有两个预定义变量：< projectname >_BINARY_DIR以及< projectname >_SOURCE_DIR
# cmake还预定义了PROJECT_BINARY_DIR和PROJECT_SOURCE_DIR 等同于上面的变量
# 在实际的应用，我强烈推荐使用后面两个变量,这样即使项目名称发生变化也不会影响CMakeLists.txt文件
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

# 设置编译选项
# 当CMAKE_BUILD_TYPE为Debug的时候，Cmake会使用变量CMAKE_FLAGS_DEBUG， CMAKE_CXX_FLAGS_DEBUG
# -g  -ggdb，添加调试信息的参数是 ，如果不添加这个参数，调试信息就不会被包含在生成的二进制中
# -Ox，优化程度最低的是 -O0 即不做优化，
# -fstrict-aliasing”表示启用严格别名规则，“-fno-strict-aliasing”表示禁用严格别名规则，当gcc的编译优化参数为“-O2”、“-O3”和“-Os”时，默认会打开“-fstrict-aliasing

# 设置编译选项, 也可以用set CMAKE_CXX_FLAGS 设置，这个是设置全局编译选项
#add_compile_options(-std=c++2a -fPIC -fno-strict-aliasing -Wall -pthread)
# -pthread 好像只能通过这样设置才会生效
set(CMAKE_CXX_FLAGS "-std=c++2a -fPIC -fno-strict-aliasing -pthread")
# 有些选项只能这样设置
# 选项-pg能产生供gprof剖析用的可执行文件。gprof是Linux下对C++程序进行性能分析的工具。
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
# -Wall 开启警告，-Wno 去掉一些警告
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-unused-function -Wno-unused-local-typedefs -Wno-comment")

# debug的编译选项
#set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g -ggdb -O0")
set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -O0")
# release的编译选项，不需要调试信息
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

if (WIN32)
    # windows platform
    #add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /EHsc")
    #set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /EHsc")
endif()

message("------------ Compiler config  -------------")
message("  CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message("  CMAKE_COMMAND: ${CMAKE_COMMAND}")
message("  enable_debugger: ${ENABLE_DEBUGGER}")
message("  build dynamic lib: ${BUILD_DYNAMIC}")
message("  CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message("  CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

#---------------------------------------------------------------------------------------
# 通过cmake_config.h.in 里面的定义
#pragma once
#define ENABLE_DEBUGGER ${ENABLE_DEBUGGER}
#define ENABLE_HOOK ${ENABLE_HOOK}
# 生成cmake_config.h
#pragma once
#define ENABLE_DEBUGGER 0
#define ENABLE_HOOK 1
#configure_file(${PROJECT_SOURCE_DIR}/cmake_config.h.in ${PROJECT_SOURCE_DIR}/cmake_config.h)
#---------------------------------------------------------------------------------------

#---------------------------------------------------------------------------------------
# include
# include_directories： include头文件时搜索的所有目录
# link_directories: 库文件存放的目录，在程序连接库文件的时候要再这些目录下寻找对应的库文件
# aux_source_directory: 查找当前目录下的所有源文件,并将名称保存到 SRC_LIST 变量
# PROJECT_SOURCE_DIR: 表示工程所在的路径，系统默认的变量
# find_package: 如果找到则会产生一系列对应的变量，比如boost会产生(Boost_INCLUDE_DIRS, Boost_LIBRARIES.....)
#---------------------------------------------------------------------------------------
message("------------ Boost -------------")
# 设定 find_package　查找.cmake的路径
#set(Boost_DIR "home/itzj00092/github.com/lib/boost/cmake")
#find_package(Boost REQUIRED COMPONENTS regex)
message("Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message("Boost_LIBRARIES: ${Boost_LIBRARIES}")
message("Boost_LIB_VERSION: ${Boost_LIB_VERSION}.")


message("------------ include -------------")
# include头文件时搜索的所有目录
include_directories (
    ${PROJECT_SOURCE_DIR}
    ${oj_dir}/include
    ${oj_dir}/include/engine
)
# 库文件存放的目录，在程序连接库文件的时候要再这些目录下寻找对应的库文件
link_directories (
    ${oj_dir}/lib
    ${oj_dir}/lib/boost
)
message("------------ -- -------------")


#---------------------------------------------------------------------------------------
# src
#---------------------------------------------------------------------------------------
message("------------ Src -------------")
#set(SRC_LIST ${PROJECT_SOURCE_DIR}/main.cpp)
aux_source_directory(${PROJECT_SOURCE_DIR}/ SRC_LIST)
#list(APPEND SRC_LIST ${PROJECT_SOURCE_DIR}/base/time.cpp)


#---------------------------------------------------------------------------------------
# target
# add_executable: 生成可执行文件
# add_library: 生成库文件,SHARED 表示生成动态库， STATIC表示生成静态库。
#---------------------------------------------------------------------------------------
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

# 重新定义 EXECUTABLE_OUTPUT_PATH 和 LIBRARY_OUTPUT_PATH 变量
# 来指定最终的目标二进制的位置(指最终生成的 hello 或者最终的共享库,不包含编译生成的中间文件)
# ${PROJECT_BINARY_DIR} 指的是外部编译所在目录,也就是本例中的 build目录。
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

#---------------------------------------------------------------------------------------
# link
# link_libraries和target_link_libraries是很重要的两个链接库的方式，虽然写法上很相似，但是功能上有很大区别：
# link_libraries用在add_executable之前，target_link_libraries用在add_executable之后
# link_libraries用来链接静态库，target_link_libraries用来链接导入库，即按照header file + .lib + .dll方式隐式调用动态库的.lib库
#---------------------------------------------------------------------------------------

# 链接Boost库文件
if(Boost_FOUND)
    target_link_libraries(${TARGET_T} ${Boost_LIBRARIES})
endif()

# 注意：顺序很重要　libnet.a　依赖　libbase.a 依赖 uuid
#target_link_libraries(${TARGET_T} libnet.a libbase.a uuid)
# 不用关心顺序 的写法
target_link_libraries(
    ${TARGET_T}
    -Wl,--start-group
    libengine.a
    liblibgo.a
    uuid
    dl
    mysqlclient 
    -Wl,--end-group
)

#---------------------------------------------------------------------------------------
# install
# TARGETS后面跟的就是我们通过ADD_EXECUTABLE或者ADD_LIBRARY定义的目标文件，可能是可执行二进制、动态库、静态库。
# 举个简单的例子：INSTALL(TARGETS myrun mylib mystaticlib
# RUNTIME DESTINATION bin
# LIBRARY DESTINATION lib
# ARCHIVE DESTINATION libstatic)
# 上面的例子会将：
# 可执行二进制myrun安装到${CMAKE_INSTALL_PREFIX}/bin目录
# 动态库libmylib安装到${CMAKE_INSTALL_PREFIX}/lib目录
# 静态库libmystaticlib安装到${CMAKE_INSTALL_PREFIX}/libstatic目录
#---------------------------------------------------------------------------------------

# 设置安装路径
set(CMAKE_INSTALL_PREFIX "${oj_dir}/")
message("  CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")

# 安装目标文件, 在 ${CMAKE_INSTALL_PREFIX}/　下
#install(TARGETS ${TARGET_T} RUNTIME DESTINATION "bin/${TARGET}")
#install(TARGETS ${TARGET_T} RUNTIME DESTINATION "bin/${TARGET}-client")
#install(TARGETS ${TARGET_T} RUNTIME DESTINATION "bin/${TARGET}-server")
#install(TARGETS ${TARGET_T} RUNTIME DESTINATION "bin/${TARGET}-main")
install(TARGETS ${TARGET_T} RUNTIME DESTINATION "bin/${TARGET}-daemon")

# 安装目录 将　${PROJECT_SOURCE_DIR}/目录下的 "*.h" 
#install(DIRECTORY ${PROJECT_SOURCE_DIR}/ 
# 拷到　${CMAKE_INSTALL_PREFIX}/include/${TARGET} 
#DESTINATION "include/${TARGET}" 
#FILES_MATCHING PATTERN "*.h"
# 排除 build 目录
#PATTERN "build" EXCLUDE
#)

######################################  ######################################
#add_custom_target(debug
#    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=DEBUG ${CMAKE_SOURCE_DIR}
#    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#    COMMENT "Switch CMAKE_BUILD_TYPE to Debug"
#)

#add_custom_target(release
#    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=RELEASE ${CMAKE_SOURCE_DIR}
#    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#    COMMENT "Switch CMAKE_BUILD_TYPE to Release"
#)

#set(PROFILE_FLAGS "-pg ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")

# message("PROFILE_FLAGS: ${PROFILE_FLAGS}")
#add_custom_target(profile
#    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=PROFILE -DCMAKE_CXX_FLAGS_PROFILE=\\'${PROFILE_FLAGS}\\' ${CMAKE_SOURCE_DIR}
#    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
#    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#    COMMENT "Switch CMAKE_BUILD_TYPE to PROFILE"
#)

#add_custom_target(uninstall
#    COMMAND rm -rf ${CMAKE_INSTALL_PREFIX}/include/${TARGET} 
#    COMMAND rm -f ${CMAKE_INSTALL_PREFIX}/lib/lib${TARGET}.a ${CMAKE_INSTALL_PREFIX}/lib/lib${TARGET}.so
#)


###################################################################################
