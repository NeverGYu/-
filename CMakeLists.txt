cmake_minimum_required(VERSION 3.20)
project(sylar)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 启用位置无关代码（解决 relocation 警告）
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 配置最终的可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 查找 yaml-cpp 库（必须）
find_package(yaml-cpp REQUIRED)

# 加载子目录
add_subdirectory(src)

# 获得测试目录文件
file(GLOB test ${CMAKE_SOURCE_DIR}/test/*.cpp)
# 遍历所有测试文件，为每个生成一个可执行文件
foreach(test_src ${test})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE lsylar) # 自动继承依赖
endforeach()


