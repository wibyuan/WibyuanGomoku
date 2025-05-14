#ifndef PLAYER_H
#define PLAYER_H

#include "Board.h" // 需要 Board 类定义

// 结构体，用于表示棋盘上的一个点或一步棋
struct Point {
    int row = -1;
    int col = -1;
};

// 玩家基类 (抽象类)
class Player {
public:
    // 虚析构函数，确保派生类析构函数能被正确调用
    virtual ~Player() = default; 

    // 纯虚函数，获取下一步棋
    // 参数: 
    //   board: 当前棋盘状态 (常量引用，AI不应直接修改)
    //   playerColor: AI 自己的棋子颜色 (BLACK_PIECE 或 WHITE_PIECE)
    // 返回值:
    //   Point: AI 计算出的最佳落子位置 (行, 列)
    virtual Point getMove(const Board& board, int playerColor) = 0; 
};

#endif // PLAYER_H
