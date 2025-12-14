// 专心交互和渲染
#include "board.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <limits>
#include <windows.h>
#include <utility>

// 游戏状态
enum class GameState {
    Over,
    Playing
};

// 多层模式
enum class LayerMode {
    Single,
    Double,
    Triple
};

// 一局游戏配置：层数 + 特殊格 + 规则 + 地图形状
struct GameConfig {
    int      layers     = 1;
    bool     useIce     = false;
    bool     useSwamp   = false;
    bool     useBarrier = false;
    GameMode winMode    = GameMode::Classic;
    MapShape mapShape   = MapShape::Cross;
};

struct GameRuntime {
    // 多层棋盘
    std::vector<Board>              floors;
    std::vector<std::vector<Board>> histories;
    int                             currentFloor = 0;

    // 选择状态
    bool selection   = false;
    int  selectedRow = -1;
    int  selectedCol = -1;
    std::vector<std::pair<int,int>> possibleTargets;

    float     cellSize  = 64.f;
    int       moveCount = 0;
    int       pegCount  = 0;
    LayerMode layerMode = LayerMode::Single;
    GameMode  gameMode  = GameMode::Classic;
    GameConfig config;

    // 跳跃动画
    bool         isAnimating   = false;
    float        animTime      = 0.f;
    float        animDuration  = 0.25f;
    sf::Vector2f animStart;
    sf::Vector2f animEnd;
    int          animFromRow   = -1;
    int          animFromCol   = -1;
    int          animToRow     = -1;
    int          animToCol     = -1;

    // 冰格滑行动画
    bool         isIceAnimating = false;
    float        iceTime        = 0.f;
    float        iceDuration    = 0.15f;
    sf::Vector2f iceStart;
    sf::Vector2f iceEnd;
    int          iceToRow       = -1;
    int          iceToCol       = -1;

    // 传送格滑行动画
    bool         isTeleportAnimating = false;
    float        teleportTime        = 0.f;
    float        teleportDuration    = 0.3f;
    int          teleportRow         = -1;
    int          teleportCol         = -1;

    bool      isGameOver = false;
    GameState& gameState;

    GameRuntime(GameState& gs) : gameState(gs) {}
};
void applyTeleportTiles(GameRuntime& rt);

// ===== 控制台交互：从用户获取配置 =====

GameMode getModeFromText() {
    std::cout << "请选择获胜规则(1-传统,2-目标格子,3-保护国王): ";
    char ch;
    std::cin >> ch;
    switch (ch) {
    case '1': return GameMode::Classic;
    case '2': return GameMode::Lattice;
    case '3': return GameMode::Chess;
    default:
        std::cerr << "无效输入，默认传统规则。\n";
        return GameMode::Classic;
    }
}

LayerMode getLayerModeFromText() {
    std::cout << "请选择层数模式(1-单层,2-双层,3-三层): ";
    char ch;
    std::cin >> ch;
    switch (ch) {
    case '1': return LayerMode::Single;
    case '2': return LayerMode::Double;
    case '3': return LayerMode::Triple;
    default:
        std::cerr << "无效输入，默认单层。\n";
        return LayerMode::Single;
    }
}

MapShape getMapModeFromText() {
    std::cout << "请选择地图形状(1-十字, 2-大十字,3-三角形, 4-菱形): ";
    char ch;
    std::cin >> ch;
    switch (ch) {
    case '1': return MapShape::Cross;
    case '2': return MapShape::BigCross;
    case '3': return MapShape::Triangle;
    case '4': return MapShape::Diamond;
    default:
        std::cerr << "无效输入，默认十字形。\n";
        return MapShape::Cross;
    }
}

GameConfig askConfigFromConsole() {
    GameConfig cfg;
    cfg.winMode  = getModeFromText();
    LayerMode lm = getLayerModeFromText();
    cfg.mapShape = getMapModeFromText();

    switch (lm) {
    case LayerMode::Single: cfg.layers = 1; break;
    case LayerMode::Double: cfg.layers = 2; break;
    case LayerMode::Triple: cfg.layers = 3; break;
    }

    char ch;
    std::cout << "是否启用冰格?(y/n): ";
    std::cin >> ch;
    cfg.useIce = (ch == 'y' || ch == 'Y');

    std::cout << "是否启用沼泽格?(y/n): ";
    std::cin >> ch;
    cfg.useSwamp = (ch == 'y' || ch == 'Y');

    std::cout << "是否启用障碍格?(y/n): ";
    std::cin >> ch;
    cfg.useBarrier = (ch == 'y' || ch == 'Y');

    return cfg;
}

