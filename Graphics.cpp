// 在所有 #include 之前定义 SDL_ENABLE_OLD_NAMES
#define SDL_ENABLE_OLD_NAMES 

#include "Graphics.h"
#include "Constants.h"
#include "Board.h" // Graphics::drawPieces 需要 Board 定义
#include "Player.h" // For Point struct
#include <iostream>
#include <cstring>      // 为了 strlen
#include <cmath>        // 为了 M_PI 和 round

// 构造函数: 初始化所有图形相关的子系统和资源
Graphics::Graphics() : window(nullptr), renderer(nullptr), font(nullptr), initialized(false), isCurrentlyFullscreen(false) {
    // 1. 初始化 SDL 视频子系统
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "图形错误: SDL 初始化失败! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }

    // 2. 初始化 SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "图形错误: SDL_ttf 初始化失败! Error: " << SDL_GetError() << std::endl; 
        SDL_Quit();
        return;
    }
    std::cout << "[调试] SDL_ttf 初始化成功。" << std::endl;

    // 3. 创建窗口 (使用 Constants.h 中的 SCREEN_WIDTH, SCREEN_HEIGHT 作为初始尺寸)
    window = SDL_CreateWindow("Wibyuan's Gomoku Game",
                              SCREEN_WIDTH, SCREEN_HEIGHT,
                              SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE); 
    if (window == nullptr) {
        std::cerr << "图形错误: 窗口创建失败! SDL_Error: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return;
    }

    // 4. 创建渲染器
    renderer = SDL_CreateRenderer(window, NULL); 
    if (renderer == nullptr) {
         std::cerr << "图形错误: 渲染器创建失败! SDL Error: " << SDL_GetError() << std::endl;
         SDL_DestroyWindow(window);
         TTF_Quit();
         SDL_Quit();
         return;
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    /*
    if (SDL_SetRenderLogicalPresentation(renderer, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX, SDL_SCALEMODE_LINEAR) != 0) {
        std::cerr << "图形警告: 设置渲染器逻辑演示区域失败! SDL_Error: " << SDL_GetError() << std::endl;
    }
    */

    // 5. 加载字体
    if (!loadFont()) {
        std::cerr << "图形警告: 字体加载失败，文本渲染将不可用。" << std::endl;
    }

    initialized = true;
    std::cout << "[调试] 图形系统初始化成功。" << std::endl;
}

// 析构函数: 清理所有加载的资源和子系统
Graphics::~Graphics() {
    std::cout << "[调试] Graphics 析构函数被调用。" << std::endl;
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    TTF_Quit();
    SDL_Quit();
    std::cout << "[调试] SDL 子系统已退出。" << std::endl;
}

// 检查初始化是否成功
bool Graphics::isInitialized() const {
    return initialized;
}

// 加载字体
bool Graphics::loadFont() {
    #if defined(FONT_INDEX) // 这段后面极其不可能执行
        font = TTF_OpenFontIndex(FONT_PATH, FONT_SIZE, FONT_INDEX);
        if (font == nullptr) {
            std::cerr << "[调试] 加载字体 '" << FONT_PATH << "' (索引 " << FONT_INDEX << ") 失败! Error: " << SDL_GetError() << std::endl;
            return false;
        }
        std::cout << "[调试] 字体 '" << FONT_PATH << "' (索引 " << FONT_INDEX << ") 加载成功。" << std::endl;
    #else
        font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
        if (font == nullptr) {
            std::cerr << "[调试] 加载字体 '" << FONT_PATH << "' 失败! Error: " << SDL_GetError() << std::endl;
            return false;
        }
        std::cout << "[调试] 字体 '" << FONT_PATH << "' 加载成功。" << std::endl;
    #endif
    return true;
}

// 设置窗口标题
void Graphics::setWindowTitle(const std::string& title) {
    if (window) {
        SDL_SetWindowTitle(window, title.c_str());
    }
}

// 清屏
void Graphics::clearScreen() {
    if (!renderer) return;
    SDL_SetRenderDrawColor(renderer, 245, 222, 179, SDL_ALPHA_OPAQUE); 
    SDL_RenderClear(renderer);
}

// 绘制棋盘网格和标记点
void Graphics::drawBoardGrid() {
    if (!renderer) return;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); 
    for (int i = 0; i < BOARD_ROWS; ++i) {
        SDL_RenderLine(renderer, BORDER_PADDING, BORDER_PADDING + i * CELL_SIZE, BORDER_PADDING + (BOARD_COLS - 1) * CELL_SIZE, BORDER_PADDING + i * CELL_SIZE);
    }
    for (int i = 0; i < BOARD_COLS; ++i) {
        SDL_RenderLine(renderer, BORDER_PADDING + i * CELL_SIZE, BORDER_PADDING, BORDER_PADDING + i * CELL_SIZE, BORDER_PADDING + (BOARD_ROWS - 1) * CELL_SIZE);
    }
    int center_row = (BOARD_ROWS -1) / 2;
    int center_col = (BOARD_COLS -1) / 2;
    int star_offset = 3;
    SDL_Point star_points_indices[] = { 
        {center_col, center_row}, {star_offset, star_offset}, {BOARD_COLS - 1 - star_offset, star_offset},
        {star_offset, BOARD_ROWS - 1 - star_offset}, {BOARD_COLS - 1 - star_offset, BOARD_ROWS - 1 - star_offset}
    };
    SDL_Color dotColor = {0, 0, 0, SDL_ALPHA_OPAQUE};
    for(const auto& p_idx : star_points_indices) {
        fillCircle(BORDER_PADDING + p_idx.x * CELL_SIZE, BORDER_PADDING + p_idx.y * CELL_SIZE, DOT_RADIUS, dotColor);
    }
}

