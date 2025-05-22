#ifndef ALPHABETAAI_H
#define ALPHABETAAI_H
// Copyright (c) 2025 wibyuan
// Licensed under the MIT License (see LICENSE for details)

#include "Player.h"
#include "Board.h"
#include "Constants.h" 
#include <vector>
#include <array>
#include <string>
#include <algorithm> 
#include <cmath>     
#include <limits>    

const int ABAI_N = 15;           
const int ABAI_B_STATES = 59049; 
const int ABAI_MAX_LINE_LEN_FND = 9; 
const int ABAI_VL_DIM1_SIZE = ABAI_MAX_LINE_LEN_FND + 1; 
const int ABAI_V_WEIGHTS_SIZE = 6; 
const int ABAI_P3_POWERS_SIZE = 15; 
const int ABAI_G_LINES = 4;         
const int ABAI_G_LINE_MAX_LEN = 2 * ABAI_N -1; 


class AlphaBetaAI : public Player {
public:
    AlphaBetaAI(int searchDepthU_default = 5, int branchFactorV_default = 30);
    Point getMove(const Board& board, int playerColor) override;

private:
    // --- 成员变量 ---
    int aiPlayerColor_op; 

    std::array<std::array<int, ABAI_N>, ABAI_N> internal_board_bf; 
    std::array<std::array<std::array<int, 2>, ABAI_B_STATES>, ABAI_VL_DIM1_SIZE> precomputed_line_values_vl; 
    std::array<int, ABAI_MAX_LINE_LEN_FND> fnd_gg_temp; 
    std::array<int, 3> fnd_cx_temp; 
    std::array<int, ABAI_V_WEIGHTS_SIZE> shape_scores_v;  
    std::array<int, ABAI_P3_POWERS_SIZE> p3_powers_p3; 
    std::array<std::array<int, ABAI_G_LINE_MAX_LEN>, ABAI_G_LINES> line_states_g; 
    
    int current_white_total_score;
    int current_black_total_score; 
    int best_r_from_dfs; 
    int best_c_from_dfs; 
    
    std::array<int, ABAI_N * ABAI_N> candidate_scores_ww; 
    
    int current_search_depth_U;
    int current_branch_factor_V;
    int default_search_depth_U;
    int default_branch_factor_V;

    // --- 私有方法 ---
    bool isOk(int r, int c) const;
    void precomputeValues(int current_len_n, int state_A, int white_score_W, int black_score_B); 
    int calculateBoardScore(); 
    void updateScoreContributionForLines(int r, int c, int weight_w); 
    void updateAIInternalState(int r, int c, int piece_o); 
    int alphaBetaSearch(int depth_n, int alpha_al, int beta_bt, int player_to_move_Op_dfs); 
    void initializeAIStateFromBoard(const Board& externalBoard); 
};

#endif // ALPHABETAAI_H
