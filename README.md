#  Peg Solitaire Tactical Edition

#  孔明棋战术版（中英双语 README）

---

# 1. Overview（项目概述）

### English

**Peg Solitaire Tactical Edition** is an advanced reinvention of the classic Peg Solitaire puzzle game, developed in **C++ + SFML**.
This version introduces tactical depth, multi-layer mechanics, special tiles, configurable rule systems, and full animation support.

### 中文

**孔明棋战术版** 是使用 **C++ + SFML** 深度重制的孔明棋解谜游戏。
与传统孔明棋不同，本项目加入了战术复杂度、多层棋盘、特殊格子、可配置规则系统，以及完整的动画系统。

---

# 2. Features（功能特性）

### English

✔ Classic movement rules
✔ Smooth animations (jump, slide, teleport pulse)
✔ Multi-layer board system (1–3 floors)
✔ Teleport tiles enabling cross-floor movement
✔ Various special tiles (Ice, Barrier, Anchor, Goal, Teleport, King)
✔ Movable King piece with survival rule
✔ Console configuration before gameplay
✔ Full undo via complete state snapshots
✔ Clear separation of logic (Board) and rendering (main.cpp)

### 中文

✔ 经典孔明棋跳跃规则
✔ 平滑动画（跳跃、冰滑、传送闪光）
✔ 多层棋盘系统（1～3 层）
✔ 传送格支持楼层间移动
✔ 多种特殊格子（冰格、障碍格、锁定格、目标格、传送格、国王格）
✔ 可移动的国王棋子（保护国王玩法）
✔ 开局前通过终端选择规则
✔ 完整撤销（使用整局快照）
✔ 逻辑与渲染彻底分离（Board 独立，main.cpp 负责显示）

---

# 3. Project Structure（项目结构）

### English

```
project/
│
├─ src/
│   ├─ main.cpp          # Rendering, animation, input, multi-floor logic
│   ├─ Board.hpp         # Board structure and rule declarations
│   └─ Board.cpp         # Rule implementation (jump, King, tiles, maps)
│
├─ CMakeLists.txt
└─ README.md
```

### 中文

```
project/
│
├─ src/
│   ├─ main.cpp          # 渲染、动画、输入、多层逻辑
│   ├─ Board.hpp         # 棋盘数据结构与规则声明
│   └─ Board.cpp         # 规则实现（跳跃、国王、特殊格、地图）
│
├─ CMakeLists.txt        # 构建配置
└─ README.md             # 说明文档
```

---

# 4. Gameplay Rules（游戏规则）

## 4.1 Jump Rules（跳跃规则）

### English

A legal jump requires:

1. Start cell contains a peg
2. End cell is empty
3. Distance is exactly 2 steps in a straight line
4. Middle cell contains a peg
5. Landing cannot be on Barrier
6. Special tile effects apply

### 中文

合法跳跃必须满足：

1. 起点为棋子
2. 终点为空格
3. 跳跃距离为“隔一格跳两格”
4. 中间格必须有棋子
5. 不能落在障碍格
6. 特殊格效果同时生效

---

## 4.2 Special Tile Rules（特殊格子规则）

### Ice（冰格）

**EN**：Landing causes automatic sliding by 1 extra step.
**中文**：落在冰格上时，棋子会沿着原方向自动再滑一格。

---

### Teleport（传送格）

**EN**：Moves peg to the same coordinate of the next floor; floors loop (1→2→3→1).
**中文**：将棋子传送至下一层同坐标位置；多层时循环传送（1→2→3→1）。

---

### Anchor（锁定格）

**EN**：A peg that lands here cannot be selected as the starting piece again.
**中文**：落在此格的棋子会被“钉住”，无法再次作为起点移动，但仍能作为桥被跳过。

---

### Barrier（障碍格）

**EN**：Cannot land on or jump through.
**中文**：不能落子，也不能被跳过。

---

### Goal（目标格）

**EN**：For Goal Mode — the final peg must land here.
**中文**：目标格模式下，最后唯一的棋子必须落在此处。

---

### King（国王）

**EN：**

* King is a **piece**, not a tile
* It moves with the peg
* If captured → **immediate game over**
* King **cannot be regenerated**
* No other peg will ever become King

**中文：**

* 国王是 **棋子** 而非格子
* King 会随棋子一起移动
* 如果被吃掉 → **立即失败**
* King 不会“复活”
* 其他棋子永远不会变成 King

---

## 4.3 Win Conditions（胜利条件）

| Mode          | EN Description                 | 中文说明                  |
| ------------- | ------------------------------ | -------------             |
| **Classic**   | End with exactly one peg       | 最终只剩一颗棋子           |
| **Goal Mode** | Final peg must be on Goal tile | 最后一枚棋子必须站在目标格  |
| **King Mode** | King survives to the end       | 国王活到最后，否则失败      |

---

# 5. Console Configuration（控制台配置系统）

### English

At startup, player selects:

* Number of floors
* Special tiles to enable (Ice/Barrier/Anchor)
* Board shape (Cross / BigCross / Triangle / Diamond)
* Win condition (Classic / Goal / King mode)

Board(s) are generated according to the configuration.

### 中文

游戏启动时，玩家需从终端选择：

* 楼层数（1~3）
* 启用哪些特殊格子（冰格/障碍/锁定格）
* 地图形状（十字/大十字/三角/菱形）
* 获胜模式（传统/目标格/保护国王）

棋盘会根据这些配置自动生成。

---

