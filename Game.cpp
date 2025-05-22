// Copyright (c) 2025 wibyuan
// Licensed under the MIT License (see LICENSE for details)
#include "Game.h"
#include "Constants.h"
#include "GreedyAI.h"
#include "AlphaBetaAI.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <cstring> 
#include <iomanip>
#include <sstream>
#include <chrono>

// 根据类型创建 Player 对象
std::unique_ptr<Player> Game::createPlayer(AIDifficulty type) {
    switch(type) {
        case AIDifficulty::GREEDY:
            std::cout << "[调试] 正在创建简单AI玩家 (GreedyAI)。" << std::endl;
            return std::make_unique<GreedyAI>();
        case AIDifficulty::ALPHA_BETA:
            std::cout << "[调试] 正在创建困难AI玩家 (AlphaBetaAI)。" << std::endl;
            return std::make_unique<AlphaBetaAI>();
        case AIDifficulty::HUMAN:
        default:
             std::cout << "[调试] 设置玩家为人类 (返回 nullptr)。" << std::endl;
            return nullptr;
    }
}

// 构造函数
Game::Game() :
    graphics(),
    board(),
    currentPlayer(BLACK_PIECE),
    gameOver(false),
    quit(false),
    gameMessage(""),
    messageColor({0,0,0,255}),
    currentState(GameState::MENU),
    isFullscreen(false),
    lastPlayedMove({-1, -1}), // 初始化 lastPlayedMove
    isDraggingAboutBox(false),
    aboutTextScrollOffsetY(0),
    totalAboutTextHeight(0),
    isDraggingTaskLogBox(false),
    taskLogTextScrollOffsetY(0),
    totalTaskLogTextHeight(0)
{
    if (!graphics.isInitialized()) {
        std::cerr << "游戏错误: 图形系统初始化失败。正在退出。" << std::endl;
        quit = true;
        return;
    }

    playerTypes[BLACK_PIECE] = AIDifficulty::HUMAN; 
    playerTypes[WHITE_PIECE] = AIDifficulty::HUMAN;

    initializeMenuButtons();
    initializePauseMenuButtons();
    initializeGameUI();
    initializeGameOverUI();
    initializeAboutScreen();
    initializeTaskLogScreen(); 

    std::cout << "[调试] 游戏已初始化，当前状态：菜单。" << std::endl;
}

// 析构函数
Game::~Game() {
    std::cout << "[调试] Game 析构函数被调用。" << std::endl;
}

// 初始化主菜单按钮
void Game::initializeMenuButtons() {
    const int buttonWidth = 350; 
    const int buttonHeight = 45; 
    const int buttonSpacing = 15; 
    int totalButtonHeight = 8 * buttonHeight + 6 * buttonSpacing;
    int startY = (SCREEN_HEIGHT - totalButtonHeight) / 2;
    // if (startY < BORDER_PADDING * 3) startY = BORDER_PADDING * 3; 
    int currentY = startY;
    int centerX = SCREEN_WIDTH / 2;

    menuButtons.clear();

    menuButtons[MainMenuOption::PLAYER_VS_PLAYER] = { static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(currentY), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight) };
    currentY += buttonHeight + buttonSpacing;
    menuButtons[MainMenuOption::HUMAN_AS_BLACK_VS_GREEDY] = { static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(currentY), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight) };
    currentY += buttonHeight + buttonSpacing;
    menuButtons[MainMenuOption::HUMAN_AS_WHITE_VS_GREEDY] = { static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(currentY), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight) };
    currentY += buttonHeight + buttonSpacing;
    menuButtons[MainMenuOption::HUMAN_AS_BLACK_VS_ALPHABETA] = { static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(currentY), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight) };
    currentY += buttonHeight + buttonSpacing;
    menuButtons[MainMenuOption::HUMAN_AS_WHITE_VS_ALPHABETA] = { static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(currentY), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight) };
    currentY += buttonHeight + buttonSpacing;
    menuButtons[MainMenuOption::SHOW_ABOUT] = { static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(currentY), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight) };
    currentY += buttonHeight + buttonSpacing;
    menuButtons[MainMenuOption::SHOW_TASK_LOG] = { static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(currentY), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight) };

    currentY += buttonHeight + buttonSpacing; 
    menuButtons[MainMenuOption::EXIT_GAME] = { static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(currentY), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight) };
}


// 初始化暂停菜单按钮
void Game::initializePauseMenuButtons() {
    const int buttonWidth = 250;
    const int buttonHeight = 50;
    const int buttonSpacing = 20;
    int centerX = SCREEN_WIDTH / 2;
    int startY = (SCREEN_HEIGHT - (2 * buttonHeight + buttonSpacing)) / 2;

    pauseMenuButtons["Continue"] = { static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(startY), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight) };
    pauseMenuButtons["BackToMenu"] = { static_cast<float>(centerX - buttonWidth / 2), static_cast<float>(startY + buttonHeight + buttonSpacing), static_cast<float>(buttonWidth), static_cast<float>(buttonHeight) };
}

// 初始化游戏界面UI
void Game::initializeGameUI() {
    const int buttonWidth = 80;
    const int buttonHeight = 30;
    pauseButtonRect = {
        static_cast<float>(SCREEN_WIDTH / 2 - buttonWidth / 2),
        static_cast<float>(SCREEN_HEIGHT - buttonHeight - BORDER_PADDING / 2),
        static_cast<float>(buttonWidth),
        static_cast<float>(buttonHeight)
    };
}

