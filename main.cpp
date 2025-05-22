// Copyright (c) 2025 wibyuan
// Licensed under the MIT License (see LICENSE for details)
#include "Game.h" // 包含我们的游戏主类头文件
#include <SDL3/SDL_main.h> // 包含 SDL 的 main 宏定义 (如果需要)
#include <iostream> // 用于错误输出

// C++ 程序的入口点
int main(int argc, char* argv[]) {
    
    // 创建 Game 类的对象
    // Game 类的构造函数会负责初始化 SDL、创建窗口/渲染器、加载资源等
    Game gomokuGame; 

    // 运行游戏主循环
    // Game 类的 run() 方法会处理事件、更新状态和渲染画面
    gomokuGame.run(); 

    // run() 方法结束后，Game 对象的析构函数会被调用，负责清理资源

    return 0; // 程序正常退出
}