// ===== 一些规则判断辅助函数 =====

// 单层：是否“只剩一个棋子且在 Goal 格子上”
bool isGoalWin(const Board& board) {
    int pegCount = board.countPegs();
    if (pegCount != 1) return false;

    for (int r = 0; r < Board::Rows; ++r) {
        for (int c = 0; c < Board::Cols; ++c) {
            if (board.at(r, c) == CellState::Peg) {
                return (board.typeAt(r, c) == CellType::Goal);
            }
        }
    }
    return false;
}

// 单层：国王是否仍然存活（有棋子在 King 格上）
bool isKingAlive(const Board& board) {
    for (int r = 0; r < Board::Rows; ++r) {
        for (int c = 0; c < Board::Cols; ++c) {
            if (board.typeAt(r, c) == CellType::King &&
                board.at(r, c) == CellState::Peg) {
                return true;
            }
        }
    }
    return false;
}

// 多层：当前棋盘 & 历史
Board& currentBoard(GameRuntime& rt) {
    return rt.floors[rt.currentFloor];
}
const Board& currentBoard(const GameRuntime& rt) {
    return rt.floors[rt.currentFloor];
}

// 多层：总棋子数
int totalPegs(const GameRuntime& rt) {
    int sum = 0;
    for (const auto& b : rt.floors) {
        sum += b.countPegs();
    }
    return sum;
}

// 多层：是否还有任意可行步
bool anyMove(const GameRuntime& rt) {
    for (const auto& b : rt.floors) {
        if (b.hasMove()) return true;
    }
    return false;
}

// 多层：目标格胜利（全局只有一个棋子且在某个 Goal 上）
bool globalGoalWin(const GameRuntime& rt) {
    if (totalPegs(rt) != 1) return false;
    for (const auto& b : rt.floors) {
        if (isGoalWin(b)) return true;
    }
    return false;
}

// 多层：是否存在至少一个活着的国王
bool anyKingAlive(const GameRuntime& rt) {
    for (const auto& b : rt.floors) {
        if (isKingAlive(b)) return true;
    }
    return false;
}

// ===== 用配置初始化整局游戏（多层） =====

void initGame(GameRuntime& rt, const GameConfig& cfg) {
    rt.config   = cfg;
    rt.gameMode = cfg.winMode;

    switch (cfg.layers) {
    case 1: rt.layerMode = LayerMode::Single; break;
    case 2: rt.layerMode = LayerMode::Double; break;
    default: rt.layerMode = LayerMode::Triple; break;
    }

    SpecialConfig sc;
    sc.useIce     = cfg.useIce;
    sc.useSwamp   = cfg.useSwamp;
    sc.useBarrier = cfg.useBarrier;
    sc.extraHoles = (cfg.layers > 1);

    rt.floors.clear();

    for (int i = 0; i < cfg.layers; ++i) {
        Board b(cfg.winMode, cfg.mapShape, sc);
        rt.floors.push_back(b);
    }
    applyTeleportTiles(rt);

    rt.histories.clear();
    rt.histories.push_back(rt.floors);

    rt.currentFloor = 0;

    rt.selection   = false;
    rt.selectedRow = -1;
    rt.selectedCol = -1;
    rt.possibleTargets.clear();

    rt.moveCount = 0;
    rt.pegCount  = totalPegs(rt);

    rt.isAnimating     = false;
    rt.animTime        = 0.f;
    rt.isIceAnimating  = false;
    rt.iceTime         = 0.f;
    rt.iceToRow        = -1;
    rt.iceToCol        = -1;
    rt.animFromRow     = rt.animFromCol = -1;
    rt.animToRow       = rt.animToCol = -1;

    rt.isGameOver = false;
}
// 固定传送点并设置为传送格
const std::vector<std::pair<int,int>> TELEPORT_POINTS = {
    {1,3},{3,1},{3,5},{5,3}
};
void applyTeleportTiles(GameRuntime& rt) {
    if (rt.config.layers == 1) return;
    for (auto pos: TELEPORT_POINTS) {
        for (auto& b: rt.floors) {
            int r = pos.first;
            int c = pos.second;
            if (!b.inBounds(r, c)) continue;

            CellType t = b.typeAt(r, c);
            if (t == CellType::Goal || t == CellType::King) {
                continue;
            }
            b.setType(r,c, CellType::Teleport);
            if (b.at(r, c) == CellState::Peg) {
                b.set(r, c, CellState::Empty);
            }
        }
    }
}
// ===== 游戏中处理点击 / 按键 =====