// 初始化游戏结束界面UI
void Game::initializeGameOverUI() {
    const int buttonWidth = 280;
    const int buttonHeight = 50;
    int buttonY = BORDER_PADDING + FONT_SIZE + 20 + FONT_SIZE + 30; 
    if (buttonY + buttonHeight > SCREEN_HEIGHT - BORDER_PADDING) { 
        buttonY = SCREEN_HEIGHT - BORDER_PADDING - buttonHeight;
    }
    int centerX = SCREEN_WIDTH / 2;

    gameOverMenuButtonRect = {
        static_cast<float>(centerX - buttonWidth / 2),
        static_cast<float>(buttonY),
        static_cast<float>(buttonWidth),
        static_cast<float>(buttonHeight)
    };
}

// 初始化关于界面
void Game::initializeAboutScreen() {
    const int boxWidth = SCREEN_WIDTH - 100;
    const int boxHeight = SCREEN_HEIGHT - 120;
    aboutBoxRect = { static_cast<float>(SCREEN_WIDTH / 2 - boxWidth / 2), static_cast<float>(SCREEN_HEIGHT / 2 - boxHeight / 2), static_cast<float>(boxWidth), static_cast<float>(boxHeight) };
    const int closeButtonSize = 25;
    const int closeButtonMargin = 8;
    aboutBoxCloseButtonRect = { aboutBoxRect.x + aboutBoxRect.w - closeButtonSize - closeButtonMargin, aboutBoxRect.y + closeButtonMargin, static_cast<float>(closeButtonSize), static_cast<float>(closeButtonSize) };
    isDraggingAboutBox = false;
    aboutTextScrollOffsetY = 0;
    totalAboutTextHeight = 0;

    rulesTextLines.clear();
    rulesTextLines.push_back("五子棋规则与游戏用法:"); rulesTextLines.push_back(" ");
    rulesTextLines.push_back("1. 黑子先手，双方轮流在棋盘交叉点落子。");
    rulesTextLines.push_back("2. 先在横、竖、斜任一方向形成连续五个"); rulesTextLines.push_back("   己方棋子者胜。");
    rulesTextLines.push_back("3. 本游戏为无禁手模式。"); 
    rulesTextLines.push_back("4. 在主菜单选择对战模式即可开始游戏。");
    rulesTextLines.push_back("5. 游戏中按 P 键或 ESC 键可暂停游戏。");
    rulesTextLines.push_back("6. 按 F11 键可切换全屏/窗口模式。");
    rulesTextLines.push_back("7. 游戏结束后点击“返回主菜单”按钮可"); rulesTextLines.push_back("   重新开始或退出。");
    rulesTextLines.push_back("8. 最新落子会以红色边框高亮显示。"); // 新增规则说明


    creditsTextLines.clear();
    creditsTextLines.push_back("致谢:"); creditsTextLines.push_back(" ");
    creditsTextLines.push_back("游戏作者: wibyuan (知乎)");
    creditsTextLines.push_back("AI 与代码协助: Gemini");
    creditsTextLines.push_back(" "); creditsTextLines.push_back("感谢您的游玩！");

    const int lineSpacing = 8;
    totalAboutTextHeight = 0;
    for(const auto& line : rulesTextLines) { totalAboutTextHeight += (FONT_SIZE + lineSpacing); }
    totalAboutTextHeight += FONT_SIZE; 
    for(const auto& line : creditsTextLines) { totalAboutTextHeight += (FONT_SIZE + lineSpacing); }
}

// 初始化任务日志界面
void Game::initializeTaskLogScreen() {
    const int boxWidth = SCREEN_WIDTH - 120;
    const int boxHeight = SCREEN_HEIGHT - 150;
    taskLogBoxRect = { static_cast<float>(SCREEN_WIDTH / 2 - boxWidth / 2), static_cast<float>(SCREEN_HEIGHT / 2 - boxHeight / 2), static_cast<float>(boxWidth), static_cast<float>(boxHeight) };
    const int closeButtonSize = 25;
    const int closeButtonMargin = 8;
    taskLogBoxCloseButtonRect = { taskLogBoxRect.x + taskLogBoxRect.w - closeButtonSize - closeButtonMargin, taskLogBoxRect.y + closeButtonMargin, static_cast<float>(closeButtonSize), static_cast<float>(closeButtonSize) };
    isDraggingTaskLogBox = false;
    taskLogTextScrollOffsetY = 0;
    totalTaskLogTextHeight = 0;

    taskLogTextLines.clear();
    taskLogTextLines.push_back("版本 1.1451 (2025-05-17):"); 
    taskLogTextLines.push_back("新增: 最新落子红色边框高亮显示功能。");
    taskLogTextLines.push_back("修复: 修复了终局的 UI 颜色显示问题。");
    taskLogTextLines.push_back(" ");
    taskLogTextLines.push_back("版本 1.145 (2025-05-14):"); 
    taskLogTextLines.push_back("新增: 全屏模式切换功能 (按 F11)。");
    taskLogTextLines.push_back("提示: 棋盘和UI元素将随窗口缩放。");
    taskLogTextLines.push_back(" ");
    taskLogTextLines.push_back("版本 1.14 (2025-05-13):");
    taskLogTextLines.push_back("新增: 任务日志功能！现在您可以查看");
    taskLogTextLines.push_back("游戏的更新历史。");
    taskLogTextLines.push_back("优化: 玩家落子后棋盘立即刷新，提升");
    taskLogTextLines.push_back("体验。");
    taskLogTextLines.push_back(" ");
    taskLogTextLines.push_back("版本 1.1 (2025-05-10):");
    taskLogTextLines.push_back("新增: “关于”界面，包含游戏规则和");
    taskLogTextLines.push_back("致谢。");
    taskLogTextLines.push_back("修复: 若干潜在的UI显示问题。");
    taskLogTextLines.push_back(" ");
    taskLogTextLines.push_back("版本 1.0 (2025-05-09):");
    taskLogTextLines.push_back("游戏初始版本发布。");
    taskLogTextLines.push_back("支持三种模式: ");
    taskLogTextLines.push_back("人人对战。");
    taskLogTextLines.push_back("人机对战 (简单AI - 贪心算法)。");
    taskLogTextLines.push_back("人机对战 (困难AI - AlphaBeta 剪枝)。");

    const int lineSpacing = 8;
    totalTaskLogTextHeight = 0;
    for(const auto& line : taskLogTextLines) { totalTaskLogTextHeight += (FONT_SIZE + lineSpacing); }
}