# 6. Multi-Layer System（多层系统）

### English

* Floors are stored in `vector<Board> floors`
* `currentFloor` determines which floor is being displayed
* Teleport tiles automatically appear at fixed coordinates (e.g., (1,3), (3,1), (3,5), (5,3))
* Teleportation cycles floors: 1→2→3→1
* Each floor generates additional empty spaces for better maneuverability

### 中文

* 所有楼层存储于 `vector<Board> floors`
* `currentFloor` 决定当前显示的楼层
* 多层游戏自动在固定坐标生成传送格（如 (1,3),(3,1),(3,5),(5,3)）
* 传送格在楼层间循环传送：1→2→3→1
* 多层会额外挖空部分格子，使空间更充足更可玩

---

# 7. Undo System（撤销系统）

### English

Every time the player completes an action, a **full snapshot of all floors** is saved:

```
history.push_back(floors);
```

Undo restores the entire game state (all floors, King position, special tiles, animations reset).

### 中文

玩家每走一步，程序都会保存 **整个多层棋盘的快照**：

```
history.push_back(floors);
```

撤销将恢复整局（所有楼层、国王位置、特殊格都回到上一状态）。

---

# 8. Animations（动画系统）

### English

The game includes 3 animation types:

1. **Jump animation** — parabola-like arc
2. **Ice slide** — linear glide movement
3. **Teleport pulse** — shrinking/transparent glowing circle

All animations are time-based (`dt`) and independent of frame rate.

### 中文

游戏包含三种动画：

1. **跳跃动画** —— 抛物线效果
2. **冰滑动画** —— 沿方向直线滑动
3. **传送脉冲动画** —— 缩放 & 透明度变化的光圈

所有动画都基于 `dt`，与帧率无关。

---

# 9. Build & Run（构建与运行）

### Requirements / 依赖

* C++17
* SFML 2.5.1
* CMake 3.16+
* Windows + Visual Studio 推荐

### Build / 构建

```
cmake -S . -B build
cmake --build build --config Debug
```

### Run / 运行

```
build/Debug/PegSolitaire.exe
```

---

# 10. Architecture (设计架构)

## Logic Layer — Board

### English

* Board rules independent from SFML
* Handles jump rules, tile effects, King logic
* Map generation for multiple shapes
* Special tile injection (Ice, Barrier, Anchor, Teleport, Goal)

### 中文

* `Board` 是纯逻辑层，与 SFML 完全解耦
* 处理跳跃规则、格子效果、国王逻辑
* 能生成多种地图形状
* 注入特殊格子（冰、障碍、锁定、传送、目标）

---

## Runtime Layer — GameRuntime

### English

Stores everything that *changes during gameplay*:

* All floors
* Current floor
* User selection
* Animation states
* History snapshots
* Config (user-selected rules)

### 中文

GameRuntime 保存**游戏运行过程中的所有变化状态**：

* 所有楼层
* 当前楼层
* 玩家选中信息
* 动画状态
* 历史快照
* 用户配置

---

## Rendering Layer — main.cpp

### English

* Handles mouse input, keyboard, undo, restart
* Produces visuals for tiles, pegs, animations
* Bridges Board logic and SFML drawing
* Drives the main game loop

### 中文

* 处理鼠标点击与键盘输入、撤销、重启
* 绘制棋盘、棋子和动画
* 将 Board 逻辑转换为可视化内容
* 驱动主循环（事件→更新→渲染）

---

# 11. Challenges & Lessons Learned（挑战与经验）

### 1. King must follow the peg, not the tile

**EN**：King cannot be treated as a tile, or new Kings will appear incorrectly.
**CN**：国王必须绑定在棋子，不可绑定在格子，否则会出现“新王”。

---

### 2. Multi-floor undo requires full state snapshots

**EN**：Undo cannot only store one floor.
**CN**：撤销必须保存整个多层棋盘状态。

---

### 3. Teleport tiles must be fixed

**EN**：Random teleporting destroys solvability.
**CN**：传送格必须固定位置，否则关卡可能无解。

---

### 4. Multi-layer boards need extra empty space

**EN**：Full boards give no movement space.
**CN**：多层必须额外挖洞，否则几乎无法移动。

---

### 5. Strict separation of logic and drawing

**EN**：Allows modifying rules without touching rendering.
**CN**：严格解耦逻辑与渲染，使规则扩展更容易。

---

# 12. Future Work（未来扩展）

### English

* UI-based rule selection (replacing console input)
* More special tiles (Mirror, Combo, Exploding tiles)
* More peg types (Ninja, Mage, etc.)
* Level loading from external JSON
* Built-in solver / auto-solvability checker
* Particle FX & advanced animations
* Sound effects

### 中文

* 基于 UI 的规则选择界面
* 更多特殊格（镜像格、连跳格、爆炸格等）
* 更多棋子类型（忍者、法师等）
* 从外部 JSON 加载自定义关卡
* 自动解题器 / 检测初始局面是否可解
* 粒子效果与高级动画
* 音效支持

---

# 13. Final Words（结语）

### English

Peg Solitaire Tactical Edition goes beyond the classic puzzle — it demonstrates real game-engine concepts:
multi-layer state, rule-driven logic, animation systems, undo stack design, and clean architecture.

### 中文

孔明棋战术版不仅是经典游戏的扩展，更是一次完整的“游戏引擎式”实践：
多层状态管理、规则驱动逻辑、动画系统、撤销栈设计、架构分层……
这些都是游戏工程的核心能力。

---
