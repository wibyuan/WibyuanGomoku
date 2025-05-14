#include "AlphaBetaAI.h" // 头文件
#include <numeric>        // 用于 std::iota
#include <algorithm>      // 用于 std::sort, std::max, std::min, std::fill
#include <limits>         // 用于 std::numeric_limits
#include <cmath>          // 用于 std::abs
#include <iostream>       // 用于调试输出
#include <vector>

// 辅助函数，用于在必要时映射棋子值
// 假设 Constants.h 中的 EMPTY_PIECE=0, BLACK_PIECE=1, WHITE_PIECE=2 在数值上对应 0,1,2
// 如果不是，则此函数需要进行映射。
// 这里的设置和我之前的 EMPTY_PIECE BLACK_PIECE 和 WHITE_PIECE 的定义可能略有冲突。
// 主要是由于我使用状态压缩加快搜索速度。
inline int map_to_internal_b_piece(int external_piece_from_board_or_player) {
    if (external_piece_from_board_or_player == EMPTY_PIECE) return 0;
    if (external_piece_from_board_or_player == BLACK_PIECE) return 1; // 黑棋的约定
    if (external_piece_from_board_or_player == WHITE_PIECE) return 2; // 白棋的约定
    // 如果棋盘 (Board) 和玩家 (Player) 正确使用这些常量，则不应发生此情况
    std::cerr << "[错误] map_to_internal_b_piece 中出现未知棋子值: " << external_piece_from_board_or_player << std::endl;
    return 0; // 出错时默认为空
}


AlphaBetaAI::AlphaBetaAI(int searchDepthU_default, int branchFactorV_default) :
    aiPlayerColor_op(EMPTY_PIECE), // 将在 getMove 中设置 (0:空, 1:黑, 2:白)
    current_white_total_score(0),
    current_black_total_score(0),
    best_r_from_dfs(-1),
    best_c_from_dfs(-1),
    current_search_depth_U(searchDepthU_default), // 已初始化，但 getMove 将进行设置
    current_branch_factor_V(branchFactorV_default),// 已初始化，但 getMove 将进行设置
    default_search_depth_U(searchDepthU_default),
    default_branch_factor_V(branchFactorV_default)
    // 数组成员会被默认初始化或在下方的方法中初始化
{
    std::cout << "[调试] 正在初始化 AlphaBetaAI (头文件V2)..." << std::endl;

    // 初始化 p3_powers_p3
    p3_powers_p3[0] = 1;
    for (int i = 1; i < ABAI_P3_POWERS_SIZE; ++i) {
        p3_powers_p3[i] = p3_powers_p3[i - 1] * 3;
    }

    // 初始化 shape_scores_v 
    shape_scores_v[0] = 0;
    shape_scores_v[1] = 1;
    for (int i = 2; i < 5; ++i) { // 直到 v[4]
        shape_scores_v[i] = shape_scores_v[i - 1] * 3;
    }
    if (5 < ABAI_V_WEIGHTS_SIZE) { // 确保 v[5] 在边界内
        shape_scores_v[5] = 10000000; // 1e7 代表五子连珠
    }


    // 初始化 fnd_cx_temp 
    fnd_cx_temp.fill(0);

    // 此操作会填充 precomputed_line_values_vl
    precomputeValues(0, 0, 0, 0); // 参数: current_len_n, state_A, white_score_W, black_score_B
    std::cout << "[调试] AlphaBetaAI 预计算 (fnd 逻辑) 完成。" << std::endl;
}

// 越界判断
bool AlphaBetaAI::isOk(int r, int c) const {
    return r >= 0 && r < ABAI_N && c >= 0 && c < ABAI_N;
}