// 运行游戏主循环
void Game::run() {
    if (quit) { return; }
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            handleGlobalEvents(event); 

            if (quit) break; 

            switch(currentState) {
                case GameState::MENU:
                    handleMenuEvents(event);
                    break;
                case GameState::PLAYING:
                case GameState::GAME_OVER:
                    handleGameEvents(event);
                    break;
                case GameState::PAUSED:
                    handlePauseEvents(event);
                    break;
                case GameState::ABOUT_SCREEN:
                    handleAboutScreenEvents(event);
                    break;
                case GameState::TASK_LOG_SCREEN:
                    handleTaskLogScreenEvents(event);
                    break;
            }
        }
        if (quit) break;

        if (currentState == GameState::PLAYING && !gameOver && players[currentPlayer]) {
             aiTurn();
        }

        render(); 
        SDL_Delay(16); 
    }
}

// 处理全局事件
void Game::handleGlobalEvents(const SDL_Event& event) {
    if (event.type == SDL_EVENT_QUIT) {
        quit = true;
    }
    if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.scancode == SDL_SCANCODE_F11) {
            graphics.toggleFullscreen();
            isFullscreen = !isFullscreen; 
            std::cout << "[调试] F11按下，切换全屏状态至: " << (isFullscreen ? "开" : "关") << std::endl;
            recalculateUIForNewSize(SCREEN_WIDTH, SCREEN_HEIGHT); 
        }
        else if (event.key.scancode == SDL_SCANCODE_ESCAPE || (currentState == GameState::PLAYING && event.key.scancode == SDL_SCANCODE_P)) {
            if (currentState == GameState::PLAYING) {
                currentState = GameState::PAUSED;
                 std::cout << "[调试] 游戏暂停。" << std::endl;
            } else if (currentState == GameState::PAUSED) {
                currentState = GameState::PLAYING;
                std::cout << "[调试] 游戏继续。" << std::endl;
            } else if (currentState == GameState::ABOUT_SCREEN) {
                currentState = GameState::MENU;
                aboutTextScrollOffsetY = 0; 
            } else if (currentState == GameState::TASK_LOG_SCREEN) {
                currentState = GameState::MENU;
                taskLogTextScrollOffsetY = 0; 
            }
        }
         if (event.key.scancode == SDL_SCANCODE_ESCAPE && 
            (currentState == GameState::MENU || currentState == GameState::GAME_OVER)) {
            quit = true;
        }
    }
}


// 处理菜单事件
void Game::handleMenuEvents(const SDL_Event& event) {
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            float logical_x, logical_y;
            SDL_RenderCoordinatesFromWindow(graphics.getRenderer(), static_cast<float>(event.button.x), static_cast<float>(event.button.y), &logical_x, &logical_y);
            SDL_FPoint mousePoint = {logical_x, logical_y};

            for (const auto& pair : menuButtons) {
                if (SDL_PointInRectFloat(&mousePoint, &pair.second)) {
                    MainMenuOption selectedOption = pair.first;
                    switch(selectedOption) {
                        case MainMenuOption::PLAYER_VS_PLAYER: 
                            startNewGame(AIDifficulty::HUMAN, AIDifficulty::HUMAN); break;
                        case MainMenuOption::HUMAN_AS_BLACK_VS_GREEDY: 
                            startNewGame(AIDifficulty::HUMAN, AIDifficulty::GREEDY); break;
                        case MainMenuOption::HUMAN_AS_WHITE_VS_GREEDY: 
                            startNewGame(AIDifficulty::GREEDY, AIDifficulty::HUMAN); break;
                        case MainMenuOption::HUMAN_AS_BLACK_VS_ALPHABETA: 
                            startNewGame(AIDifficulty::HUMAN, AIDifficulty::ALPHA_BETA); break;
                        case MainMenuOption::HUMAN_AS_WHITE_VS_ALPHABETA: 
                            startNewGame(AIDifficulty::ALPHA_BETA, AIDifficulty::HUMAN); break;
                        case MainMenuOption::SHOW_ABOUT: 
                            currentState = GameState::ABOUT_SCREEN; aboutTextScrollOffsetY = 0; break;
                        case MainMenuOption::SHOW_TASK_LOG: 
                            currentState = GameState::TASK_LOG_SCREEN; taskLogTextScrollOffsetY = 0; break;
                        case MainMenuOption::EXIT_GAME: 
                            quit = true; break;
                    }
                    if (quit || currentState != GameState::MENU) break; 
                }
            }
        }
    }
}

