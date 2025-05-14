#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include "Constants.h" // 包含常量定义

class Board {
public:
    // 构造函数
    Board();

    // 重置棋盘
    void reset();

    // 获取指定位置的棋子状态
    int getPiece(int row, int col) const;

    // 检查指定位置是否可以落子
    bool isValidMove(int row, int col) const;

    // 在指定位置放置棋子
    // 返回值: 如果成功放置则为 true, 否则为 false (例如位置无效或已有棋子)
    bool placePiece(int row, int col, int player);

    // 检查指定玩家在最后落子 (r, c) 后是否获胜
    bool checkWin(int r, int c, int player) const;

    // 检查棋盘是否已满 (用于判断平局)
    bool isFull() const;

private:
    // 存储棋盘状态的二维向量
    std::vector<std::vector<int>> grid; 
};

#endif // BOARD_H