// 绘制棋子 (增加 lastPlayedMove 参数)
void Graphics::drawPieces(const Board& board_ref, const Point& lastPlayedMove) {
    if (!renderer) return;
    SDL_Color highlightColor = {255, 0, 0, 255}; 
    int highlightRadiusOuter = PIECE_RADIUS + 1; 

    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            int piece = board_ref.getPiece(r, c);
            if (piece != EMPTY_PIECE) {
                SDL_Color pieceColor = (piece == BLACK_PIECE) ? 
                                       SDL_Color{0, 0, 0, SDL_ALPHA_OPAQUE} :
                                       SDL_Color{255, 255, 255, SDL_ALPHA_OPAQUE};
                int centerX = BORDER_PADDING + c * CELL_SIZE;
                int centerY = BORDER_PADDING + r * CELL_SIZE;
                
                fillCircle(centerX, centerY, PIECE_RADIUS, pieceColor);

                if (piece == WHITE_PIECE) { 
                    drawCircle(centerX, centerY, PIECE_RADIUS, {0, 0, 0, SDL_ALPHA_OPAQUE});
                }

                if (r == lastPlayedMove.row && c == lastPlayedMove.col) {
                    drawCircle(centerX, centerY, highlightRadiusOuter, highlightColor);
                }
            }
        }
    }
}

// 渲染文本
void Graphics::renderText(const std::string& text, int x, int y, SDL_Color fgColor) {
    if (!renderer || !font || text.empty()) { 
        if (!font) std::cerr << "[错误] renderText 调用时字体未加载。" << std::endl;
        return;
    }
    const char* text_cstr = text.c_str();
    

    size_t text_length = strlen(text_cstr); 
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text_cstr, text_length, fgColor); 
    
    if (textSurface == nullptr) {
        std::cerr << "[错误] TTF_RenderText_Blended 失败! Error: " << SDL_GetError() << std::endl;
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr) {
        std::cerr << "[错误] 从渲染文本创建纹理失败! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroySurface(textSurface);
        return;
    }
    SDL_FRect renderQuad = {
        static_cast<float>(x), static_cast<float>(y),
        static_cast<float>(textSurface->w), static_cast<float>(textSurface->h)
    };
    SDL_DestroySurface(textSurface);
    if (SDL_RenderTexture(renderer, textTexture, nullptr, &renderQuad) != 0) {
         std::cerr << "[错误] 渲染纹理失败! SDL_Error: " << SDL_GetError() << std::endl;
    }
    SDL_DestroyTexture(textTexture);
}

// 获取文本渲染后的尺寸
SDL_Point Graphics::getTextDimensions(const std::string& text) {
    SDL_Point dimensions = {0, 0}; 
    if (!this->font || text.empty()) {
        if (!this->font) std::cerr << "[错误] getTextDimensions 调用时字体未加载。" << std::endl;
        return dimensions;
    }
    const char* text_cstr = text.c_str();

    SDL_Color tempColor = {0,0,0,0}; 
    size_t text_length = strlen(text_cstr);
    SDL_Surface* tempSurface = TTF_RenderText_Blended(font, text_cstr, text_length, tempColor);

    if (tempSurface) {
        dimensions.x = tempSurface->w;
        dimensions.y = tempSurface->h;
        SDL_DestroySurface(tempSurface);
    } else {
        // 根据编译器错误，TTF_GetError 未定义，尝试使用 SDL_GetError
        std::cerr << "[警告] getTextDimensions 通过渲染临时表面失败: " << SDL_GetError() << std::endl;
        dimensions.x = static_cast<int>(text.length() * FONT_SIZE * 0.6); 
        dimensions.y = FONT_SIZE;
    }
    return dimensions;
}


TTF_Font* Graphics::getFont() {
    return this->font;
}

void Graphics::presentScreen() {
    if (renderer) {
        SDL_RenderPresent(renderer);
    }
}

SDL_Renderer* Graphics::getRenderer() {
    return renderer;
}

SDL_Window* Graphics::getWindow() {
    return window;
}

void Graphics::toggleFullscreen() {
    if (!window) return;
    if (isCurrentlyFullscreen) {
        if (SDL_SetWindowFullscreen(window, SDL_FALSE) != 0) { 
            std::cerr << "图形错误: 切换回窗口模式失败! SDL_Error: " << SDL_GetError() << std::endl;
        } else {
            isCurrentlyFullscreen = false;
            std::cout << "[调试] 已切换到窗口模式。" << std::endl;
        }
    } else {
        if (SDL_SetWindowFullscreen(window, SDL_TRUE) != 0) { 
            std::cerr << "图形错误: 切换到全屏模式失败! SDL_Error: " << SDL_GetError() << std::endl;
        } else {
            isCurrentlyFullscreen = true;
            std::cout << "[调试] 已切换到全屏模式。" << std::endl;
        }
    }
}

void Graphics::fillCircle(int centerX, int centerY, int radius, SDL_Color color) {
    if (!renderer) return;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderPoint(renderer, centerX + dx, centerY + dy);
            }
        }
    }
}

void Graphics::drawCircle(int centerX, int centerY, int radius, SDL_Color color) {
    if (!renderer) return;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int angle_deg = 0; angle_deg < 360; ++angle_deg) {
        double angle_rad = angle_deg * (M_PI / 180.0); 
        int x = centerX + static_cast<int>(round(radius * SDL_cos(angle_rad))); 
        int y = centerY + static_cast<int>(round(radius * SDL_sin(angle_rad)));
        SDL_RenderPoint(renderer, x, y);
    }
}