void handlePlaying(const sf::Event& event,
                   GameState& gameState,
                   GameRuntime& rt)
{
    // 动画中或已结束：不接受操作
    if (rt.isAnimating || rt.isIceAnimating || rt.isTeleportAnimating || rt.isGameOver) {
        return;
    }

    Board& board = currentBoard(rt);


    // ESC：退出（这里先简单设为结束）
    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::Escape) {
        rt.selection = false;
        rt.possibleTargets.clear();
        gameState = GameState::Over;
        return;
    }

    // Q/E：切换楼层
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Q) {
            if (rt.currentFloor > 0) {
                rt.currentFloor--;
                rt.selection = false;
                rt.possibleTargets.clear();
            }
            return;
        }
        if (event.key.code == sf::Keyboard::E) {
            if (rt.currentFloor + 1 < static_cast<int>(rt.floors.size())) {
                rt.currentFloor++;
                rt.selection = false;
                rt.possibleTargets.clear();
            }
            return;
        }
    }

    // 撤销（Z）——当前楼层
    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::Z) {
        if (rt.histories.size() > 1) {
            rt.floors = rt.histories.back();
            rt.histories.pop_back();
        }
        rt.selection = false;
        rt.possibleTargets.clear();
        return;
    }

    // 重开（R）——整局重新根据配置生成
    if (event.type == sf::Event::KeyPressed &&
        event.key.code == sf::Keyboard::R) {
        initGame(rt, rt.config);
        return;
    }

    // 鼠标左键：选子 / 走子
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        int mx = event.mouseButton.x;
        int my = event.mouseButton.y;

        int col = static_cast<int>(mx / rt.cellSize);
        int row = static_cast<int>(my / rt.cellSize);

        if (!board.inBounds(row, col)) {
            rt.selection = false;
            rt.possibleTargets.clear();
            return;
        }

        // 未选中棋子：尝试选择
        if (!rt.selection) {
            if (board.at(row, col) == CellState::Peg &&
                board.canMove(row, col) &&
                board.typeAt(row,col) != CellType::Swamp) {
                rt.selection   = true;
                rt.selectedRow = row;
                rt.selectedCol = col;
                rt.possibleTargets = board.getPossibleTargets(row, col);
            } else {
                rt.selection = false;
                rt.possibleTargets.clear();
            }
            return;
        }
        // 已选中棋子：尝试跳跃
        else {
            int fr = rt.selectedRow;
            int fc = rt.selectedCol;

            if (board.canJump(fr, fc, row, col)) {
                // 加入历史栈
                rt.histories.push_back(rt.floors);

                int jumpRow  = row;
                int jumpCol  = col;
                int finalRow = jumpRow;
                int finalCol = jumpCol;

                // 真正执行跳跃
                board.applyJump(fr, fc, jumpRow, jumpCol);

                // 冰格逻辑：如果落在 Ice 上，且前方一格为空 & 不是沼泽/障碍，则滑一步
                bool hasIceSlide = false;
                if (board.typeAt(jumpRow, jumpCol) == CellType::Ice) {
                    int dr = jumpRow - fr;
                    int dc = jumpCol - fc;
                    int stepR = (dr == 0 ? 0 : dr / std::abs(dr));
                    int stepC = (dc == 0 ? 0 : dc / std::abs(dc));

                    int slideRow = jumpRow + stepR;
                    int slideCol = jumpCol + stepC;

                    if (board.inBounds(slideRow, slideCol) &&
                        board.at(slideRow, slideCol) == CellState::Empty &&
                        board.typeAt(slideRow, slideCol) != CellType::Barrier)
                    {
                        bool kingSliding = (board.typeAt(jumpRow, jumpCol) == CellType::King);
                        
                        board.set(jumpRow, jumpCol, CellState::Empty);
                        board.set(slideRow, slideCol, CellState::Peg);

                        if (kingSliding) {
                            board.setType(jumpRow, jumpCol, CellType::Normal);
                            board.setType(slideRow, slideCol, CellType::King);
                        }
                        finalRow = slideRow;
                        finalCol = slideCol;
                        hasIceSlide = true;
                    }
                }
                // 传送格逻辑：如果落在 Teleport 上，则传送到对应位置
                int srcFloor = rt.currentFloor;
                int teleRow = finalRow;
                int teleCol = finalCol;

                if (rt.config.layers > 1 &&
                    board.typeAt(teleRow, teleCol) == CellType::Teleport &&
                    srcFloor + 1 < static_cast<int>(rt.floors.size()))
                {   
                    int dstFloor = (srcFloor + 1) % (int)rt.floors.size();
                    Board& dstBoard = rt.floors[dstFloor];

                    if (dstBoard.inBounds(teleRow, teleCol) &&
                        dstBoard.at(teleRow, teleCol) == CellState::Empty) 
                    {
                        bool kingTeleporting = (board.typeAt(teleRow, teleCol) == CellType::King);

                        board.set(teleRow, teleCol, CellState::Empty);
                        if (kingTeleporting) {
                            board.setType(teleRow, teleCol, CellType::Normal);
                        }

                        dstBoard.set(teleRow, teleCol, CellState::Peg);

                        if (kingTeleporting) {
                            dstBoard.setType(teleRow, teleCol, CellType::King);
                        }

                        rt.currentFloor = dstFloor;

                        rt.isTeleportAnimating = true;
                        rt.teleportTime = 0.f;
                        rt.teleportRow = teleRow;
                        rt.teleportCol = teleCol;
                    }
                }
                rt.moveCount++;
                rt.pegCount = totalPegs(rt);

                // 第一段跳跃动画
                float pr = rt.cellSize * 0.35f;
                rt.isAnimating = true;
                rt.animTime    = 0.f;
                rt.animFromRow = fr;
                rt.animFromCol = fc;
                rt.animToRow   = jumpRow;
                rt.animToCol   = jumpCol;
                rt.animStart = {
                    fc * rt.cellSize + rt.cellSize * 0.5f - pr,
                    fr * rt.cellSize + rt.cellSize * 0.5f - pr
                };
                rt.animEnd = {
                    jumpCol * rt.cellSize + rt.cellSize * 0.5f - pr,
                    jumpRow * rt.cellSize + rt.cellSize * 0.5f - pr
                };

                // 第二段冰滑动画
                if (hasIceSlide) {
                    rt.isIceAnimating = true;
                    rt.iceTime        = 0.f;
                    rt.iceToRow       = finalRow;
                    rt.iceToCol       = finalCol;
                    rt.iceStart = {
                        jumpCol * rt.cellSize + rt.cellSize * 0.5f,
                        jumpRow * rt.cellSize + rt.cellSize * 0.5f
                    };
                    rt.iceEnd = {
                        finalCol * rt.cellSize + rt.cellSize * 0.5f,
                        finalRow * rt.cellSize + rt.cellSize * 0.5f
                    };
                } else {
                    rt.isIceAnimating = false;
                    rt.iceToRow = rt.iceToCol = -1;
                }

                // 清除高亮
                rt.selection = false;
                rt.possibleTargets.clear();
            } else {
                // 点击非法落点 → 取消选中
                rt.selection = false;
                rt.possibleTargets.clear();
            }

            return;
        }
    }
}

