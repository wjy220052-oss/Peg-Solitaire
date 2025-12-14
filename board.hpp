#pragma once
#include <vector>

// 游戏模式：传统 / 目标格子 / 保护国王
enum class GameMode {
    Classic,
    Lattice,
    Chess
};

// 地图形状
enum class MapShape {
    Cross,      // 十字
    BigCross,   // 大十字
    Triangle,   // 三角形
    Diamond     // 菱形
};

// 棋子状态
enum class CellState {
    Invalid,    // 无效格（不参与游戏）
    Empty,      // 空格
    Peg         // 有棋子
};

// 格子类型（技能）
enum class CellType {
    Normal,     // 普通格
    Ice,        // 冰格：落上去再沿方向滑一格
    Swamp,      // 沼泽：不能落子
    Barrier,    // 障碍：不能落子，也不能跳过
    Goal,       // 目标格：目标模式用
    King,        // 国王格：保护国王模式用
    Teleport    // 传送格：跳到另一格
};

// 一局游戏的特殊格子配置
struct SpecialConfig {
    bool useIce     = false;
    bool useSwamp   = false;
    bool useBarrier = false;
    bool extraHoles = false;
};

// 只负责规则和数据
class Board {
public:
    static constexpr int Rows = 7;
    static constexpr int Cols = 7;

    Board(GameMode mode = GameMode::Classic,
          MapShape shape = MapShape::Cross,
          SpecialConfig special = {});

    void setMode(GameMode m);
    void setShape(MapShape s);
    void setSpecialConfig(const SpecialConfig& cfg);

    void reset();   // 重新生成棋盘（形状 + 目标格/国王 + 特殊格）

    // 访问
    CellState at(int r, int c) const;
    CellType  typeAt(int r, int c) const;
    void set(int r, int c, CellState state);
    void setType(int r, int c, CellType t);

    bool inBounds(int r, int c) const;  // 在棋盘内且不是 Invalid

    bool canMove(int r, int c) const;   // 该格上的棋子是否有合法跳跃
    std::vector<std::pair<int,int>> getPossibleTargets(int r, int c) const;  // 获取合法跳跃目标

    bool canJump(int r1, int c1, int r2, int c2) const;
    void applyJump(int r1, int c1, int r2, int c2);

    int  countPegs() const;             // 棋盘上棋子的数量
    bool hasMove() const;               // 是否还有任何可行步
    bool isSolved() const;              // 是否只剩一个棋子（传统模式用）

private:
    void initBoardArrays();
    void initShape();       // 根据形状生成基本棋局
    void initCross();
    void initBigCross();
    void initTriangle();
    void initDiamond();

    void initWinCells();    // 按游戏模式设置 Goal / King 等
    void applySpecialTiles();// 按 SpecialConfig 随机布置冰格/沼泽/障碍
    void digRandomHoles();  // 随机挖空格

    GameMode      mode_;
    MapShape      shape_;
    SpecialConfig special_;
    CellState     board_[Rows][Cols];
    CellType      type_[Rows][Cols];
};
