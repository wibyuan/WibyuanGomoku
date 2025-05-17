#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>
#include "Constants.h" 
#include "Player.h" // For Point struct

class Board; // 前向声明

class Graphics {
public:
    Graphics();
    ~Graphics();

    bool isInitialized() const;

    void clearScreen();
    void drawBoardGrid();
    // 修改：增加 lastPlayedMove 参数用于高亮
    void drawPieces(const Board& board, const Point& lastPlayedMove);
    void renderText(const std::string& text, int x, int y, SDL_Color color);
    void presentScreen();
    SDL_Renderer* getRenderer();
    SDL_Window* getWindow(); 
    void setWindowTitle(const std::string& title);
    SDL_Point getTextDimensions(const std::string& text); // SDL_Point for UI dimensions
    TTF_Font* getFont();

    void toggleFullscreen(); 

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    bool initialized;
    bool isCurrentlyFullscreen; 

    bool loadFont();
    // 保持 fillCircle 和 drawCircle 为私有辅助函数
    void fillCircle(int centerX, int centerY, int radius, SDL_Color color);
    void drawCircle(int centerX, int centerY, int radius, SDL_Color color);
};

#endif // GRAPHICS_H
