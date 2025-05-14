#include "Board.h" // 包含对应的头文件
#include <vector> // 确保包含 vector

// 构造函数: 初始化棋盘网格
Board::Board() {
    // 使用 resize 来创建指定大小的二维向量，并初始化为 EMPTY_PIECE (0)
    grid.resize(BOARD_ROWS, std::vector<int>(BOARD_COLS, EMPTY_PIECE));
    // 或者如果已经在头文件中初始化，这里可以为空，但显式初始化更清晰
    // reset(); // 也可以在构造函数中调用 reset
}

// 重置棋盘: 将所有格子设置为空
void Board::reset() {
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            grid[r][c] = EMPTY_PIECE;
        }
    }
}

// 获取指定位置的棋子状态
int Board::getPiece(int row, int col) const {
    // 添加边界检查，防止访问越界
    if (row >= 0 && row < BOARD_ROWS && col >= 0 && col < BOARD_COLS) {
        return grid[row][col];
    }
    return -1; // 返回一个无效值表示越界或错误
}

// 检查指定位置是否可以落子
bool Board::isValidMove(int row, int col) const {
    // 检查是否在棋盘边界内，并且该位置为空
    return row >= 0 && row < BOARD_ROWS && 
           col >= 0 && col < BOARD_COLS && 
           grid[row][col] == EMPTY_PIECE;
}

// 在指定位置放置棋子
bool Board::placePiece(int row, int col, int player) {
    if (isValidMove(row, col)) {
        grid[row][col] = player;
        return true; // 成功放置
    }
    return false; // 位置无效或已有棋子
}

// 检查指定玩家在最后落子 (r, c) 后是否获胜
bool Board::checkWin(int r, int c, int player) const {
    if (player == EMPTY_PIECE) return false; // 空棋不能获胜

    int count;
    // 方向数组: 水平, 垂直, 主对角线(\), 副对角线(/)
    int dr[] = {0, 1, 1, 1}; // row changes
    int dc[] = {1, 0, 1, -1}; // col changes

    for (int i = 0; i < 4; ++i) { // 遍历四个基本方向
        count = 1; // 包含当前落子

        // 检查第一个子方向
        for (int k = 1; k < 5; ++k) {
            int nr = r + dr[i] * k;
            int nc = c + dc[i] * k;
            // 使用 getPiece 进行边界检查并获取棋子
            if (getPiece(nr, nc) == player) { 
                count++;
            } else {
                break;
            }
        }
        // 检查相反的子方向
        for (int k = 1; k < 5; ++k) {
            int nr = r - dr[i] * k; // 注意这里是减去
            int nc = c - dc[i] * k; // 注意这里是减去
            // 使用 getPiece 进行边界检查并获取棋子
            if (getPiece(nr, nc) == player) {
                count++;
            } else {
                break;
            }
        }
        if (count >= 5) {
            return true; // 找到五子连珠
        }
    }
    return false; // 未找到五子连珠
}

// 检查棋盘是否已满 (用于判断平局)
bool Board::isFull() const {
    for (int r = 0; r < BOARD_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (grid[r][c] == EMPTY_PIECE) {
                return false; // 找到空位，棋盘未满
            }
        }
    }
    return true; // 没有找到空位，棋盘已满
}