// ===== 绘制函数 =====

void drawGame(sf::RenderWindow& window,
              GameRuntime& rt)
{
    window.clear(sf::Color::Black);

    const Board& board = currentBoard(rt);

    sf::RectangleShape cell({rt.cellSize - 2.f, rt.cellSize - 2.f});
    cell.setOutlineThickness(2.f);
    cell.setOutlineColor(sf::Color::Black);
    cell.setFillColor(sf::Color(60,60,60));

    float pegRadius = rt.cellSize * 0.35f;

    sf::CircleShape peg(pegRadius);
    peg.setOutlineThickness(2.f);
    peg.setOutlineColor(sf::Color::Black);
    peg.setFillColor(sf::Color(220,220,50));
    peg.setOrigin(pegRadius, pegRadius);

    sf::CircleShape animPeg(pegRadius);
    animPeg.setOutlineThickness(2.f);
    animPeg.setOutlineColor(sf::Color::Black);
    animPeg.setFillColor(sf::Color(220,220,50));
    animPeg.setOrigin(pegRadius, pegRadius);

    // 国王棋子：用方块画
    sf::RectangleShape kingPiece;
    float kingSize = rt.cellSize * 0.6f;
    kingPiece.setSize({kingSize, kingSize});
    kingPiece.setOrigin(kingSize / 2.f, kingSize / 2.f);
    kingPiece.setFillColor(sf::Color(255, 215, 0)); // 金黄
    kingPiece.setOutlineThickness(2.f);
    kingPiece.setOutlineColor(sf::Color::Red);

    for (int r = 0; r < Board::Rows; ++r) {
        for (int c = 0; c < Board::Cols; ++c) {
            CellState state = board.at(r, c);
            CellType  type  = board.typeAt(r, c);

            if (state == CellState::Invalid) continue;

            float x = c * rt.cellSize + 1.f;
            float y = r * rt.cellSize + 1.f;

            cell.setPosition(x, y);
            cell.setFillColor(sf::Color(60, 60, 60));

            // 选中格子（红）
            if (rt.selection && r == rt.selectedRow && c == rt.selectedCol) {
                cell.setFillColor(sf::Color::Red);
            }

            // 可跳目标（绿）
            bool isPossible = false;
            if (rt.selection) {
                for (auto target : rt.possibleTargets) {
                    if (target.first == r && target.second == c) {
                        isPossible = true;
                        break;
                    }
                }
            }
            if (isPossible) {
                cell.setFillColor(sf::Color::Green);
            }

            // 特殊格染色
            if (type == CellType::Ice) {
                cell.setFillColor(sf::Color::Blue);
            } else if (type == CellType::Barrier) {
                cell.setFillColor(sf::Color(150, 75, 0)); // 棕色
            } else if (type == CellType::Swamp) {
                cell.setFillColor(sf::Color(0, 100, 0));  // 深绿
            } else if (type == CellType::Goal) {
                cell.setFillColor(sf::Color::Cyan);
            } else if (type == CellType::Teleport) {
                cell.setFillColor(sf::Color(128, 0, 128)); // 紫色格子黄色格子
            }

            window.draw(cell);

            // 静态棋子（非动画中那颗）
            if (state == CellState::Peg) {
                bool skip = false;
                if (rt.isAnimating &&
                    r == rt.animToRow && c == rt.animToCol) {
                    skip = true;
                }
                if (rt.isIceAnimating &&
                    r == rt.iceToRow && c == rt.iceToCol) {
                    skip = true;
                }

                if (!skip) {
                    // 国王棋子方块
                    if (type == CellType::King) {
                        kingPiece.setPosition(
                            x + rt.cellSize * 0.5f,
                            y + rt.cellSize * 0.5f
                        );
                        window.draw(kingPiece);
                    } else {
                        peg.setPosition(
                            x + rt.cellSize * 0.5f,
                            y + rt.cellSize * 0.5f
                        );
                        window.draw(peg);
                    }
                }
            }
        }
    }

    // 跳跃动画（圆形）
    if (rt.isAnimating) {
        float t = rt.animTime / rt.animDuration;
        if (t > 1.f) t = 1.f;

        float basex = rt.animStart.x * (1.f - t) + rt.animEnd.x * t;
        float basey = rt.animStart.y * (1.f - t) + rt.animEnd.y * t;

        float H = rt.cellSize * 0.6f;
        float offset = -H * (4.f * t * (1.f - t));

        float drawX = basex + pegRadius;
        float drawY = basey + offset + pegRadius;

        animPeg.setPosition(drawX, drawY);
        window.draw(animPeg);
    }

    // 冰滑动画（圆形）
    if (!rt.isAnimating && rt.isIceAnimating) {
        float t = rt.iceTime / rt.iceDuration;
        if (t > 1.f) t = 1.f;

        float x = rt.iceStart.x * (1.f - t) + rt.iceEnd.x * t;
        float y = rt.iceStart.y * (1.f - t) + rt.iceEnd.y * t;

        animPeg.setPosition(x, y);
        window.draw(animPeg);
    }
    if (rt.isTeleportAnimating &&
        &currentBoard(rt) == &rt.floors[rt.currentFloor] &&
        rt.teleportRow >= 0 && rt.teleportCol >= 0) 
    {
        float t = rt.teleportTime / rt.teleportDuration;
        if (t > 1.f) t = 1.f;

        float basedRadius = rt.cellSize * 0.35f;
        float radius = basedRadius * (1.2f - 0.8f * t);

        sf::CircleShape tp(radius);

        // 颜色变淡
        unsigned char alpha = static_cast<unsigned char>(255.f * (1.f - t));
        tp.setFillColor(sf::Color(255, 255, 255, alpha));
        tp.setOutlineThickness(1.f);
        tp.setOutlineColor(sf::Color(255, 255, 255, alpha));

        tp.setOrigin(radius, radius);

        float x = (rt.teleportCol + 0.5f) * rt.cellSize;
        float y = (rt.teleportRow + 0.5f) * rt.cellSize;

        tp.setPosition(x, y);
        window.draw(tp);
    }

    window.display();
}

