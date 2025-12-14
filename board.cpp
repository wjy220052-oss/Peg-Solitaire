// 只做规则和数据
#include "board.hpp"
#include <random>
#include <cmath>

// 小工具：随机整数 [a,b]
static int randomInt(int a, int b) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(a, b);
    return dist(gen);
}

Board::Board(GameMode mode, MapShape shape, SpecialConfig special)
    : mode_(mode), shape_(shape), special_(special)
{
    reset();
}

void Board::setMode(GameMode m) {
    mode_ = m;
    reset();
}

void Board::setShape(MapShape s) {
    shape_ = s;
    reset();
}

void Board::setSpecialConfig(const SpecialConfig& cfg) {
    special_ = cfg;
    reset();
}

void Board::initBoardArrays() {
    for (int r = 0; r < Rows; ++r) {
        for (int c = 0; c < Cols; ++c) {
            board_[r][c] = CellState::Invalid;
            type_[r][c]  = CellType::Normal;
        }
    }
}

void Board::reset() {
    initBoardArrays();
    initShape();             // 按照形状铺满格子
    initWinCells();         // 根据模式设置Goal/King
    if (special_.extraHoles) {
        digRandomHoles();       // 随机挖洞
    }
    applySpecialTiles();   //   根据特殊配置设置特殊格子
}

// ===== 形状 =====

void Board::initCross() {
    // 中间三行全是棋子
    for (int r = 2; r <= 4; ++r) {
        for (int c = 0; c < Cols; ++c) {
            board_[r][c] = CellState::Peg;
        }
    }
    // 中间三列也是棋子
    for (int r = 0; r < Rows; ++r) {
        for (int c = 2; c <= 4; ++c) {
            board_[r][c] = CellState::Peg;
        }
    }
    // 中心空
    board_[Rows/2][Cols/2] = CellState::Empty;
}

void Board::initBigCross() {
    // 比普通十字更粗一点
    for (int r = 1; r <= 5; ++r) {
        for (int c = 0; c < Cols; ++c) {
            board_[r][c] = CellState::Peg;
        }
    }
    for (int r = 0; r < Rows; ++r) {
        for (int c = 1; c <= 5; ++c) {
            board_[r][c] = CellState::Peg;
        }
    }
    board_[Rows/2][Cols/2] = CellState::Empty;
}

void Board::initTriangle() {
    // 中间为底边的等腰三角形
    int mid = Cols / 2;
    for (int r = 0; r < Rows; ++r) {
        int half = r;
        for (int c = mid - half; c <= mid + half; ++c) {
            if (c >= 0 && c < Cols) {
                board_[r][c] = CellState::Peg;
            }
        }
    }
    board_[Rows-1][Cols/2] = CellState::Empty;
}

void Board::initDiamond() {
    int cr = Rows / 2;
    int cc = Cols / 2;
    for (int r = 0; r < Rows; ++r) {
        for (int c = 0; c < Cols; ++c) {
            if (std::abs(r - cr) + std::abs(c - cc) <= 3) {
                board_[r][c] = CellState::Peg;
            }
        }
    }
    board_[cr][cc] = CellState::Empty;
}

void Board::initShape() {
    switch (shape_) {
    case MapShape::Cross:     initCross();     break;
    case MapShape::BigCross:  initBigCross();  break;
    case MapShape::Triangle:  initTriangle();  break;
    case MapShape::Diamond:   initDiamond();   break;
    }
}

// ===== 按规则设置目标格 / 国王 =====

void Board::initWinCells() {
    int cr = Rows / 2;
    int cc = Cols / 2;

    if (mode_ == GameMode::Lattice) {
        // 目标格：一开始必须是空格
        if (board_[cr][cc] != CellState::Invalid) {
            type_[cr][cc] = CellType::Goal;
            board_[cr][cc] = CellState::Empty;
            return;
        }
        // 中心无效，就选第一个有效格
        for (int r = 0; r < Rows; ++r) {
            for (int c = 0; c < Cols; ++c) {
                if (board_[r][c] != CellState::Invalid) {
                    type_[r][c] = CellType::Goal;
                    board_[r][c] = CellState::Empty;
                    return;
                }
            }
        }
    } else if (mode_ == GameMode::Chess) {
        // 国王格：必须有棋子
        if (board_[cr][cc] == CellState::Peg) {
            type_[cr][cc] = CellType::King;
            return;
        }
        for (int r = 0; r < Rows; ++r) {
            for (int c = 0; c < Cols; ++c) {
                if (board_[r][c] == CellState::Peg) {
                    type_[r][c] = CellType::King;
                    return;
                }
            }
        }
    }
}

// ===== 随机布置特殊格子 =====

