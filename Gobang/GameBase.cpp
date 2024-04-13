#include "GameBase.h"

GameBase::GameBase() : _Board(new Board), _MainWindow(new MainWindow(_Board)), _Player(new Player(_Board, _MainWindow)),
                       _GameState(GameState::kMenu), _bHumanFlag(true)
{
    InitGame();
}

void GameBase::InitGame() {
    _MainWindow->show();
    ActiveRound();
}

void GameBase::ActiveRound() {
    if (_GameState != GameState::kActivated) {
        _GameState = GameState::kActivated;
    }

    _Player->PutPawn(_bHumanFlag);
}
