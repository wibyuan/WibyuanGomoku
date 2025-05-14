#include "GreedyAI.h"
#include <algorithm> // 用于 std::fill, std::max
#include <limits>    // 用于 std::numeric_limits
#include <cmath>     // 用于 std::abs
#include <iostream>  // 用于调试输出
#include <vector>    // 确保包含，虽然主要用 std::array

// 构造函数: 初始化权重等
GreedyAI::GreedyAI() : 
    k1_factor(3), 
    k2_factor(3),
    currentTotalBoardScore(0) 
{
    std::cout << "[调试] GreedyAI 实例已创建。" << std::endl; // 修改调试输出为中文
    v_weights[0] = 0; 
    v_weights[1] = 1; 
    for (int i = 2; i < 5; ++i) {
        v_weights[i] = v_weights[i - 1] * k1_factor;
    }
    v_weights[5] = 10000000; // 五子连珠的极高分
}

// 辅助函数：检查坐标是否在棋盘内
bool GreedyAI::isOk(int r, int c) const {
    return r >= 0 && r < GAI_N && c >= 0 && c < GAI_N;
}

// 核心评估函数
// 计算在点 (r_center, c_center) 落子或模拟落子后，该点对棋盘局势的得分贡献。
// 这个函数会检查以 (r_center, c_center) 为中心的四个方向（水平、垂直、两条对角线）上长度为5的线段。
int GreedyAI::calculateScoreContribution(int r_center, int c_center) {
    int score_contribution_for_point = 0; // 该点的得分贡献累加值
    const int directions = 4; // 四个方向：水平，垂直，主对角线，副对角线
    const int dr_gdy[] = {1, 1, 1, 0};  // 行方向向量 (对应垂直、主对角线、副对角线、水平) 
    const int dc_gdy[] = {-1, 0, 1, 1}; // 列方向向量 (对应副对角线、垂直、主对角线、水平) 

    for (int z = 0; z < directions; ++z) { // 遍历四个方向
        current_segment_len = 0;        // 当前评估线段的实际长度
        piece_counts_in_segment.fill(0); // 清空当前线段中各棋子类型的计数 (0:空, 1:黑, 2:白)

        int r_scan_start = r_center; // 扫描线段的起始行
        int c_scan_start = c_center; // 扫描线段的起始列
        int e1_offset_from_center = 0; // 当前扫描点相对于中心点的偏移量 (用于确保线段长度不超过5)

        // 从中心点向一个方向回溯最多4格，找到5格线段的起始点
        for(int k=0; k<4; ++k) { 
            int prev_r = r_scan_start - dr_gdy[z];
            int prev_c = c_scan_start - dc_gdy[z];
            if (!isOk(prev_r, prev_c) || e1_offset_from_center <= -4) break; // 超出棋盘或已回溯足够远
            r_scan_start = prev_r;
            c_scan_start = prev_c;
            e1_offset_from_center--;
        }
        
        // 从找到的起始点开始，向前扫描最多9格 (GAI_E)，形成评估线段
        int r_current = r_scan_start;
        int c_current = c_scan_start;
        while (isOk(r_current, c_current) && e1_offset_from_center <= 4 && current_segment_len < GAI_E) {
            current_line_segment[current_segment_len++] = internalBoard[r_current][c_current]; // 记录线段上的棋子
            r_current += dr_gdy[z];
            c_current += dc_gdy[z];
            e1_offset_from_center++; 
        }

        if (current_segment_len < 5) continue; // 如果线段长度不足5，则无法形成五子，跳过此方向

        // 初始化滑动窗口（长度为5）的前4个棋子的计数
        for (int i = 0; i < 4; ++i) { 
            if (current_line_segment[i] >=0 && current_line_segment[i] < static_cast<int>(piece_counts_in_segment.size())) {
                piece_counts_in_segment[current_line_segment[i]]++; 
            }
        }
        
        // 滑动窗口，评估每个长度为5的子线段
        for (int i = 4; i < current_segment_len; ++i) {
            // 将新进入窗口的棋子加入计数
            if (current_line_segment[i] >=0 && current_line_segment[i] < static_cast<int>(piece_counts_in_segment.size())) {
                piece_counts_in_segment[current_line_segment[i]]++; 
            }
            
            // 根据当前5格窗口内的棋子情况计算得分
            if (piece_counts_in_segment[aiPlayerColor] == 0) { // 如果AI的棋子数为0 (即没有我方棋子)
                // 那么评估对手棋形的分数 (负贡献)
                if (piece_counts_in_segment[opponentColor] > 0 && piece_counts_in_segment[opponentColor] < static_cast<int>(v_weights.size())) {
                     score_contribution_for_point -= k2_factor * v_weights[piece_counts_in_segment[opponentColor]]; 
                }
            }
            if (piece_counts_in_segment[opponentColor] == 0) { // 如果对手的棋子数为0 (即没有对方棋子)
                // 那么评估我方棋形的分数 (正贡献)
                 if (piece_counts_in_segment[aiPlayerColor] > 0 && piece_counts_in_segment[aiPlayerColor] < static_cast<int>(v_weights.size())) {
                    score_contribution_for_point += v_weights[piece_counts_in_segment[aiPlayerColor]]; 
                 }
            }
            
            // 将滑出窗口的棋子从计数中移除
            if (i-4 >= 0 && current_line_segment[i-4] >=0 && current_line_segment[i-4] < static_cast<int>(piece_counts_in_segment.size())) {
                piece_counts_in_segment[current_line_segment[i-4]]--; 
            }
        }
    }
    return score_contribution_for_point;
}

