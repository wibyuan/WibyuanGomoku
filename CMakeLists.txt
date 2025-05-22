# Copyright (c) 2025 wibyuan
# Licensed under the MIT License (see LICENSE for details)
# CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和语言
project(WibyuanGomoku LANGUAGES CXX)

# 设置 C++ 标准 (例如 C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- SDL3 和 SDL3_ttf 开发库路径配置 ---
# 将这些路径设置为 CMake 缓存变量，方便您根据其本地环境进行配置。
# 您在首次配置时，可以通过 CMake GUI 或命令行 -D选项来修改这些路径。
# 例如: cmake .. -DSDL3_ARCH_DEVEL_PATH="您的/SDL3/路径"
set(SDL3_ARCH_DEVEL_PATH "../SDL3-devel-3.2.12-mingw/SDL3-3.2.12/x86_64-w64-mingw32" 
    CACHE PATH "指向您的 SDL3 开发库根目录的路径 (例如 .../x86_64-w64-mingw32)")
set(SDL3_TTF_ARCH_DEVEL_PATH "../SDL3_ttf-devel-3.2.2-mingw/SDL3_ttf-3.2.2/x86_64-w64-mingw32" 
    CACHE PATH "指向您的 SDL3_ttf 开发库根目录的路径 (例如 .../x86_64-w64-mingw32)")
# 如果需要 SDL_image:
# set(SDL3_IMAGE_ARCH_DEVEL_PATH "您的/SDL3_image/路径" CACHE PATH "指向您的 SDL3_image 开发库的路径") 

# 检查您配置的路径是否有效 (可选但推荐)
if(NOT (IS_DIRECTORY "${SDL3_ARCH_DEVEL_PATH}/include/SDL3" AND IS_DIRECTORY "${SDL3_ARCH_DEVEL_PATH}/lib"))
    message(WARNING "SDL3 开发库路径 (${SDL3_ARCH_DEVEL_PATH}) 可能无效或不完整。请确保它指向包含 include/SDL3 和 lib 子目录的 SDL3 开发库根目录。")
endif()
if(NOT (IS_DIRECTORY "${SDL3_TTF_ARCH_DEVEL_PATH}/include/SDL3_ttf" AND IS_DIRECTORY "${SDL3_TTF_ARCH_DEVEL_PATH}/lib"))
    message(WARNING "SDL3_ttf 开发库路径 (${SDL3_TTF_ARCH_DEVEL_PATH}) 可能无效或不完整。请确保它指向包含 include/SDL3_ttf 和 lib 子目录的 SDL3_ttf 开发库根目录。")
endif()

# --- 添加头文件包含路径 ---
# ${CMAKE_CURRENT_SOURCE_DIR} 指向包含当前 CMakeLists.txt 文件的目录，即项目根目录。
# 这确保了项目内部的头文件（如 Game.h, Board.h 等）能被正确找到。
include_directories(
    "${SDL3_ARCH_DEVEL_PATH}/include"    
    "${SDL3_TTF_ARCH_DEVEL_PATH}/include" 
    # 如果需要 SDL_image:
    # "${SDL3_IMAGE_ARCH_DEVEL_PATH}/include" 
    ${CMAKE_CURRENT_SOURCE_DIR} 
)

# --- 添加库文件搜索路径 ---
# 这告诉链接器在哪里查找 SDL3 和 SDL3_ttf 的库文件 (例如 .lib, .a)。
# 对于运行时 DLL，您仍然需要确保它们在可执行文件旁边或在系统的 PATH 中。
link_directories(
    "${SDL3_ARCH_DEVEL_PATH}/lib"
    "${SDL3_TTF_ARCH_DEVEL_PATH}/lib"
    # 如果需要 SDL_image:
    # "${SDL3_IMAGE_ARCH_DEVEL_PATH}/lib"
)

# --- 定义项目源文件列表 ---
# 所有 .cpp 文件都列在这里。
set(APP_SOURCES
    main.cpp 
    Board.cpp 
    Graphics.cpp 
    Game.cpp
    Constants.cpp 
    GreedyAI.cpp    
    AlphaBetaAI.cpp 
)

# --- 设置应用程序图标 (仅 Windows MinGW) ---
# 这部分代码会尝试查找项目根目录下的 icon.rc 文件，
# 如果找到，则将其添加到编译源中，以便为 .exe 文件嵌入图标。
if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "GNU") # 检查是否为Windows且使用GNU编译器(通常是MinGW)
    set(RESOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc") 
    if(EXISTS "${RESOURCE_FILE}")
        message(STATUS "找到资源文件: ${RESOURCE_FILE}")
        list(APPEND APP_SOURCES ${RESOURCE_FILE}) # 将资源文件添加到源文件列表
    else()
        message(WARNING "资源文件 icon.rc 未在项目根目录找到。将不设置应用程序图标。")
    endif()
endif()
# ---------------------------------------------

# 添加可执行文件
# WibyuanGomoku_app 是您程序最终生成的可执行文件名。
# ${APP_SOURCES} 包含了所有的 .cpp 文件以及可能的 .rc 文件。
add_executable(WibyuanGomoku_app ${APP_SOURCES})

# --- 手动指定需要链接的库 ---
# PRIVATE 表示这些库仅 WibyuanGomoku_app 目标本身需要链接，不会传递给其他依赖此目标的库。
# mingw32: 对于 MinGW 编译器，链接此库通常是为了支持 WinMain 入口点（与 -mwindows 配合使用）。
# SDL3 和 SDL3_ttf: 链接 SDL 核心库和字体库的名称。link_directories 会帮助找到它们。
target_link_libraries(WibyuanGomoku_app PRIVATE 
    mingw32 
    SDL3 
    SDL3_ttf 
    # 如果需要 SDL_image:
    # SDL3_image 
)

# --- 设置链接器标志以隐藏控制台窗口 (仅 Windows MinGW) ---
# -mwindows 标志告诉链接器创建一个窗口应用程序而不是控制台应用程序。
if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set_target_properties(WibyuanGomoku_app PROPERTIES LINK_FLAGS "-mwindows")
endif()
# -----------------------------------------------------------------

# --- 关于 DLL 复制的提示 ---
# 这部分消息会在 CMake 配置完成时显示，您运行时可能需要手动复制 DLL。
if(WIN32)
    message(STATUS "CMakeLists.txt: 配置完成。请记得将必要的DLL文件 (例如 SDL3.dll, SDL3_ttf.dll, freetype.dll, zlib1.dll 以及可能的MinGW运行时DLL) 复制到最终可执行文件所在的目录。")
else()
    message(STATUS "CMakeLists.txt: 配置完成。")
endif()

message(STATUS "CMake 项目 WibyuanGomoku 已配置。")