// 预处理
void AlphaBetaAI::precomputeValues(int current_len_n, int state_A, int white_score_W, int black_score_B) {
    if (current_len_n < ABAI_VL_DIM1_SIZE && state_A < ABAI_B_STATES) {
        precomputed_line_values_vl[current_len_n][state_A][0] = white_score_W; // 白方分数
        precomputed_line_values_vl[current_len_n][state_A][1] = black_score_B; // 黑方分数
    }

    if (current_len_n == ABAI_MAX_LINE_LEN_FND) return; //最大深度

    for (int piece_type_value = 0; piece_type_value < 3; ++piece_type_value) { // 0: 空, 1: 黑 (内部表示), 2: 白 (内部表示)
        if (current_len_n < ABAI_MAX_LINE_LEN_FND) { // fnd_gg_temp 的边界检查
             fnd_gg_temp[current_len_n] = piece_type_value;
        }
        ++fnd_cx_temp[piece_type_value];
        int next_W = white_score_W;
        int next_B = black_score_B;

        if (current_len_n >= 4) { // 5个棋子的窗口 (索引从 n-4 到 n)
            // piece_type_value 1 代表黑棋，2 代表白棋
            if (fnd_cx_temp[1] == 0 && fnd_cx_temp[2] < ABAI_V_WEIGHTS_SIZE) { // 没有黑棋，计算白棋分数
                next_W += shape_scores_v[fnd_cx_temp[2]];
            }
            if (fnd_cx_temp[2] == 0 && fnd_cx_temp[1] < ABAI_V_WEIGHTS_SIZE) { // 没有白棋，计算黑棋分数
                next_B += shape_scores_v[fnd_cx_temp[1]];
            }
            if (current_len_n - 4 < ABAI_MAX_LINE_LEN_FND) { // 边界检查
                --fnd_cx_temp[fnd_gg_temp[current_len_n - 4]];
            }
        }
        precomputeValues(current_len_n + 1, state_A * 3 + piece_type_value, next_W, next_B);
        if (current_len_n >= 4) {
             if (current_len_n - 4 < ABAI_MAX_LINE_LEN_FND) { // 边界检查
                ++fnd_cx_temp[fnd_gg_temp[current_len_n - 4]];
            }
        }
        --fnd_cx_temp[piece_type_value];
    }
}

// aiPlayerColor_op 对于黑棋是1，对于白棋是2
int AlphaBetaAI::calculateBoardScore() {
    if (aiPlayerColor_op == 1) { // AI 是黑棋
        return current_black_total_score - 3 * current_white_total_score;
    } else { // AI 是白棋 (aiPlayerColor_op == 2)
        return current_white_total_score - 3 * current_black_total_score;
    }
    // 如果 aiPlayerColor_op 总是1或2，则不应执行到这里
    // return 0; 
}

void AlphaBetaAI::updateScoreContributionForLines(int r, int c, int weight_w) {
    int L, R, d, e; // L,R为左右边界，d为长度，e为该线的状态编码

    // 水平方向: line_states_g[0][行索引], p3_powers_p3 的索引是列索引
    L = std::max(c - 4, 0); R = std::min(c + 4, ABAI_N - 1); d = R - L + 1;
    if (d > 0 && d < ABAI_VL_DIM1_SIZE && L < ABAI_P3_POWERS_SIZE && d < ABAI_P3_POWERS_SIZE && r < ABAI_G_LINE_MAX_LEN) { // 检查 r 是否在 line_states_g[0] 的有效范围内
        e = line_states_g[0][r] / p3_powers_p3[L] % p3_powers_p3[d];
        if (e >=0 && e < ABAI_B_STATES) {
             current_black_total_score += weight_w * precomputed_line_values_vl[d][e][1];
             current_white_total_score += weight_w * precomputed_line_values_vl[d][e][0];
        }
    }

    // 垂直方向: line_states_g[1][列索引], p3_powers_p3 的索引是行索引
    L = std::max(r - 4, 0); R = std::min(r + 4, ABAI_N - 1); d = R - L + 1;
    if (d > 0 && d < ABAI_VL_DIM1_SIZE && L < ABAI_P3_POWERS_SIZE && d < ABAI_P3_POWERS_SIZE && c < ABAI_G_LINE_MAX_LEN) { // 检查 c 是否在 line_states_g[1] 的有效范围内
        e = line_states_g[1][c] / p3_powers_p3[L] % p3_powers_p3[d];
         if (e >=0 && e < ABAI_B_STATES) {
            current_black_total_score += weight_w * precomputed_line_values_vl[d][e][1];
            current_white_total_score += weight_w * precomputed_line_values_vl[d][e][0];
        }
    }

    // 主对角线方向: line_states_g[2][r+c], p3_powers_p3 的索引是 r (行索引)、
    int diag_idx = r + c;
    L = std::max({r - 4, 0, diag_idx - (ABAI_N - 1)}); R = std::min({r + 4, ABAI_N - 1, diag_idx});
    d = R - L + 1;
    if (d > 0 && d < ABAI_VL_DIM1_SIZE && L < ABAI_P3_POWERS_SIZE && d < ABAI_P3_POWERS_SIZE && diag_idx >=0 && diag_idx < ABAI_G_LINE_MAX_LEN) {
        e = line_states_g[2][diag_idx] / p3_powers_p3[L] % p3_powers_p3[d];
        if (e >=0 && e < ABAI_B_STATES) {
            current_black_total_score += weight_w * precomputed_line_values_vl[d][e][1];
            current_white_total_score += weight_w * precomputed_line_values_vl[d][e][0];
        }
    }

    // 副对角线方向: line_states_g[3][r-c+N-1], p3_powers_p3 的索引是 r (行索引)
    int anti_diag_idx = r - c + (ABAI_N - 1);
    L = std::max({r - 4, 0, r - c}); R = std::min({r + 4, ABAI_N - 1, (ABAI_N - 1) + (r - c)});
    d = R - L + 1;
     if (d > 0 && d < ABAI_VL_DIM1_SIZE && L < ABAI_P3_POWERS_SIZE && d < ABAI_P3_POWERS_SIZE && anti_diag_idx >=0 && anti_diag_idx < ABAI_G_LINE_MAX_LEN) {
        e = line_states_g[3][anti_diag_idx] / p3_powers_p3[L] % p3_powers_p3[d];
        if (e >=0 && e < ABAI_B_STATES) {
            current_black_total_score += weight_w * precomputed_line_values_vl[d][e][1];
            current_white_total_score += weight_w * precomputed_line_values_vl[d][e][0];
        }
    }
}

