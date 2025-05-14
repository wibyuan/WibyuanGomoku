#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>
#include "Constants.h" // 确保Constants.h在此之前被包含或其内容在此可用
class Board; // 前向声明

class Graphics {
public:
    Graphics();
    ~Graphics();

    bool isInitialized() const;

    void clearScreen();
    void drawBoardGrid();
    void drawPieces(const Board& board);
    void renderText(const std::string& text, int x, int y, SDL_Color color);
    void presentScreen();
    SDL_Renderer* getRenderer();
    SDL_Window* getWindow(); // 新增：获取窗口指针，方便Game类操作
    void setWindowTitle(const std::string& title);
    SDL_Point getTextDimensions(const std::string& text);
    TTF_Font* getFont();

    void toggleFullscreen(); // 新增：切换全屏模式的方法

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    bool initialized;
    bool isCurrentlyFullscreen; // 新增：跟踪当前是否为全屏状态

    // 存储原始窗口尺寸以便恢复 (如果需要更复杂的恢复逻辑)
    // int originalWindowWidth;
    // int originalWindowHeight;


    bool loadFont();
    void fillCircle(int centerX, int centerY, int radius, SDL_Color color);
    void drawCircle(int centerX, int centerY, int radius, SDL_Color color);
};

#endif // GRAPHICS_H