void Board::applySpecialTiles() {
    // 冰格：可以保留棋子
    if (special_.useIce) {
        int count = randomInt(1, 5);
        for (int i = 0; i < count; ++i) {
            int r = randomInt(0, Rows-1);
            int c = randomInt(0, Cols-1);
            if (board_[r][c] == CellState::Invalid) {
                --i; 
                continue; 
            }
            if (type_[r][c] == CellType::Goal ||
                type_[r][c] == CellType::King) {
                --i; 
                continue; 
            }

            type_[r][c] = CellType::Ice;
        }
    }

    // 沼泽格：没有棋子，不能落子
    if (special_.useSwamp) {
        int count = randomInt(1, 5);
        for (int i = 0; i < count; ++i) {
            int r = randomInt(0, Rows-1);
            int c = randomInt(0, Cols-1);
            if (board_[r][c] == CellState::Invalid) { --i; continue; }
            if (type_[r][c] == CellType::Goal ||
                type_[r][c] == CellType::King)   { --i; continue; }

            type_[r][c] = CellType::Swamp;
            board_[r][c] = CellState::Empty; // 沼泽格起始不放棋子
        }
    }

    // 障碍格：没有棋子，不能落子，也不能被跳过
    if (special_.useBarrier) {
        int count = randomInt(1, 5);
        for (int i = 0; i < count; ++i) {
            int r = randomInt(0, Rows-1);
            int c = randomInt(0, Cols-1);
            if (board_[r][c] == CellState::Invalid) { --i; continue; }
            if (type_[r][c] == CellType::Goal ||
                type_[r][c] == CellType::King)   { --i; continue; }

            type_[r][c] = CellType::Barrier;
            board_[r][c] = CellState::Empty; // 障碍格起始不放棋子
        }
    }
}

void Board::digRandomHoles(){
    int holes = randomInt(1, 5);
    int made = 0;
    int guard = 100;
    while (made < holes && guard-- > 0) {
        int r = randomInt(0, Rows-1);
        int c = randomInt(0, Cols-1);
        if (board_[r][c] == CellState::Peg &&
            type_[r][c] != CellType::Goal &&
            type_[r][c] != CellType::King
        ) {board_[r][c] = CellState::Empty;
            ++made;
        }
    }
}

// ===== 基本访问 =====

CellState Board::at(int r, int c) const {
    return board_[r][c];
}

CellType Board::typeAt(int r, int c) const {
    return type_[r][c];
}

void Board::set(int r, int c, CellState state) {
    board_[r][c] = state;
}

void Board::setType(int r, int c, CellType type) {
    if (r < 0 || r >= Rows || c < 0 || c >= Cols) return;
    type_[r][c] = type;
}
bool Board::inBounds(int r, int c) const {
    if (r < 0 || r >= Rows || c < 0 || c >= Cols) return false;
    if (board_[r][c] == CellState::Invalid)      return false;
    return true;
}

// ===== 走子规则 =====

bool Board::canJump(int r1, int c1, int r2, int c2) const {
    if (!inBounds(r1, c1) || !inBounds(r2, c2)) return false;

    if (board_[r1][c1] != CellState::Peg)   return false;
    if (board_[r2][c2] != CellState::Empty) return false;

    // 不能落在沼泽或障碍上
    if (type_[r2][c2] == CellType::Barrier) return false;

    int dr = r2 - r1;
    int dc = c2 - c1;

    // 只能直线跳两格
    if (!((std::abs(dr) == 2 && dc == 0) ||
          (std::abs(dc) == 2 && dr == 0))) {
        return false;
    }

    int rm = r1 + dr / 2;
    int cm = c1 + dc / 2;

    if (!inBounds(rm, cm)) return false;
    if (board_[rm][cm] != CellState::Peg) return false;

    // 中间不能是障碍
    if (type_[rm][cm] == CellType::Barrier) return false;

    return true;
}

bool Board::canMove(int r, int c) const {
    if (!inBounds(r, c))              return false;
    if (board_[r][c] != CellState::Peg) return false;

    static const int dr[4] = {-2, 2, 0, 0};
    static const int dc[4] = {0, 0, -2, 2};
    for (int k = 0; k < 4; ++k) {
        int r2 = r + dr[k];
        int c2 = c + dc[k];
        if (canJump(r, c, r2, c2)) return true;
    }
    return false;
}

std::vector<std::pair<int,int>> Board::getPossibleTargets(int r, int c) const {
    std::vector<std::pair<int,int>> res;
    if (!inBounds(r, c) || board_[r][c] != CellState::Peg) {
        return res;
    }
    static const int dr[4] = {-2, 2, 0, 0};
    static const int dc[4] = {0, 0, -2, 2};
    for (int k = 0; k < 4; ++k) {
        int r2 = r + dr[k];
        int c2 = c + dc[k];
        if (canJump(r, c, r2, c2)) {
            res.emplace_back(r2, c2);
        }
    }
    return res;
}

void Board::applyJump(int r1, int c1, int r2, int c2) {
    if (!canJump(r1, c1, r2, c2)) return;

    int dr = r2 - r1;
    int dc = c2 - c1;
    int rm = r1 + dr / 2;
    int cm = c1 + dc / 2;

    bool kingMoving = (type_[r1][c1] == CellType::King);
    bool kingCaptured = (type_[rm][cm] == CellType::King);

    board_[r1][c1] = CellState::Empty;
    board_[rm][cm] = CellState::Empty;
    board_[r2][c2] = CellState::Peg;

    if (kingMoving){
        type_[r1][c2] = CellType::Normal;
        type_[r2][c2] = CellType::King;
    }
    if (kingCaptured){
        type_[rm][cm] = CellType::Normal;
    }
}

int Board::countPegs() const {
    int cnt = 0;
    for (int r = 0; r < Rows; ++r) {
        for (int c = 0; c < Cols; ++c) {
            if (board_[r][c] == CellState::Peg) ++cnt;
        }
    }
    return cnt;
}

bool Board::hasMove() const {
    for (int r = 0; r < Rows; ++r) {
        for (int c = 0; c < Cols; ++c) {
            if (canMove(r, c)) return true;
        }
    }
    return false;
}

bool Board::isSolved() const {
    return countPegs() == 1;
}