// piece_o 对于空是0，黑是1，白是2
void AlphaBetaAI::updateAIInternalState(int r, int c, int piece_o) {
    if (!isOk(r,c)) return;

    updateScoreContributionForLines(r, c, -1); // 减去旧分数
    internal_board_bf[r][c] = piece_o;

    int diff;
    // 水平方向 g[0][r], p3 的索引是 c
    if (r < ABAI_N && c < ABAI_P3_POWERS_SIZE) { 
         diff = piece_o - (line_states_g[0][r] / p3_powers_p3[c] % 3);
         line_states_g[0][r] += diff * p3_powers_p3[c];
    }
    // 垂直方向 g[1][c], p3 的索引是 r
    if (c < ABAI_N && r < ABAI_P3_POWERS_SIZE) { 
        diff = piece_o - (line_states_g[1][c] / p3_powers_p3[r] % 3);
        line_states_g[1][c] += diff * p3_powers_p3[r];
    }
    // 主对角线 g[2][r+c], p3 的索引是 r
    int diag_idx = r + c;
    if (diag_idx < ABAI_G_LINE_MAX_LEN && r < ABAI_P3_POWERS_SIZE) {
        diff = piece_o - (line_states_g[2][diag_idx] / p3_powers_p3[r] % 3);
        line_states_g[2][diag_idx] += diff * p3_powers_p3[r];
    }
    // 副对角线 g[3][r-c+N-1], p3 的索引是 r
    int anti_diag_idx = r - c + (ABAI_N - 1);
    if (anti_diag_idx >=0 && anti_diag_idx < ABAI_G_LINE_MAX_LEN && r < ABAI_P3_POWERS_SIZE) {
        diff = piece_o - (line_states_g[3][anti_diag_idx] / p3_powers_p3[r] % 3);
        line_states_g[3][anti_diag_idx] += diff * p3_powers_p3[r];
    }

    updateScoreContributionForLines(r, c, 1); // 加上新分数
}

void AlphaBetaAI::initializeAIStateFromBoard(const Board& externalBoard) {
    current_black_total_score = 0;
    current_white_total_score = 0;

    for(auto& row_bf : internal_board_bf) row_bf.fill(0); // 0 代表空 (内部约定)
    for(auto& line_array : line_states_g) line_array.fill(0);

    // p3_powers_p3 和 shape_scores_v 在构造函数中初始化。
    // precomputeValues 在构造函数中调用。

    for (int r_idx = 0; r_idx < ABAI_N; ++r_idx) {
        for (int c_idx = 0; c_idx < ABAI_N; ++c_idx) {
            int piece_external = externalBoard.getPiece(r_idx, c_idx);
            int piece_internal = map_to_internal_b_piece(piece_external);
            if (piece_internal != 0) { // 如果不是空
                updateAIInternalState(r_idx, c_idx, piece_internal);
            }
        }
    }
    // aiPlayerColor_op 在 getMove 的开始处设置
    std::cout << "[调试] AlphaBetaAI 状态已初始化 (头文件V2)。 AI op=" << aiPlayerColor_op
              << ", 白方总分: " << current_white_total_score << ", 黑方总分: " << current_black_total_score << std::endl;
}


