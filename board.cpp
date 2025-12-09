#include "board.hpp" //将Board.hpp头文件包含进来，然后就可以完善头文件内的函数
#include <iostream>
//绘制一个7*7的棋盘，中间三行三列除却中心格均为有子，其余均无子
Board::Board(){
    for (int r = 0; r < Rows;r++){
        for (int c = 0;c < Cols;c++){
            board[r][c] = CellState :: Invalid;
    }
    }
     for (int r = 2;r <= 4;++r){
         for (int c = 0;c < Cols;++c){
             board[r][c] = CellState :: Peg;
         }
     }
     for (int r = 0;r < Rows;++r){
         for (int c = 2;c <= 4;++c){
             board[r][c] = CellState :: Peg;
         }
     }
    board[3][3] = CellState :: Empty;
}
// 完成对每个格子状态的读取以及边界的检测
CellState Board::at(int r, int c) const{
    return board[r][c];
}
void Board::set(int r, int c,CellState state){
    board[r][c] = state;
}
// 判断是否在棋盘内
bool Board::inBounds(int r, int c) const{
    return (!(board[r][c] == CellState :: Invalid)) && (r >= 0 && r < Rows && c >= 0 && c < Cols);
}
// 判断这个子具不具备移动的可能性
bool Board::canMove(int r, int c) const{
    if (!inBounds(r,c)){
        return false;
    }
    if (board[r][c] != CellState :: Peg){
        return false;
    }
    const int dr[] = {2,-2,0,0};
    const int dc[] = {0,0,-2,2};
    for (int i = 0;i < 4;i++){
        int r2 = r + dr[i];
        int c2 = c + dc[i];
        if (canJump(r,c,r2,c2) && inBounds(r2,c2)){
            return true;
        } 
    }
    return false;
}

//判断这个方式能不能跳
bool Board::canJump(int r1, int c1, int r2, int c2) const{
    // 如果不在棋盘内，是无效的
    if (!inBounds(r1,c1) || !inBounds(r2,c2)){
        return false; 
    }
    // 如果起始位置无子或目标位置有子，则无效
    if (board[r1][c1] != CellState :: Peg || board[r2][c2] != CellState :: Empty){
        return false;
    }
    int dr = r2 - r1;
    int dc = c2 - c1;
    bool collinear = (dr == 0 && (dc == 2 || dc == -2)) || (dc == 0 && (dr == 2 || dr == -2));
    if (!collinear){
        return false;
    }
    int r3 = (r1 + r2) / 2;
    int c3 = (c1 + c2) / 2;
    if (board[r3][c3] != CellState :: Peg){
        return false;
    }
    else {
        return true;
    }
}
// 执行跳跃
void Board::applyJump(int r1, int c1, int r2, int c2){
    if (!canJump(r1,c1,r2,c2)){
        return;
    }
    set(r1,c1,CellState::Empty);                          //起始位置没有棋子
    set(r2,c2,CellState::Peg);                              //目标位置有棋子                                        
    set((r1 + r2) / 2,(c1 + c2) / 2,CellState::Empty);    //中间位置的棋子被吃掉
}
// 计算棋子数量
int Board::countPegs() const{
    int count = 0;
    for (int r = 0;r < Rows;r++){
        for (int c = 0;c < Cols;c++){
            if (board[r][c] == CellState :: Peg){
                count++;
            }
        }
    }
    return count;
}
// 判断是否完成游戏(遍历每个棋子)
bool Board::isSolved() const{
    for (int r = 0;r < Rows;r++){
        for (int c = 0;c < Cols;c++){
            if (board[r][c] != CellState :: Peg){
                continue;                               //如果当前位置不是棋子，则跳过
            }
            const int dr[] = {2,-2,0,0};               //定义四个方向(行向)
            const int dc[] = {0,0,-2,2};             //定义四个方向(列向)
            for (int i = 0;i < 4;i++){
                int r2 = r + dr[i];
                int c2 = c + dc[i];
                if (canJump(r,c,r2,c2) && inBounds(r2,c2)){
                    return true;
                }
            }
        }
    }
    return false; 
}