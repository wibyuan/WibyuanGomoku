#ifndef GREEDYAI_H
#define GREEDYAI_H

#include "Player.h"    // 继承自 Player 基类
#include "Board.h"     // AI 需要与 Board 对象交互
#include "Constants.h" // 包含棋盘大小、棋子颜色等常量
#include <vector>      // 虽然我们主要用 std::array，但包含 std::vector 无害
#include <array>      // 用于固定大小的数组，如棋盘
#include <string>     // 字符串处理
#include <algorithm>  // 例如 std::fill
#include <cmath>      // 例如 std::abs

const int GAI_N = 15; // 棋盘维度
const int GAI_E = 9;  // 评估线段的最大长度

class GreedyAI : public Player {
public:
    // 构造函数：初始化权重等内部状态
    GreedyAI();

    // 实现基类 Player 的 getMove 方法，这是 AI 的主要接口
    // board: 当前棋盘状态，只读
    // playerColor: AI 当前执棋的颜色 (BLACK_PIECE 或 WHITE_PIECE)
    // 返回值: AI 计算出的最佳落子点 {row, col}
    Point getMove(const Board& board, int playerColor) override;

private:
    // --- 成员变量 ---
    // AI 自己的棋子颜色
    int aiPlayerColor; 
    // 对手的棋子颜色 
    int opponentColor; 

    // AI 内部维护的棋盘状态 
    std::array<std::array<int, GAI_N>, GAI_N> internalBoard; 

    // 评估棋形的权重数组
    std::array<int, 6> v_weights; 
    
    // 评估参数 
    int k1_factor; // 用于计算 v_weights
    int k2_factor; // 用于计算对手棋形的负面影响
    
    // 评估一条线段时使用的临时数组
    std::array<int, GAI_E> current_line_segment; // 当前评估线段
    std::array<int, 3> piece_counts_in_segment;  //  (0:空, 1:黑, 2:白)
    int current_segment_len;                     // 当前评估线段的实际长度

    // 当前棋盘的总评估分数
    // 这个分数会在每次模拟落子或撤销时更新
    int currentTotalBoardScore;

    // --- 私有方法 ---

    // 辅助函数：检查坐标 (r, c) 是否在棋盘内
    bool isOk(int r, int c) const;

    // 核心评估函数 
    // 计算点 (r, c) 上的棋子（或空位）对其所在四条线段的得分贡献。
    // 这个函数的结果会用来更新 currentTotalBoardScore。
    int calculateScoreContribution(int r_center, int c_center);

    // 更新内部棋盘状态并在 (r, c) 处放置棋子 player，
    // 同时更新 currentTotalBoardScore
    void updateBoardAndScore(int r, int c, int player);

    // 初始化 AI 的内部状态，包括 internalBoard 和 currentTotalBoardScore
    void initializeAIState(const Board& externalBoard);
};

#endif // GREEDYAI_H
