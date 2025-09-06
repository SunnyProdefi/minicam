cmake_minimum_required(VERSION 3.16)
project(minicam LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# warnings
if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 1) 节点源码（包含 MC_REGISTER_NODE 的 .cpp）
#    如果你的节点分散在不同目录，可以多写几个 GLOB，然后 list(APPEND)
file(GLOB NODE_SRCS
  src/nodes/*.cpp
)

# 2) 其它核心源码（不含节点）
file(GLOB_RECURSE MINICAM_CORE
  src/*.cpp
  src/*/*.cpp
)

# 从核心源码列表里剔除节点源（避免重复编译）
# 注意：GLOB 的返回是绝对路径，必要时可以 message(STATUS ...) 打印确认
foreach(node_src IN LISTS NODE_SRCS)
  list(REMOVE_ITEM MINICAM_CORE "${node_src}")
endforeach()

# 3) 节点编成 OBJECT 库（不会单独生成 .a/.lib，只有 .o/.obj）
add_library(nodes_obj OBJECT ${NODE_SRCS})

# 4) 其它源码仍然编成静态库
add_library(minicam STATIC ${MINICAM_CORE})

# 5) 可执行程序：链接核心库 + 直接并入节点对象文件
add_executable(demo app/main.cpp)
target_link_libraries(demo PRIVATE minicam)
target_sources(demo PRIVATE $<TARGET_OBJECTS:nodes_obj>)