// 更新内部棋盘状态并相应地更新 currentTotalBoardScore (棋盘总评估分)
void GreedyAI::updateBoardAndScore(int r, int c, int player) {
    if (!isOk(r,c)) { // 检查坐标有效性
        std::cerr << "[错误] GreedyAI::updateBoardAndScore - 无效坐标: (" << r << "," << c << ")" << std::endl;
        return;
    }
    // 先减去该点在修改前对总分的贡献 (因为该点状态将改变)
    currentTotalBoardScore -= calculateScoreContribution(r, c); 
    internalBoard[r][c] = player; // 在内部棋盘上落子或移除棋子 (player为EMPTY_PIECE时)                             
    // 再加上该点在修改后对总分的贡献
    currentTotalBoardScore += calculateScoreContribution(r, c); 
}


// 初始化 AI 的内部状态 (棋盘和总评估分)
void GreedyAI::initializeAIState(const Board& externalBoard) {
    currentTotalBoardScore = 0; // 重置总评估分
    // 从外部棋盘复制状态到内部棋盘
    for(int r = 0; r < GAI_N; ++r) {
        for(int c = 0; c < GAI_N; ++c) {
            internalBoard[r][c] = externalBoard.getPiece(r, c);
        }
    }
    // 根据复制后的内部棋盘，重新计算初始的总评估分
    for(int r = 0; r < GAI_N; ++r) {
        for(int c = 0; c < GAI_N; ++c) {
            // 注意：这里是对每个点调用calculateScoreContribution，
            // 这与updateBoardAndScore中的用法不同。
            // initializeAIState的目的是基于当前完整棋盘计算一个初始总分。
            currentTotalBoardScore += calculateScoreContribution(r, c);
        }
    }
    std::cout << "[调试] GreedyAI 初始棋盘分数 (currentTotalBoardScore): " << currentTotalBoardScore << std::endl;
}


// 获取 AI 的下一步棋
Point GreedyAI::getMove(const Board& board, int playerColor) {
    aiPlayerColor = playerColor; // 设置AI执棋颜色
    opponentColor = (playerColor == BLACK_PIECE) ? WHITE_PIECE : BLACK_PIECE; // 设置对手颜色

    initializeAIState(board); // 初始化AI的内部棋盘和评估分数
    
    int bestScoreForAI = std::numeric_limits<int>::min(); // AI能获得的最佳分数，初始化为最小值
    Point bestMove = {-1, -1}; // 最佳落子点，初始化为无效值

    // 遍历棋盘所有空位，尝试落子并评估
    for (int r_try = 0; r_try < GAI_N; ++r_try) {
        for (int c_try = 0; c_try < GAI_N; ++c_try) {
            if (internalBoard[r_try][c_try] == EMPTY_PIECE) { // 如果是空位
                
                // 模拟AI在此处落子
                updateBoardAndScore(r_try, c_try, aiPlayerColor);
                int scoreAfterAIMove = currentTotalBoardScore; // 获取AI落子后的棋盘总评估分
                // 撤销模拟落子，恢复棋盘状态和总评估分
                updateBoardAndScore(r_try, c_try, EMPTY_PIECE); 

                bool updateBest = false; // 是否更新最佳走法的标志
                if (bestMove.row == -1) { // 如果还没有找到任何有效走法
                    updateBest = true;
                } else if (scoreAfterAIMove > bestScoreForAI) { // 如果当前模拟走法得分更高
                     updateBest = true;
                } else if (scoreAfterAIMove == bestScoreForAI) { // 如果得分相同
                     // 平局打破规则：选择离棋盘中心更近的走法
                     if (std::abs(r_try - (GAI_N / 2)) + std::abs(c_try - (GAI_N / 2)) < 
                         std::abs(bestMove.row - (GAI_N / 2)) + std::abs(bestMove.col - (GAI_N / 2))) {
                         updateBest = true;
                     }
                }

                if (updateBest) { // 如果需要更新最佳走法
                    bestScoreForAI = scoreAfterAIMove;
                    bestMove.row = r_try;
                    bestMove.col = c_try;
                }
            }
        }
    }
    
    // 如果没有找到任何有效走法（例如棋盘已满或出现意外情况）
    if (bestMove.row == -1) {
         std::cerr << "[AI警告] GreedyAI 未找到有效走法。返回中心点或第一个可用空位。" << std::endl;
         if (board.isValidMove(GAI_N / 2, GAI_N / 2)) { // 尝试中心点
             bestMove.row = GAI_N / 2;
             bestMove.col = GAI_N / 2;
         } else { // 否则，遍历查找第一个可用的空位
             for(int r_fallback = 0; r_fallback < GAI_N; ++r_fallback) {
                 for(int c_fallback = 0; c_fallback < GAI_N; ++c_fallback) {
                     if(board.isValidMove(r_fallback, c_fallback)) {
                         bestMove.row = r_fallback;
                         bestMove.col = c_fallback;
                         goto found_fallback_greedy_final_cpp_impl_v3; // 使用 goto 跳出嵌套循环
                     }
                 }
             }
             found_fallback_greedy_final_cpp_impl_v3:; // goto 标签
         }
    }

    std::cout << "[AI] GreedyAI 选择走法: 行=" << bestMove.row << ", 列=" << bestMove.col << "，棋盘评估分: " << bestScoreForAI << std::endl;
    return bestMove;
}
