#include "board.hpp"
#include <SFML/Graphics.hpp>    //SFML图形库
#include <iostream>
int main(){ 
    const float cellSize = 64.f;       //绘制格子的大小
    Board Board;                       //创建一个棋盘对象
    sf::RenderWindow window(
        sf::VideoMode(
            static_cast<unsigned int>(Board::Cols * cellSize),      //录入的宽强行转化为整型
            static_cast<unsigned int>(Board::Rows * cellSize)       //高
        ),
        "Peg Solitaire"                  //窗口标题
    );
    // 引入状态量判断交互
    bool Selection = false;
    int selectedRow = -1;
    int selectedCol = -1;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {                          //检查窗口是否有待处理的事件，如果有，则记录进event，并返回true
            if (event.type == sf::Event::Closed) {                 //如果窗口关闭，则关闭窗口
                window.close();
            }
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {    //如果鼠标左键按下
                int mouseX = event.mouseButton.x;
                int mouseY = event.mouseButton.y;
                int col = static_cast<int>(mouseX / cellSize);
                int row = static_cast<int>(mouseY / cellSize);
                if (!Board.inBounds(row, col)) {
                    Selection = false;
                    continue;
                } else {
                    if (!Selection) {                               //如果没有选子，那么就按起跳子处理
                        if (Board.canMove(row, col)) {
                            Selection = true;
                            selectedRow = row;
                            selectedCol = col;
                            continue;
                        }else{
                            Selection = false;
                            continue;
                        }
                    }else {
                        int fromRow = selectedRow;                     //如果已经选子，那么就按目的位置处理
                        int fromCol = selectedCol;
                        int toRow = row;
                        int toCol = col;
                        if (fromRow == toRow && fromCol == toCol) {
                            Selection = false;
                        }
                        else {
                        if (Board.canJump(fromRow, fromCol, toRow, toCol)) {        //如果可以跳，那么就跳
                            Board.applyJump(fromRow, fromCol, toRow, toCol);
                            Selection = false;
                        }else {
                            continue;
                        }
                    }   
                    }
                }  
            }
            Board.countPegs();
        }
    // 绘图
    window.clear(sf::Color::Black);                            //清空窗口内容
    sf::RectangleShape cell({cellSize-2.f, cellSize-2.f});     //绘制格子      
    cell.setOutlineThickness(2.f);                             //格子的边框像素为2
    cell.setFillColor(sf::Color::White);                       //格子的颜色为白色

    sf::CircleShape peg(cellSize/2.f);                         //绘制棋子
    peg.setFillColor(sf::Color(220,220,50));                   //棋子的颜色为黄色

    for (int r = 0;r < Board::Rows;r++){
        for (int c = 0;c < Board::Cols;c++){
            CellState state = Board.at(r,c);
            // 如果是空子，则不画
            if (state == CellState::Invalid) {
                continue;
            }
            float x = c * cellSize + 1.f;
            float y = r * cellSize + 1.f;
            // 默认格子颜色
            cell.setPosition(x,y);
            cell.setFillColor(sf::Color(60,60,60));            //格子填充的颜色为灰色

            if (Selection && r == selectedRow && c == selectedCol) {
                cell.setFillColor(sf::Color::Red);     //如果选子，则格子颜色为浅灰色
            }
            window.draw(cell);
            // 画子
            if (state == CellState::Peg) {
                peg.setPosition(
                    x + cellSize * 0.5f - peg.getRadius(),
                    y + cellSize * 0.5f - peg.getRadius());
                window.draw(peg);
            }
        }
    }
    window.display();
    if (!Board.isSolved()) {
        std :: cout << "Pegs left: " << Board.countPegs() << std::endl;
        break;
    };
    }
}

