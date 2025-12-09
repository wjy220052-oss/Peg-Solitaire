#pragma once
// #ifndef BOARD_HPP
// #define BOARD_HPP
// #endif
//棋子的状态
enum class CellState{
    Invalid,   //无效格子，不参与游戏
    Empty,     //空格子，可以放置棋子
    Peg        //有棋子

};
// 之后的主函数只需通过内部函数即可操作棋盘，当我们添加新的玩法时只需在此处修改
class Board{
    public:
        static constexpr int Rows = 7;  //比const更稳定的常量
        static constexpr int Cols = 7;
        Board();
        CellState at(int r,int c) const;    //读取每个格子的状态
        void set(int r,int c,CellState state); //写格子
        bool inBounds(int r,int c) const;   //判断是否在边界内
        bool canMove(int r,int c) const;    //判断是否可能移动
        bool canJump(int r1, int c1, int r2, int c2) const; //判断是否可以从（r1,c1）跳到（r2,c2）
        void applyJump(int r1, int c1, int r2, int c2); //执行跳跃，前提是canJump为true
        int countPegs() const; //计算棋子数量
        bool isSolved() const; //判断是否完成游戏
    private:
        // 真正存棋子状态的数据：7*7的二维数组
        CellState board[Rows][Cols];  
};