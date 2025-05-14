#include "Constants.h" // 包含头文件以确保声明与定义匹配

// --- 提供常量的实际定义 ---

// 棋盘常量
const int BOARD_ROWS = 15;    
const int BOARD_COLS = 15;    
const int CELL_SIZE = 40;     
const int BORDER_PADDING = 20; 
const int DOT_RADIUS = 4;      
const int PIECE_RADIUS = CELL_SIZE / 2 - 4; 

// 窗口尺寸
const int SCREEN_WIDTH = BOARD_COLS * CELL_SIZE + 2 * BORDER_PADDING;
const int SCREEN_HEIGHT = BOARD_ROWS * CELL_SIZE + 2 * BORDER_PADDING;

// 字体常量
const char* FONT_PATH = "msyh.ttc"; // 使用支持中文的字体，微软雅黑
const int FONT_SIZE = 28;           

// 玩家定义
const int EMPTY_PIECE = 0;
const int BLACK_PIECE = 1;
const int WHITE_PIECE = 2;

