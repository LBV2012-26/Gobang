#pragma once

#include <memory>
#include "Board.h"
#include "MainWindow.h"
#include "Player.h"

class GameBase {
private:
    enum class GameState {
        kMenu, kActivated, kWon, kLose
    };

public:
    GameBase();

private:
    void InitGame();
    void ActiveRound();

private:
    std::shared_ptr<Board>      _Board;
    std::shared_ptr<MainWindow> _MainWindow;
    std::shared_ptr<Player>     _Player;
    GameState                   _GameState;
    bool                        _bHumanFlag;
};