// 处理暂停菜单事件
void Game::handlePauseEvents(const SDL_Event& event) {
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            float logical_x, logical_y;
            SDL_RenderCoordinatesFromWindow(graphics.getRenderer(), static_cast<float>(event.button.x), static_cast<float>(event.button.y), &logical_x, &logical_y);
            SDL_FPoint mousePoint = {logical_x, logical_y};

            if (pauseMenuButtons.count("Continue") && SDL_PointInRectFloat(&mousePoint, &pauseMenuButtons.at("Continue"))) {
                currentState = GameState::PLAYING;
            } else if (pauseMenuButtons.count("BackToMenu") && SDL_PointInRectFloat(&mousePoint, &pauseMenuButtons.at("BackToMenu"))) {
                currentState = GameState::MENU; gameOver = false; gameMessage = "";
                lastPlayedMove = {-1, -1}; // 重置高亮
                graphics.setWindowTitle("Wibyuan's Gomoku Game");
            }
        }
    }
}

// 处理游戏事件
void Game::handleGameEvents(const SDL_Event& event) {
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            float logical_x, logical_y;
            SDL_RenderCoordinatesFromWindow(graphics.getRenderer(), static_cast<float>(event.button.x), static_cast<float>(event.button.y), &logical_x, &logical_y);
            SDL_FPoint mousePoint = {logical_x, logical_y};
            
            if (currentState == GameState::GAME_OVER && SDL_PointInRectFloat(&mousePoint, &gameOverMenuButtonRect)) {
                currentState = GameState::MENU; gameOver = false; gameMessage = "";
                lastPlayedMove = {-1, -1}; // 重置高亮
                graphics.setWindowTitle("Wibyuan's Gomoku Game");
            } else if (currentState == GameState::PLAYING && SDL_PointInRectFloat(&mousePoint, &pauseButtonRect)) {
                currentState = GameState::PAUSED;
                std::cout << "[调试] 点击暂停按钮，游戏暂停。" << std::endl;
            } else if (currentState == GameState::PLAYING && !gameOver && !players[currentPlayer]) { 
                int boardClickedX_px = static_cast<int>(mousePoint.x) - BORDER_PADDING;
                int boardClickedY_px = static_cast<int>(mousePoint.y) - BORDER_PADDING;
                int col = static_cast<int>((static_cast<float>(boardClickedX_px) + CELL_SIZE / 2.0f) / CELL_SIZE);
                int row = static_cast<int>((static_cast<float>(boardClickedY_px) + CELL_SIZE / 2.0f) / CELL_SIZE);
                
                if (board.isValidMove(row, col)) { 
                    update(row, col);
                } else {
                    std::cout << "[调试] 人类玩家无效落子于 (" << row << ", " << col << ") - 该位置不可落子。" << std::endl;
                }
            }
        }
    }
}

// 处理关于界面事件
void Game::handleAboutScreenEvents(const SDL_Event& event) {
    float logical_x, logical_y;
    SDL_FPoint mousePoint = {0,0}; 

    if (event.type == SDL_EVENT_MOUSE_MOTION || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
         SDL_GetMouseState(&logical_x, &logical_y); 
         SDL_RenderCoordinatesFromWindow(graphics.getRenderer(), logical_x, logical_y, &logical_x, &logical_y);
         mousePoint = {logical_x, logical_y};
    }

    switch (event.type) {
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (SDL_PointInRectFloat(&mousePoint, &aboutBoxCloseButtonRect)) {
                    currentState = GameState::MENU; aboutTextScrollOffsetY = 0;
                } else if (mousePoint.y >= aboutBoxRect.y && mousePoint.y <= aboutBoxRect.y + 35 && 
                           mousePoint.x >= aboutBoxRect.x && mousePoint.x <= aboutBoxRect.x + aboutBoxRect.w - (aboutBoxCloseButtonRect.w + 10) ) {
                    isDraggingAboutBox = true;
                    dragOffset.x = static_cast<int>(mousePoint.x - aboutBoxRect.x);
                    dragOffset.y = static_cast<int>(mousePoint.y - aboutBoxRect.y);
                }
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event.button.button == SDL_BUTTON_LEFT) { isDraggingAboutBox = false; }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            if (isDraggingAboutBox) {
                aboutBoxRect.x = mousePoint.x - dragOffset.x;
                aboutBoxRect.y = mousePoint.y - dragOffset.y;
                if(aboutBoxRect.x < 0) aboutBoxRect.x = 0;
                if(aboutBoxRect.y < 0) aboutBoxRect.y = 0;
                if(aboutBoxRect.x + aboutBoxRect.w > SCREEN_WIDTH) aboutBoxRect.x = SCREEN_WIDTH - aboutBoxRect.w;
                if(aboutBoxRect.y + aboutBoxRect.h > SCREEN_HEIGHT) aboutBoxRect.y = SCREEN_HEIGHT - aboutBoxRect.h;
                aboutBoxCloseButtonRect.x = aboutBoxRect.x + aboutBoxRect.w - aboutBoxCloseButtonRect.w - 8; 
                aboutBoxCloseButtonRect.y = aboutBoxRect.y + 8;
            }
            break;
        case SDL_EVENT_MOUSE_WHEEL: 
            if (event.wheel.y > 0) { aboutTextScrollOffsetY -= CELL_SIZE; } 
            else if (event.wheel.y < 0) { aboutTextScrollOffsetY += CELL_SIZE; } 
            if (aboutTextScrollOffsetY < 0) { aboutTextScrollOffsetY = 0; }
            float textRenderAreaHeight = aboutBoxRect.h - (35 + 20); 
            if (totalAboutTextHeight > textRenderAreaHeight) { 
                if (aboutTextScrollOffsetY > totalAboutTextHeight - static_cast<int>(textRenderAreaHeight)) {
                    aboutTextScrollOffsetY = totalAboutTextHeight - static_cast<int>(textRenderAreaHeight);
                }
            } else { 
                aboutTextScrollOffsetY = 0; 
            }
            break;
    }
}

