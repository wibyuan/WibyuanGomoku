# Wibyuan 五子棋游戏 (Wibyuan's Gomoku Game)

这是一个使用 C++ 和 SDL3 开发的五子棋游戏，具有图形用户界面，并包含不同难度的人工智能对手。

## 特色功能

* 人人对战模式
* 人机对战模式：
    * 简单AI (基于贪心算法)
    * 困难AI (基于 Alpha-Beta 剪枝算法)
* 图形化棋盘和棋子显示
* 全屏/窗口模式切换 (快捷键: F11)
* 游戏暂停与继续 (快捷键: ESC 或 P键，在游戏中)
* “关于”界面（包含游戏规则和致谢）
* “更新日志”界面，展示版本迭代信息
* 最新落子高亮显示（5.17）
* 人机对战时显示当前回合方（to do）

## 截图

![img](https://picx.zhimg.com/80/v2-25881c952326cd91d945f8a897246d32_1440w.png)

![img](https://picx.zhimg.com/80/v2-40ce01462da001147618f73074629b71_1440w.png)

## 如何编译与运行

### 依赖项

* C++17 兼容的编译器 (例如 MinGW GCC, MSVC, Clang)
* CMake (建议版本 3.10 或更高)
* SDL3 开发库
* SDL3_ttf 开发库
* 字体文件：`msyh.ttc` (微软雅黑，用于显示中文)

### 构建步骤 (以 MinGW 为例)

1.  **克隆仓库**:
    ```bash
    git clone https://github.com/wibyuan/WibyuanGomoku.git
    cd WibyuanGomoku
    ```

2.  **准备 SDL 开发库**:
    本项目使用 CMake 进行构建，并通过 `CMakeLists.txt` 文件中的相对路径引用 SDL3 和 SDL3_ttf 开发库。为使默认配置生效，请按以下结构组织您的文件夹：
    ```
    <某个父目录>/
    ├── SDL3-devel-3.2.12-mingw/  (包含 SDL3 的 x86_64-w64-mingw32 等子目录)
    │   └── SDL3-3.2.12/
    │       └── x86_64-w64-mingw32/
    │           ├── include/
    │           └── lib/
    ├── SDL3_ttf-devel-3.2.2-mingw/ (包含 SDL3_ttf 的 x86_64-w64-mingw32 等子目录)
    │   └── SDL3_ttf-3.2.2/
    │       └── x86_64-w64-mingw32/
    │           ├── include/
    │           └── lib/
    └── WibyuanGomoku/ (您的项目仓库根目录，包含 CMakeLists.txt)
        ├── CMakeLists.txt
        ├── main.cpp
        └── ... (其他项目文件)
    ```
    简而言之，将 `SDL3-devel-3.2.12-mingw` 和 `SDL3_ttf-devel-3.2.2-mingw` 这两个文件夹放置在与 `WibyuanGomoku` 项目文件夹**同级**的目录中。

3.  **配置 SDL 依赖路径 (如果您的 SDL 库不在上述推荐的相对位置)**:
    如果您将 SDL3 和 SDL3_ttf 开发库安装在其他位置，您需要在首次运行 CMake 配置时通过命令行参数覆盖默认路径：
    * 打开命令行，进入您克隆的 `WibyuanGomoku` 目录。
    * 创建并进入构建目录：
        ```bash
        mkdir build
        cd build
        ```
    * 运行 CMake 并指定路径：
        ```bash
        cmake .. -G "MinGW Makefiles" \
                -DSDL3_ARCH_DEVEL_PATH="您本地的/SDL3/开发库/x86_64-w64-mingw32" \
                -DSDL3_TTF_ARCH_DEVEL_PATH="您本地的/SDL3_ttf/开发库/x86_64-w64-mingw32"
        ```
        **示例**:
        ```bash
        cmake .. -G "MinGW Makefiles" \
                -DSDL3_ARCH_DEVEL_PATH="C:/my_sdl_libs/SDL3-3.2.12/x86_64-w64-mingw32" \
                -DSDL3_TTF_ARCH_DEVEL_PATH="C:/my_sdl_libs/SDL3_ttf-3.2.2/x86_64-w64-mingw32"
        ```
        如果您使用 CMake GUI，也可以在配置后在缓存变量列表中找到并修改 `SDL3_ARCH_DEVEL_PATH` 和 `SDL3_TTF_ARCH_DEVEL_PATH`。

4.  **编译项目**:
    在 `build` 目录下运行：
    ```bash
    cmake --build .
    ```
    或者，如果使用的是 MinGW Makefiles，可以直接运行 `mingw32-make`。

5.  **准备运行环境**:
    编译成功后，可执行文件（例如 `WibyuanGomoku_app.exe`）会生成在 `build` 目录（或者您在 CMake 中指定的输出目录，通常是 `build/Debug/` 或 `build/Release/`）。
    要运行游戏，您需要将以下文件复制到与可执行文件**相同的目录**下：
    * `SDL3.dll` (来自您配置的 SDL3 开发库的 `bin` 目录)
    * `SDL3_ttf.dll` (来自您配置的 SDL3_ttf 开发库的 `bin` 目录)
    * `msyh.ttc` (字体文件，您需要将其放在与可执行文件同级，或者修改 `Constants.cpp` 中的 `FONT_PATH` 指向其正确位置)
    * 可能的 MinGW 运行时 DLL（如果您的编译器是动态链接运行时的，例如 `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll`）。
    * `zlib1.dll` (通常是 SDL_ttf 的依赖，也需要从 SDL_ttf 或其依赖的 FreeType 的 `bin` 目录获取)。

6.  **运行游戏**:
    双击 `WibyuanGomoku_app.exe`。

## 游戏玩法

* 通过主菜单选择游戏模式。
* 黑子先手，玩家轮流在棋盘的交叉点落子。
* 先在横向、纵向或斜向连成五个己方棋子者获胜。
* 本游戏为无禁手模式。
* 按 `F11` 键切换全屏/窗口模式。
* 游戏中按 `ESC` 键或 `P` 键暂停游戏。

## 更新日志

请查看游戏内的“更新日志”功能了解最新的改动。

## 致谢

* 游戏作者: wibyuan (知乎: [https://www.zhihu.com/people/wib-85](https://www.zhihu.com/people/wib-85))
* AI 与代码协助: Gemini (Google)
* 使用的库: SDL3, SDL3_ttf (SDL_ttf 内部使用了 FreeType 和 HarfBuzz 等库进行字体渲染)

## 许可证

本项目采用 **MIT License** 许可证。详情请参阅项目中的 `LICENSE` 文件：[LICENSE](https://github.com/wibyuan/WibyuanGomoku/blob/main/LICENSE)