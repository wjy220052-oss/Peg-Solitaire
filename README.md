1. Overview（项目概述）

EN
Peg Solitaire Tactical Edition is a C++ + SFML implementation of the classic single-player puzzle Peg Solitaire, enhanced with an extensible architecture suitable for rule modifications (“mods”), visual interaction, and further game system upgrades.

This project was built from scratch, including game logic, rendering, input handling, and complete debugging. It also documents all pitfalls and lessons learned during development — ideal for portfolio demonstration or learning C++ game engineering fundamentals.

中文
孔明棋战术版使用 C++ + SFML 从零实现传统单机解谜游戏，并基于模块化架构设计，支持未来扩展（魔改规则）、视觉交互、复杂玩法追加等。

项目包含了完整的游戏逻辑、渲染系统、用户输入处理，以及从零调试环境到成品的全部踩坑记录，非常适合作为作品集或学习 C++ 游戏开发的示例工程。

📌 2. Features（功能特性）

EN

Classic 7×7 cross-shaped Peg Solitaire board
Visual board rendering (SFML)
Click-to-select and click-to-jump mechanics
Automatic rule validation (legal moves only)
Real-time rendering loop (game loop architecture)
Modular logic layer (Board class independent from SFML)
Comprehensive debugging outputs
Designed with expandability in mind (future "mod" rules: Ice, Trap, Goal etc.)

中文

经典 7×7 十字孔明棋棋盘
使用 SFML 的可视化渲染
点击选中、点击跳子 的交互方式
自动规则判断（只有合理跳法才生效）
实时渲染循环（符合游戏引擎结构）
逻辑层与渲染层完全分离（Board 独立可扩展）
全程加入调试输出，方便定位问题
将来可轻松扩展魔改规则（冰格、陷阱、目标格等）

3. Project Structure（项目结构）
project/
│
├─ src/
│   ├─ main.cpp          # Rendering + input handling + game loop
│   ├─ Board.hpp         # Game logic declarations
│   └─ Board.cpp         # Game logic implementation
│
├─ CMakeLists.txt        # Build configuration
└─ README.md             # This file


中文说明

main.cpp：负责渲染、事件处理、游戏主循环
Board.hpp/.cpp：全部游戏规则与棋盘逻辑
CMakeLists.txt：构建流程定义
README.md：项目说明文件

4. Game Rules（游戏规则）

A valid move requires:
Starting cell contains a peg
Ending cell is empty
Move is strictly horizontal or vertical, exactly 2 cells away
The cell in between contains a peg (which will be “jumped” and removed)
Goal:
Reduce the board to as few pegs as possible — ideally 1 peg.

中文
一个合法跳跃必须满足以下条件：
起点格必须有棋子
终点格必须为空
跳跃必须是 上下左右直线、隔一格跳二格
中间格必须有子（会被吃掉）
游戏目标：
尽量消除所有棋子，最好只剩 1 颗。

5. Build & Run（构建与运行）
EN

Requirements:
C++17 compatible compiler
SFML 2.5.1
CMake 3.16+
Windows + Visual Studio Build Tools recommended
Build:
cmake -S . -B build
cmake --build build --config Debug
Run:
build/Debug/PegSolitaire.exe

中文

依赖：
支持 C++17 的编译器
SFML 2.5.1
CMake 3.16+
推荐 Windows + VS 工具链
构建：
cmake -S . -B build
cmake --build build --config Debug
运行：
build/Debug/PegSolitaire.exe

📌 6. Architecture Design（架构设计）

EN
The project follows a clean separation between:
Logic Layer(Board)

Implements all rules: move validation, jump execution, board state
Does not depend on SFML
Easily extendable for future “mods”
Rendering Layer (main.cpp)
Handles user mouse input
Draws board & pegs
Manages game loop (event → update → render)
This architecture mirrors real game engines and allows modifying one layer without breaking the other.

中文
本项目强调清晰的分层结构：
逻辑层（Board）
实现全部规则：合法判断、执行跳子、棋盘状态
不依赖 SFML
未来非常容易扩展新规则
渲染层（main.cpp）
接收鼠标输入
绘制棋盘与棋子
负责游戏循环（事件 → 更新 → 渲染）
这样的设计与真正的游戏引擎一致，使得逻辑或渲染任一部分都能独立升级。

7. Pitfalls & Lessons Learned（踩坑与经验总结）

1. Running the Wrong Executable（运行错了程序）

EN
VS Code attempted to compile and run src/main.cpp directly (producing main.exe),
which ignores CMake, ignores SFML include paths, and always fails.

Fix:
Always run the CMake-built executable:

build/Debug/PegSolitaire.exe


中文
VS Code 默认会用自身的“单文件编译模式”，生成 main.exe，
完全不会链接 SFML，也不会使用 CMake 配置，导致各种错误。

解决：
必须运行 CMake 生成的：

build/Debug/PegSolitaire.exe

2. Wrong inBounds() Logic（边界判断写反）

EN
A wrong implementation of inBounds() allowed out-of-range coordinates and
misclassified valid cells as invalid, breaking all move logic.

中文
inBounds() 写成了错误的逻辑，使得越界坐标被认为在棋盘内、
有效格子被判成棋盘外，导致棋盘行为完全混乱。

Correct version:

return (r >= 0 && r < Rows && c >= 0 && c < Cols);

3. applyJump() Using Invalid Instead of Empty（跳子后把棋子设成“棋盘外”）

EN
Mistakenly writing:

set(r1, c1, Invalid);


broke the board shape — future moves became impossible.

中文
将跳子后的原位置与被吃掉的位置设为 Invalid（棋盘外）
导致棋盘结构被破坏，只能操作一次。

Correct:

set(r1,c1,Empty);
set(mid,mid,Empty);

4. Uninitialized selectedRow/selectedCol（未初始化的选择状态）

EN
These variables contained garbage values when no valid piece was selected,
causing second clicks to reference invalid coordinates.

Fix: initialize them:

int selectedRow = -1;
int selectedCol = -1;


中文
selectedRow / selectedCol 未初始化，
在非法点击后保持垃圾值，导致后续点击进入错误分支，跳子失败。

解决：

int selectedRow = -1;
int selectedCol = -1;

5. Drawing Outside the Main Loop（绘图位置错误导致白屏）

EN
Rendering was accidentally placed outside while(window.isOpen()),
so no frame was drawn until the window closed → white screen.

中文
绘图代码被放在主循环外，
程序只处理事件不刷新画面 → 白屏。

8. Future Work（未来扩展）

EN

Special tiles: Goal, Ice, Trap

Special units: King, Ninja, Mage

Undo system using state stack

Multi-level support (read level files)

Animation system (smooth jumping)

Sound effects and UI text

中文

特殊格子：终点格、冰格、陷阱格

特殊棋子：王、忍者、法师

撤销功能（使用状态栈）

多关卡系统（读入外部关卡文件）

动画系统（跳子平滑插值）

UI 提示与音效

9. License（许可证）

MIT License 或其他你选择的开源协议。

10. Final Words（结语）

EN
This project represents not only a playable game but also a full journey of building, debugging, and understanding fundamental game engine concepts — event loops, rendering pipelines, logic separation, and state management.

中文
本项目不仅是一个可玩的小游戏，更是从零搭建游戏引擎关键结构的完整实践：
事件循环、渲染管线、逻辑分层、状态管理等核心概念均在其中体现
