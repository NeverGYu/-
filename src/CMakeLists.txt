# 定义一个SRC_LSIT 变量，包含了当前目录下的所有源文件
aux_source_directory(./base BASE_SRC)
aux_source_directory(./http HTTP_SRC)
aux_source_directory(./stream STREAM_SRC)
list(REMOVE_ITEM SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp") 

# 生成静态库
add_library(lsylar ${BASE_SRC} ${HTTP_SRC} ${STREAM_SRC})
target_include_directories(lsylar PUBLIC 
            ${CMAKE_SOURCE_DIR}/include/base
            ${CMAKE_SOURCE_DIR}/include/http/http-parser
            ${CMAKE_SOURCE_DIR}/include/http/base
            ${CMAKE_SOURCE_DIR}/include/stream)

# 声明依赖（PUBLIC 传递到上层, PRIVATE 只能本层使用）
target_link_libraries(lsylar PUBLIC
    dl
    yaml-cpp::yaml-cpp 
    pthread)     

# 生成动态库
#add_library(sylar SHARED ${BASE_SRC} ${HTTP_SRC} ${STREAM_SRC})
#target_include_directories(sylar PUBLIC 
#            ${CMAKE_SOURCE_DIR}/include/base
 #           ${CMAKE_SOURCE_DIR}/include/http/http-parser
  #          ${CMAKE_SOURCE_DIR}/include/http/base
   #         ${CMAKE_SOURCE_DIR}/include/stream)
            