// ===== 结算 & 成绩记录 =====

void evaluation(bool win,
                int pegCount,
                int moveCount,
                GameMode mode)
{
    std::cout << "剩余棋子数: " << pegCount << "\n";
    std::cout << "总步数: "   << moveCount << "\n";

    if (win) {
        std::cout << "—— 恭喜，胜利！——\n";
    } else {
        std::cout << "—— 本局失败，再接再厉！——\n";
    }

    // 只有在传统模式胜利时，统计“最好成绩”
    if (mode != GameMode::Classic || !win) return;

    std::vector<int> number(2);
    number[0] = std::numeric_limits<int>::max(); // 最少棋子
    number[1] = std::numeric_limits<int>::max(); // 最少步数

    std::ifstream file("bestscore.txt");
    if (file) {
        std::string line;
        int idx = 0;
        while (idx < 2 && std::getline(file, line)) {
            try {
                number[idx] = std::stoi(line);
            } catch (...) {}
            ++idx;
        }
        file.close();
    }

    if (pegCount < number[0]) number[0] = pegCount;
    if (moveCount < number[1]) number[1] = moveCount;

    std::cout << "历史最少棋子: " << number[0] << "\n";
    std::cout << "历史最少步数: " << number[1] << "\n";

    std::ofstream file2("bestscore.txt");
    if (file2) {
        file2 << number[0] << "\n";
        file2 << number[1] << "\n";
        file2.close();
    }
}

