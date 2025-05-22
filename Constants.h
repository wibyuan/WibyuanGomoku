#ifndef CONSTANTS_H
#define CONSTANTS_H
// Copyright (c) 2025 wibyuan
// Licensed under the MIT License (see LICENSE for details)

// --- 使用 extern 声明常量 (定义将在 Constants.cpp 中) ---

// 棋盘常量
extern const int BOARD_ROWS;    
extern const int BOARD_COLS;    
extern const int CELL_SIZE;     
extern const int BORDER_PADDING; 
extern const int DOT_RADIUS;      
extern const int PIECE_RADIUS; 

// 窗口尺寸
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

// 字体常量
extern const char* FONT_PATH; 
extern const int FONT_SIZE;           

// 玩家定义
extern const int EMPTY_PIECE;
extern const int BLACK_PIECE;
extern const int WHITE_PIECE;

#endif // CONSTANTS_H