// 处理任务日志界面事件
void Game::handleTaskLogScreenEvents(const SDL_Event& event) {
    float logical_x, logical_y;
    SDL_FPoint mousePoint = {0,0};

    if (event.type == SDL_EVENT_MOUSE_MOTION || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
         SDL_GetMouseState(&logical_x, &logical_y);
         SDL_RenderCoordinatesFromWindow(graphics.getRenderer(), logical_x, logical_y, &logical_x, &logical_y);
         mousePoint = {logical_x, logical_y};
    }

    switch (event.type) {
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (SDL_PointInRectFloat(&mousePoint, &taskLogBoxCloseButtonRect)) {
                    currentState = GameState::MENU; taskLogTextScrollOffsetY = 0;
                } else if (mousePoint.y >= taskLogBoxRect.y && mousePoint.y <= taskLogBoxRect.y + 35 && 
                           mousePoint.x >= taskLogBoxRect.x && mousePoint.x <= taskLogBoxRect.x + taskLogBoxRect.w - (taskLogBoxCloseButtonRect.w + 10)) {
                    isDraggingTaskLogBox = true;
                    dragOffsetTaskLog.x = static_cast<int>(mousePoint.x - taskLogBoxRect.x);
                    dragOffsetTaskLog.y = static_cast<int>(mousePoint.y - taskLogBoxRect.y);
                }
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event.button.button == SDL_BUTTON_LEFT) { isDraggingTaskLogBox = false; }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            if (isDraggingTaskLogBox) {
                taskLogBoxRect.x = mousePoint.x - dragOffsetTaskLog.x;
                taskLogBoxRect.y = mousePoint.y - dragOffsetTaskLog.y;
                if(taskLogBoxRect.x < 0) taskLogBoxRect.x = 0;
                if(taskLogBoxRect.y < 0) taskLogBoxRect.y = 0;
                if(taskLogBoxRect.x + taskLogBoxRect.w > SCREEN_WIDTH) taskLogBoxRect.x = SCREEN_WIDTH - taskLogBoxRect.w;
                if(taskLogBoxRect.y + taskLogBoxRect.h > SCREEN_HEIGHT) taskLogBoxRect.y = SCREEN_HEIGHT - taskLogBoxRect.h;
                taskLogBoxCloseButtonRect.x = taskLogBoxRect.x + taskLogBoxRect.w - taskLogBoxCloseButtonRect.w - 8;
                taskLogBoxCloseButtonRect.y = taskLogBoxRect.y + 8;
            }
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            if (event.wheel.y > 0) { taskLogTextScrollOffsetY -= CELL_SIZE; } 
            else if (event.wheel.y < 0) { taskLogTextScrollOffsetY += CELL_SIZE; }
            if (taskLogTextScrollOffsetY < 0) { taskLogTextScrollOffsetY = 0; }
            float textRenderAreaHeight = taskLogBoxRect.h - (35 + 20); 
            if (totalTaskLogTextHeight > textRenderAreaHeight) {
                if (taskLogTextScrollOffsetY > totalTaskLogTextHeight - static_cast<int>(textRenderAreaHeight)) {
                    taskLogTextScrollOffsetY = totalTaskLogTextHeight - static_cast<int>(textRenderAreaHeight);
                }
            } else { taskLogTextScrollOffsetY = 0; }
            break;
    }
}


// AI回合
void Game::aiTurn() {
     if (!players[currentPlayer] || gameOver) return;
     
     Point aiMove = players[currentPlayer]->getMove(board, currentPlayer);
     
     if (board.isValidMove(aiMove.row, aiMove.col)) { 
        update(aiMove.row, aiMove.col);
     } else {
        std::cerr << "[AI 错误] AI (" << currentPlayer << ") 尝试了一个无效的落子点: ("
                  << aiMove.row << "," << aiMove.col << "). 这不应该发生。" << std::endl;
        bool foundFallback = false;
        for(int r=0; r<BOARD_ROWS; ++r) {
            for(int c=0; c<BOARD_COLS; ++c) {
                if(board.isValidMove(r,c)){
                    update(r,c);
                    foundFallback = true;
                    goto ai_fallback_done_highlight_preserve; 
                }
            }
        }
        ai_fallback_done_highlight_preserve:; // Label for goto
        if(!foundFallback) { 
             if (!gameOver) {
                gameOver = true; currentState = GameState::GAME_OVER;
                gameMessage = "平局 (AI无法移动)!"; messageColor = {50,50,250,255};
                std::cout << "[信息] " << gameMessage << std::endl;
            }
        }
     }
}