// ===== main =====

int main() {
    // 解决 Windows 控制台中文乱码
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    GameState gameState = GameState::Playing;
    GameRuntime rt(gameState);

    // 控制台获取一局配置
    GameConfig cfg = askConfigFromConsole();
    initGame(rt, cfg);

    // 创建窗口（按单层大小来，所有层大小相同）
    sf::RenderWindow window(
        sf::VideoMode(
            static_cast<unsigned int>(Board::Cols * rt.cellSize),
            static_cast<unsigned int>(Board::Rows * rt.cellSize)
        ),
        "Peg Solitaire - Multi Layer"
    );

    sf::Clock gameClock;

    while (window.isOpen()) {
        sf::Event event;
        float dt = gameClock.restart().asSeconds();

        // 更新动画时间
        if (rt.isAnimating) {
            rt.animTime += dt;
            if (rt.animTime >= rt.animDuration) {
                rt.animTime   = rt.animDuration;
                rt.isAnimating = false;

                if (rt.isIceAnimating) {
                    rt.iceTime = 0.f;
                }
            }
        }
        if (!rt.isAnimating && rt.isIceAnimating) {
            rt.iceTime += dt;
            if (rt.iceTime >= rt.iceDuration) {
                rt.iceTime        = rt.iceDuration;
                rt.isIceAnimating = false;
            }
        }
        if (rt.isTeleportAnimating){
            rt.teleportTime += dt;
            if (rt.teleportTime >= rt.teleportDuration) {
                rt.teleportTime = rt.teleportDuration;
                rt.isTeleportAnimating = false;
            }
        }

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (gameState == GameState::Playing) {
                handlePlaying(event, gameState, rt);
            }
        }

        if (gameState == GameState::Playing) {
            drawGame(window, rt);

            rt.pegCount = totalPegs(rt);
            bool hasMoveAll = anyMove(rt);

            // Chess 模式：如果国王全灭，立即失败
            if (rt.gameMode == GameMode::Chess &&
                !anyKingAlive(rt) &&
                !rt.isGameOver) {
                rt.isGameOver = true;
                evaluation(false, rt.pegCount, rt.moveCount, rt.gameMode);
                break;
            }

            // 没有任何可行步：根据模式判断胜负
            if (!hasMoveAll && !rt.isGameOver) {
                rt.isGameOver = true;

                bool win = false;
                switch (rt.gameMode) {
                case GameMode::Classic:
                    win = (rt.pegCount == 1);
                    break;
                case GameMode::Lattice:
                    win = globalGoalWin(rt);
                    break;
                case GameMode::Chess:
                    win = anyKingAlive(rt);
                    break;
                }

                evaluation(win, rt.pegCount, rt.moveCount, rt.gameMode);
                break;
            }
        }
    }

    return 0;
}