// player_to_move_Op_dfs 对于黑棋是1，白棋是2
int AlphaBetaAI::alphaBetaSearch(int depth_n, int alpha_al, int beta_bt, int player_to_move_Op_dfs) {
    if (depth_n == current_search_depth_U || abs(calculateBoardScore()) >= 1000000) {
        return calculateBoardScore(); // 从 aiPlayerColor_op 的视角进行评估
    }

    // 生成候选走法并进行启发式评分
    for (int r_idx = 0; r_idx < ABAI_N; ++r_idx) {
        for (int c_idx = 0; c_idx < ABAI_N; ++c_idx) {
            if (internal_board_bf[r_idx][c_idx] == 0) { // 如果是空位
                updateAIInternalState(r_idx, c_idx, player_to_move_Op_dfs);
                // 从 player_to_move_Op_dfs (当前轮到下棋的玩家) 的视角评分
                // 这与 calculateBoardScore 不同，后者使用 aiPlayerColor_op (AI本身的颜色)
                int current_eval_for_ww;
                if (player_to_move_Op_dfs == 1) { // 黑棋的回合下这一步
                    current_eval_for_ww = current_black_total_score - 3 * current_white_total_score;
                } else { // 白棋的回合 (player_to_move_Op_dfs == 2)
                    current_eval_for_ww = current_white_total_score - 3 * current_black_total_score;
                }
                candidate_scores_ww[r_idx * ABAI_N + c_idx] = current_eval_for_ww;
                updateAIInternalState(r_idx, c_idx, 0); // 撤销走法
            } else {
                candidate_scores_ww[r_idx * ABAI_N + c_idx] = -1000000000; // 负十亿，极大的值
            }
        }
    }

    std::vector<int> move_indices(ABAI_N * ABAI_N); // 使用 vector 进行排序
    std::iota(move_indices.begin(), move_indices.end(), 0); // 用 0, 1, 2... 填充

    // 根据启发式评分对候选走法排序
    std::sort(move_indices.begin(), move_indices.end(),
        [&](const int& a_idx, const int& b_idx) {
        if (candidate_scores_ww[a_idx] > candidate_scores_ww[b_idx]) return true;
        if (candidate_scores_ww[a_idx] < candidate_scores_ww[b_idx]) return false;
        // 平局打破规则：离中心点(7,7)更近的更好 (ABAI_N/2)
        int r_a = a_idx / ABAI_N, c_a = a_idx % ABAI_N;
        int r_b = b_idx / ABAI_N, c_b = b_idx % ABAI_N;
        // 意味着曼哈顿距离小的更好
        return (std::abs(r_a - ABAI_N/2) + std::abs(c_a - ABAI_N/2)) < (std::abs(r_b - ABAI_N/2) + std::abs(c_b - ABAI_N/2));
    });

    int best_val_for_node_nm = alpha_al; 
    // 如果当前dfs的玩家不是AI本身，则这是一个MIN节点，对于AI来说，初始值为beta
    if (aiPlayerColor_op != player_to_move_Op_dfs) {
        best_val_for_node_nm = beta_bt;
    }

    int moves_explored_e = 0; // 已探索的走法计数
    // 循环到分支因子
    for (int e_loop_idx = 0; e_loop_idx < current_branch_factor_V && e_loop_idx < static_cast<int>(move_indices.size()); ++e_loop_idx) {
        int move_idx = move_indices[e_loop_idx];
        int r = move_idx / ABAI_N;
        int c = move_idx % ABAI_N;

        if (internal_board_bf[r][c] == 0) { // 如果是空位
            updateAIInternalState(r, c, player_to_move_Op_dfs);
            int recursive_score_w = alphaBetaSearch(depth_n + 1, alpha_al, beta_bt, 3 - player_to_move_Op_dfs); // 得到对方玩家

            // else { bt=nm=min(nm,w); }
            if (player_to_move_Op_dfs == aiPlayerColor_op) { // AI 的 MAX 节点
                if (depth_n == 0) { // 根节点
                    bool best_move_is_invalid_or_not_set = !isOk(best_r_from_dfs, best_c_from_dfs) ||
                                                           (isOk(best_r_from_dfs,best_c_from_dfs) && internal_board_bf[best_r_from_dfs][best_c_from_dfs] != 0);

                    if (best_val_for_node_nm < recursive_score_w || best_move_is_invalid_or_not_set) {
                        best_r_from_dfs = r;
                        best_c_from_dfs = c;
                    }
                }
                best_val_for_node_nm = std::max(best_val_for_node_nm, recursive_score_w);
                alpha_al = std::max(alpha_al, best_val_for_node_nm); // 更新alpha值
            } else { // AI 的 MIN 节点 (轮到对手下棋的搜索层)
                best_val_for_node_nm = std::min(best_val_for_node_nm, recursive_score_w);
                beta_bt = std::min(beta_bt, best_val_for_node_nm); // 更新beta值
            }
            updateAIInternalState(r, c, 0); // 撤销走法
            moves_explored_e++;

            if (alpha_al >= beta_bt) { // Alpha-Beta剪枝条件
                break; 
            }
        }
    }
    if (moves_explored_e == 0) { // 如果没有有效的走法被探索（例如，所有分支都被剪枝或没有空位）
        return calculateBoardScore(); // 如果没有走法，当前棋盘分数是最佳猜测。
    }
    return best_val_for_node_nm;
}