// 更新游戏逻辑
void Game::update(int row, int col) {
    int playerWhoMoved = currentPlayer;
    bool isHumanPlayer = !players[playerWhoMoved];

    // 1. 放置棋子
    bool placed = board.placePiece(row, col, playerWhoMoved);

    if (placed) {
        lastPlayedMove = {row, col}; // 更新最后落子位置
        std::cout << "[信息] 玩家 " << playerWhoMoved << " 在 (" << row << ", " << col << ") 落子。" << std::endl;
        if (isHumanPlayer && currentState == GameState::PLAYING && !gameOver) {
            render(); 
            SDL_Delay(10); 
        }

        // 2. 检查胜负
        if (board.checkWin(row, col, playerWhoMoved)) {
            gameOver = true;
            currentState = GameState::GAME_OVER;
            gameMessage = (playerWhoMoved == BLACK_PIECE) ? "黑子获胜！" : "白子获胜！";
            messageColor = (playerWhoMoved == BLACK_PIECE) ? SDL_Color{0,0,0,255} : SDL_Color{255,255,255,255}; 
        } else if (board.isFull()) {
            gameOver = true;
            currentState = GameState::GAME_OVER;
            gameMessage = "平局！";
            messageColor = {50,50,250,255}; 
        } else {
            // 3. 切换玩家
            currentPlayer = (currentPlayer == BLACK_PIECE) ? WHITE_PIECE : BLACK_PIECE;
        }
        if(gameOver) { std::cout << "[信息] " << gameMessage << std::endl; }
    } else {
        if (isHumanPlayer) {
             std::cout << "[调试] 人类玩家无效落子于 (" << row << ", " << col << ") - placePiece返回false" << std::endl;
        } else {
             std::cerr << "[AI 错误] AI (" << playerWhoMoved <<") placePiece 失败于 (" << row << ", " << col << ")" << std::endl;
        }
    }
}


// 主渲染函数
void Game::render() {
    graphics.clearScreen(); 

    switch(currentState) {
        case GameState::MENU:
            renderMenu();
            break;
        case GameState::PLAYING:
        case GameState::GAME_OVER: 
            renderGame();
            break;
        case GameState::PAUSED:
            renderGame(); 
            renderPauseMenu(); 
            break;
        case GameState::ABOUT_SCREEN:
            renderMenu(); 
            renderAboutScreen();
            break;
        case GameState::TASK_LOG_SCREEN:
            renderMenu(); 
            renderTaskLogScreen();
            break;
    }
    graphics.presentScreen();
}

// 渲染菜单
void Game::renderMenu() {
    SDL_SetRenderDrawColor(graphics.getRenderer(), 200, 200, 220, 255); 
    SDL_RenderClear(graphics.getRenderer());

    std::string title = "选择模式";
    SDL_Color titleColor = {0,0,100,255}; 
    SDL_Point titleDim = graphics.getTextDimensions(title);
    graphics.renderText(title, SCREEN_WIDTH/2 - titleDim.x/2, BORDER_PADDING * 2, titleColor);
    
    SDL_Color btnBgColor = {230,230,230,255}; 
    SDL_Color btnTextColor = {50,50,50,255};   
    SDL_Color exitBtnBgColor = {220,180,180,255}; 
    SDL_Color exitBtnTextColor = {100,0,0,255};   

    std::map<MainMenuOption, std::string> buttonTexts = {
        {MainMenuOption::PLAYER_VS_PLAYER, "人人对战"},
        {MainMenuOption::HUMAN_AS_BLACK_VS_GREEDY, "执黑 vs 简单AI"},
        {MainMenuOption::HUMAN_AS_WHITE_VS_GREEDY, "执白 vs 简单AI"},
        {MainMenuOption::HUMAN_AS_BLACK_VS_ALPHABETA, "执黑 vs 困难AI"}, 
        {MainMenuOption::HUMAN_AS_WHITE_VS_ALPHABETA, "执白 vs 困难AI"}, 
        {MainMenuOption::SHOW_ABOUT, "游戏说明与致谢"},
        {MainMenuOption::SHOW_TASK_LOG, "更新日志"},
        {MainMenuOption::EXIT_GAME, "退出游戏"}
    };

    for (const auto& pair : menuButtons) {
        MainMenuOption option = pair.first;
        if (buttonTexts.count(option)) { 
            const SDL_FRect& rect = pair.second;
            std::string text = buttonTexts.at(option); 
            
            SDL_Color currentBg = (option == MainMenuOption::EXIT_GAME) ? exitBtnBgColor : btnBgColor;
            SDL_Color currentFg = (option == MainMenuOption::EXIT_GAME) ? exitBtnTextColor : btnTextColor;

            SDL_SetRenderDrawColor(graphics.getRenderer(), currentBg.r, currentBg.g, currentBg.b, currentBg.a);
            SDL_RenderFillRect(graphics.getRenderer(), &rect);
            
            SDL_Point txtDim = graphics.getTextDimensions(text);
            graphics.renderText(text, 
                                static_cast<int>(rect.x + (rect.w - txtDim.x) / 2), 
                                static_cast<int>(rect.y + (rect.h - txtDim.y) / 2), 
                                currentFg);
        }
    }
}

