# cmake_minimum_required(VERSION 2.5)
project(mymuduo)

# 作为库文件（不单独使用），故编译为so动态库, 放在根目录的lib文件夹下
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# 设置调试信息以及启动C++11语言标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")

# 定义所有参与编译的源代码文件（. 表示所有源文件）
aux_source_directory(. SRC_LIST) 
# 编译生成动态库muduo
add_library(mymuduo SHARED ${SRC_LIST})   