Point AlphaBetaAI::getMove(const Board& board, int playerColor) {
    aiPlayerColor_op = map_to_internal_b_piece(playerColor); 

    int num_pieces_on_board = 0;
    for(int r=0; r<ABAI_N; ++r) for(int c=0; c<ABAI_N; ++c)
        if(map_to_internal_b_piece(board.getPiece(r,c)) != 0) num_pieces_on_board++;

    best_r_from_dfs = -1; 
    best_c_from_dfs = -1;

    bool is_ai_black_first_move = (aiPlayerColor_op == 1 && num_pieces_on_board == 0);

    if (is_ai_black_first_move) { 
        current_search_depth_U = 6;
        current_branch_factor_V = 13;
        best_r_from_dfs = ABAI_N / 2; 
        best_c_from_dfs = ABAI_N / 2; 
        std::cout << "[AI] AlphaBetaAI (头文件V2) 黑棋开局于中心。U="
                  << current_search_depth_U << ", V=" << current_branch_factor_V << std::endl;
    } else {
        initializeAIStateFromBoard(board); 

        if (aiPlayerColor_op == 1) { // AI是黑棋 (但不是第一步)
            current_search_depth_U = default_search_depth_U; 
            current_branch_factor_V = 25; // 黑棋的默认分支因子
        } else { // AI是白棋 (aiPlayerColor_op == 2)
            current_search_depth_U = default_search_depth_U; 
            int original_V_for_white = 30; // 白棋的默认分支因子

            if (num_pieces_on_board == 1) { // 这是AI白棋的 *第一手* 棋
                current_branch_factor_V = 17; //  <--- 为白棋第一手减少分支因子
                std::cout << "[AI 调试] AI 白方第一手棋，临时降低分支因子 V 至: " << current_branch_factor_V << std::endl;
            } else {
                current_branch_factor_V = original_V_for_white; // 白棋后续走法的正常V值
            }
        }
        
        std::cout << "[AI 调试] 中/后期游戏 (头文件V2)。 AI op=" << aiPlayerColor_op
                  << ", U=" << current_search_depth_U << ", V=" << current_branch_factor_V << std::endl;
        alphaBetaSearch(0, -1000000000, 1000000000, aiPlayerColor_op); 
    }

    Point bestMovePoint;
    if (best_r_from_dfs != -1 && best_c_from_dfs != -1 &&
        isOk(best_r_from_dfs, best_c_from_dfs) &&
        map_to_internal_b_piece(board.getPiece(best_r_from_dfs, best_c_from_dfs)) == 0 ) { 
        bestMovePoint.row = best_r_from_dfs;
        bestMovePoint.col = best_c_from_dfs;
    } else { 
        std::cerr << "[AI 警告] AlphaBetaAI getMove (头文件V2) 未找到有效的最佳走法。执行后备方案。" << std::endl;
        bool found_fallback = false;
        for (int r_fallback = 0; r_fallback < ABAI_N; ++r_fallback) {
            for (int c_fallback = 0; c_fallback < ABAI_N; ++c_fallback) {
                if (map_to_internal_b_piece(board.getPiece(r_fallback, c_fallback)) == 0) {
                    bestMovePoint.row = r_fallback;
                    bestMovePoint.col = c_fallback;
                    found_fallback = true;
                    goto fallback_move_found_code_b_strict_v2; 
                }
            }
        }
        fallback_move_found_code_b_strict_v2:; 
        if (!found_fallback) { 
            std::cerr << "[AI 错误] AlphaBetaAI getMove (头文件V2): 棋盘上未找到有效走法！" << std::endl;
            bestMovePoint.row = -1; bestMovePoint.col = -1; 
        }
    }

    std::cout << "[AI] AlphaBetaAI (头文件V2) 最终决策: 行=" << bestMovePoint.row << ", 列=" << bestMovePoint.col << std::endl;
    return bestMovePoint;
}