// 渲染暂停菜单
void Game::renderPauseMenu() {
    SDL_SetRenderDrawBlendMode(graphics.getRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(graphics.getRenderer(), 0, 0, 0, 150); 
    SDL_FRect overlayRect = {0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT};
    SDL_RenderFillRect(graphics.getRenderer(), &overlayRect);
    SDL_SetRenderDrawBlendMode(graphics.getRenderer(), SDL_BLENDMODE_NONE);

    std::string title = "游戏暂停";
    SDL_Color titleColor={255,255,255,255}; 
    SDL_Point titleDim = graphics.getTextDimensions(title);
    graphics.renderText(title, SCREEN_WIDTH/2 - titleDim.x/2, SCREEN_HEIGHT/2 - 100, titleColor);

    SDL_Color btnTextColor={220,220,220,255};
    SDL_Color btnBgColor={70,70,90,255}; 

    if (pauseMenuButtons.count("Continue")) {
        const auto& rect = pauseMenuButtons.at("Continue");
        SDL_SetRenderDrawColor(graphics.getRenderer(),btnBgColor.r,btnBgColor.g,btnBgColor.b,btnBgColor.a);
        SDL_RenderFillRect(graphics.getRenderer(),&rect);
        std::string text = "继续游戏";
        SDL_Point txtDim=graphics.getTextDimensions(text);
        graphics.renderText(text, (int)(rect.x+(rect.w-txtDim.x)/2), (int)(rect.y+(rect.h-txtDim.y)/2),btnTextColor);
    }
    if (pauseMenuButtons.count("BackToMenu")) {
        const auto& rect = pauseMenuButtons.at("BackToMenu");
        SDL_SetRenderDrawColor(graphics.getRenderer(),btnBgColor.r,btnBgColor.g,btnBgColor.b,btnBgColor.a);
        SDL_RenderFillRect(graphics.getRenderer(),&rect);
        std::string text = "返回主菜单";
        SDL_Point txtDim=graphics.getTextDimensions(text);
        graphics.renderText(text, (int)(rect.x+(rect.w-txtDim.x)/2), (int)(rect.y+(rect.h-txtDim.y)/2),btnTextColor);
    }
}

// 渲染游戏界面
void Game::renderGame() {
    graphics.drawBoardGrid();
    graphics.drawPieces(board, lastPlayedMove); // 传递 lastPlayedMove

    if (currentState == GameState::GAME_OVER && !gameMessage.empty()) {
        SDL_Point msgDim = graphics.getTextDimensions(gameMessage);
        SDL_FRect msgBgRect = {
            static_cast<float>(SCREEN_WIDTH/2 - msgDim.x/2 - 10),
            static_cast<float>(BORDER_PADDING - 5),
            static_cast<float>(msgDim.x + 20),
            static_cast<float>(msgDim.y + 10)
        };
        SDL_SetRenderDrawColor(graphics.getRenderer(), 200, 200, 200, 200); 
        SDL_RenderFillRect(graphics.getRenderer(), &msgBgRect);

        graphics.renderText(gameMessage, SCREEN_WIDTH/2 - msgDim.x/2, BORDER_PADDING, messageColor);
        
        SDL_Color btnBgColor={100,150,200,255}; 
        SDL_Color btnTextColor={255,255,255,255}; 
        SDL_SetRenderDrawColor(graphics.getRenderer(),btnBgColor.r,btnBgColor.g,btnBgColor.b,btnBgColor.a);
        SDL_RenderFillRect(graphics.getRenderer(),&gameOverMenuButtonRect);
        std::string btnTxt="返回主菜单";
        SDL_Point btnDim=graphics.getTextDimensions(btnTxt);
        graphics.renderText(btnTxt, (int)(gameOverMenuButtonRect.x+(gameOverMenuButtonRect.w-btnDim.x)/2), 
                                   (int)(gameOverMenuButtonRect.y+(gameOverMenuButtonRect.h-btnDim.y)/2), btnTextColor);
    } else if (currentState == GameState::PLAYING) {
        SDL_SetRenderDrawColor(graphics.getRenderer(),180,180,180,255); 
        SDL_RenderFillRect(graphics.getRenderer(),&pauseButtonRect);
        SDL_Color btnTextColor={0,0,0,255}; 
        std::string pauseTxt="暂停";
        SDL_Point pauseDim=graphics.getTextDimensions(pauseTxt);
        graphics.renderText(pauseTxt, (int)(pauseButtonRect.x+(pauseButtonRect.w-pauseDim.x)/2), 
                                     (int)(pauseButtonRect.y+(pauseButtonRect.h-pauseDim.y)/2), btnTextColor);
    }
}


// 渲染关于界面
void Game::renderAboutScreen() {
    SDL_SetRenderDrawBlendMode(graphics.getRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(graphics.getRenderer(),220,220,230,245); SDL_RenderFillRect(graphics.getRenderer(),&aboutBoxRect);
    SDL_SetRenderDrawBlendMode(graphics.getRenderer(), SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(graphics.getRenderer(),80,80,100,255); SDL_RenderRect(graphics.getRenderer(),&aboutBoxRect);
    SDL_FRect titleBar={aboutBoxRect.x,aboutBoxRect.y,aboutBoxRect.w,35.f}; SDL_SetRenderDrawColor(graphics.getRenderer(),130,130,160,255); SDL_RenderFillRect(graphics.getRenderer(),&titleBar);
    std::string title="游戏说明与致谢"; SDL_Point titleDim=graphics.getTextDimensions(title);
    graphics.renderText(title, (int)(titleBar.x+(titleBar.w-titleDim.x)/2), (int)(titleBar.y+(titleBar.h-titleDim.y)/2), {255,255,255,255});
    SDL_SetRenderDrawColor(graphics.getRenderer(),220,80,80,255); SDL_RenderFillRect(graphics.getRenderer(),&aboutBoxCloseButtonRect);
    SDL_Point xDim=graphics.getTextDimensions("X"); graphics.renderText("X", (int)(aboutBoxCloseButtonRect.x+(aboutBoxCloseButtonRect.w-xDim.x)/2), (int)(aboutBoxCloseButtonRect.y+(aboutBoxCloseButtonRect.h-xDim.y)/2),{255,255,255,255});
    SDL_Color textColor={30,30,30,255}; const int lineSp=8, padX=20, padYTop=10, padYBot=10;
    aboutTextViewport = {(int)(aboutBoxRect.x+padX), (int)(aboutBoxRect.y+titleBar.h+padYTop), (int)(aboutBoxRect.w-2*padX), (int)(aboutBoxRect.h-titleBar.h-padYTop-padYBot)};
    SDL_SetRenderViewport(graphics.getRenderer(), &aboutTextViewport);
    int currentY_rel = 0 - aboutTextScrollOffsetY;
    for(const auto& line : rulesTextLines){ graphics.renderText(line,0,currentY_rel,textColor); currentY_rel+=FONT_SIZE+lineSp; }
    currentY_rel += FONT_SIZE/2;
    for(const auto& line : creditsTextLines){ graphics.renderText(line,0,currentY_rel,textColor); currentY_rel+=FONT_SIZE+lineSp; }
    SDL_SetRenderViewport(graphics.getRenderer(), NULL);
}

// 渲染任务日志界面
void Game::renderTaskLogScreen() {
    SDL_SetRenderDrawBlendMode(graphics.getRenderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(graphics.getRenderer(),210,230,210,245); SDL_RenderFillRect(graphics.getRenderer(),&taskLogBoxRect); 
    SDL_SetRenderDrawBlendMode(graphics.getRenderer(), SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(graphics.getRenderer(),70,100,70,255); SDL_RenderRect(graphics.getRenderer(),&taskLogBoxRect); 
    SDL_FRect titleBar={taskLogBoxRect.x,taskLogBoxRect.y,taskLogBoxRect.w,35.f}; SDL_SetRenderDrawColor(graphics.getRenderer(),110,140,110,255); SDL_RenderFillRect(graphics.getRenderer(),&titleBar); 
    std::string title="更新日志"; SDL_Point titleDim=graphics.getTextDimensions(title);
    graphics.renderText(title, (int)(titleBar.x+(titleBar.w-titleDim.x)/2), (int)(titleBar.y+(titleBar.h-titleDim.y)/2), {255,255,255,255});
    SDL_SetRenderDrawColor(graphics.getRenderer(),200,70,70,255); SDL_RenderFillRect(graphics.getRenderer(),&taskLogBoxCloseButtonRect); 
    SDL_Point xDim=graphics.getTextDimensions("X"); graphics.renderText("X", (int)(taskLogBoxCloseButtonRect.x+(taskLogBoxCloseButtonRect.w-xDim.x)/2), (int)(taskLogBoxCloseButtonRect.y+(taskLogBoxCloseButtonRect.h-xDim.y)/2),{255,255,255,255});
    SDL_Color textColor={10,30,10,255}; const int lineSp=8, padX=20, padYTop=10, padYBot=10; 
    taskLogTextViewport = {(int)(taskLogBoxRect.x+padX), (int)(taskLogBoxRect.y+titleBar.h+padYTop), (int)(taskLogBoxRect.w-2*padX), (int)(taskLogBoxRect.h-titleBar.h-padYTop-padYBot)};
    SDL_SetRenderViewport(graphics.getRenderer(), &taskLogTextViewport);
    int currentY_rel = 0 - taskLogTextScrollOffsetY;
    for(const auto& line : taskLogTextLines){ graphics.renderText(line,0,currentY_rel,textColor); currentY_rel+=FONT_SIZE+lineSp; }
    SDL_SetRenderViewport(graphics.getRenderer(), NULL);
}


// 重置游戏内部状态
void Game::resetGameInternals() {
    board.reset();
    currentPlayer = BLACK_PIECE;
    gameOver = false;
    gameMessage = "";
    lastPlayedMove = {-1, -1}; // 重置最后落子位置
    std::cout << "[信息] 游戏内部状态已重置。" << std::endl;
    graphics.setWindowTitle("Wibyuan's Gomoku Game");
}

// 开始新游戏
void Game::startNewGame(AIDifficulty p1Type, AIDifficulty p2Type) {
    players[BLACK_PIECE].reset(); 
    players[WHITE_PIECE].reset();

    playerTypes[BLACK_PIECE] = p1Type;
    playerTypes[WHITE_PIECE] = p2Type;
    
    players[BLACK_PIECE] = createPlayer(p1Type);
    players[WHITE_PIECE] = createPlayer(p2Type);
    
    resetGameInternals(); // 会重置 lastPlayedMove
    currentState = GameState::PLAYING;
    std::cout << "[信息] 开始新游戏。P1类型: " << static_cast<int>(p1Type) 
              << ", P2类型: " << static_cast<int>(p2Type) << std::endl;
    graphics.setWindowTitle("Wibyuan's Gomoku Game - 游戏中");
}


// 当窗口大小改变时，重新计算UI元素位置和大小
void Game::recalculateUIForNewSize(int newWidth, int newHeight) {
    std::cout << "[调试] recalculateUIForNewSize 调用，新尺寸: " << newWidth << "x" << newHeight << std::endl;
    initializeMenuButtons();
    initializePauseMenuButtons();
    initializeGameUI();
    initializeGameOverUI();
    initializeAboutScreen(); 
    initializeTaskLogScreen();
}
