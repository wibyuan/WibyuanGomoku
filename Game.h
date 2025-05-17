#ifndef GAME_H
#define GAME_H

#include <string>
#include <memory>
#include <vector>
#include <array>
#include <map>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include "Board.h"
#include "Graphics.h"
#include "Constants.h"
#include "Player.h" // Point 结构体在此定义

// 定义游戏状态枚举
enum class GameState {
    MENU,                   // 菜单状态
    PLAYING,                // 游戏进行中状态
    PAUSED,                 // 暂停状态
    GAME_OVER,              // 游戏结束状态
    ABOUT_SCREEN,           // 关于界面状态
    TASK_LOG_SCREEN         // 任务日志界面状态
};

// 定义 AI 难度级别枚举
enum class AIDifficulty {
    HUMAN,      // 人类玩家
    GREEDY,     // 简单AI (贪心)
    ALPHA_BETA  // 困难AI (Alpha-Beta剪枝)
};

// --- 主菜单选项枚举 ---
enum class MainMenuOption {
    PLAYER_VS_PLAYER,           // 玩家对战玩家
    HUMAN_AS_BLACK_VS_GREEDY,   // 人类执黑 vs 简单AI
    HUMAN_AS_WHITE_VS_GREEDY,   // 人类执白 vs 简单AI
    HUMAN_AS_BLACK_VS_ALPHABETA,// 人类执黑 vs 困难AI
    HUMAN_AS_WHITE_VS_ALPHABETA,// 人类执白 vs 困难AI
    SHOW_ABOUT,                 // 显示关于界面
    SHOW_TASK_LOG,              // 显示任务日志选项
    EXIT_GAME                   // 退出游戏
};


// Game 类声明
class Game {
public:
    Game();  // 构造函数
    ~Game(); // 析构函数
    void run(); // 运行游戏主循环

private:
    // --- 核心游戏成员变量 ---
    Board board;         // 棋盘对象
    Graphics graphics;   // 图形处理对象
    int currentPlayer;   // 当前玩家 (BLACK_PIECE 或 WHITE_PIECE)
    bool gameOver;       // 游戏是否结束的标志
    std::string gameMessage; // 游戏结束时的消息 (例如 "黑子获胜！")
    SDL_Color messageColor;  // 游戏消息的颜色
    bool quit;           // 是否退出游戏的标志
    GameState currentState; // 当前游戏状态
    bool isFullscreen;   // 新增：跟踪游戏是否处于全屏模式

    Point lastPlayedMove; // 新增：记录最后一次落子的位置

    // --- 玩家管理 ---
    std::array<std::unique_ptr<Player>, 3> players;
    std::array<AIDifficulty, 3> playerTypes;

    // --- UI元素矩形区域 ---
    std::map<MainMenuOption, SDL_FRect> menuButtons;
    std::map<std::string, SDL_FRect> pauseMenuButtons;
    SDL_FRect pauseButtonRect;
    SDL_FRect gameOverMenuButtonRect;

    // --- 关于界面变量 ---
    SDL_FRect aboutBoxRect;
    SDL_FRect aboutBoxCloseButtonRect;
    bool isDraggingAboutBox;
    SDL_Point dragOffset; // SDL_Point for UI dragging, distinct from game's Point struct
    std::vector<std::string> rulesTextLines;
    std::vector<std::string> creditsTextLines;
    int aboutTextScrollOffsetY;
    int totalAboutTextHeight;
    SDL_Rect aboutTextViewport;

    // --- 任务日志界面变量 ---
    SDL_FRect taskLogBoxRect;
    SDL_FRect taskLogBoxCloseButtonRect;
    bool isDraggingTaskLogBox;
    SDL_Point dragOffsetTaskLog; // SDL_Point for UI dragging
    std::vector<std::string> taskLogTextLines;
    int taskLogTextScrollOffsetY;
    int totalTaskLogTextHeight;
    SDL_Rect taskLogTextViewport;

    // --- 私有方法声明 ---

    // 初始化方法
    void initializeMenuButtons();
    void initializePauseMenuButtons();
    void initializeGameUI();
    void initializeGameOverUI();
    void initializeAboutScreen();
    void initializeTaskLogScreen();

    // 事件处理方法
    void handleGlobalEvents(const SDL_Event& event);
    void handleMenuEvents(const SDL_Event& event);
    void handlePauseEvents(const SDL_Event& event);
    void handleGameEvents(const SDL_Event& event);
    void handleAboutScreenEvents(const SDL_Event& event);
    void handleTaskLogScreenEvents(const SDL_Event& event);

    // 游戏逻辑更新方法
    void update(int row, int col);

    // 渲染方法
    void renderMenu();
    void renderPauseMenu();
    void renderGame();
    void renderAboutScreen();
    void renderTaskLogScreen();
    void render();

    // 其他辅助方法
    void resetGameInternals();
    void aiTurn();
    std::unique_ptr<Player> createPlayer(AIDifficulty type);
    void startNewGame(AIDifficulty p1Type, AIDifficulty p2Type);

    void recalculateUIForNewSize(int newWidth, int newHeight);
};

#endif // GAME_H
