# 定义一个SRC_LSIT 变量，包含了当前目录下的所有源文件
aux_source_directory(. SRC_LIST)
list(REMOVE_ITEM SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp") 

# 生成静态库
add_library(lsylar ${SRC_LIST})
target_include_directories(lsylar PUBLIC 
            ${CMAKE_SOURCE_DIR}/include/base
            $<TARGET_PROPERTY:yaml-cpp::yaml-cpp,INTERFACE_INCLUDE_DIRECTORIES>)

# 声明依赖（PUBLIC 传递到上层）
target_link_libraries(lsylar PUBLIC 
    dl
    yaml-cpp::yaml-cpp 
    pthread
)     

# 主程序（仅需链接静态库）
add_executable(sylar main.cpp) 
target_link_libraries(sylar PRIVATE lsylar)
