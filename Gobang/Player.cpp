#include "Player.h"
#include "MainWindow.h"

#include <cstdlib>
#include <limits>
#include <random>
#include <Windows.h>

#ifdef _DEBUG
#include <QDebug>
#endif // _DEBUG

#ifdef min
#undef min
#endif // min
#ifdef max
#undef max
#endif // max

Player::Player(std::shared_ptr<Board> Board, std::shared_ptr<MainWindow> MainWindow) :
    _Board(Board), _Evaluator(nullptr), _MainWindow(MainWindow), _bHumanFlag(true)
{
    connect(_MainWindow.get(), &MainWindow::Signal_MouseEvent, this, &Player::Slot_MouseEvent);
    InitPlayer();
}

void Player::PutPawn(bool bHumanFlag) {
    _bHumanFlag = bHumanFlag;
}

void Player::InitPlayer() {
    _HumanPawn   = Board::_kWhite;
    _MachinePawn = Board::_kBlack;
    _Evaluator   = std::make_shared<Evaluator>(Evaluator(_Board, _MachinePawn));
}

void Player::HumanPutPawn(const Board::PawnInfo& Pawn) {
    auto Result = _Board->PutPawn(Pawn);
    if (Result.second == false) {
        return;
    }
    _bHumanFlag = !_bHumanFlag;
    MachinePutPawn(Result.first);
}

void Player::MachinePutPawn(const Board::PawnInfo& LastHumanPawn) {
    if (_Board->GetPawnCount() == 0 && _MachinePawn == Board::_kBlack) {
        _Board->PutPawn({ 7, 7, _MachinePawn }, true);
        return;
    }

    if (_Board->GetPawnCount() >= 2) {
        if (_Board->GetPawnCount() <= 6) {
            _Evaluator->Minimax(0, 4, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), true, _MachinePawn);
        } else if (_Board->GetPawnCount() <= 60) {
            _Evaluator->Minimax(0, 6, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), true, _MachinePawn);
        } else {
            _Evaluator->Minimax(0, 8, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), true, _MachinePawn);
        }
        _Board->PutPawn(_Evaluator->GetBestMove(), true);
    } else {
        Sleep(500);
        int RandomNumber = 0;
        int NextPos[2]{};
        do {
            for (int i = 0; i != 2; ++i) {
                std::mt19937 Engine(std::random_device{}());
                std::uniform_int_distribution<int> Distribution(-100, 100);
                RandomNumber = Distribution(Engine);
                if (RandomNumber < 0) {
                    NextPos[i] = -1;
                } else {
                    NextPos[i] = 1;
                }
            }
        } while (_Board->PutPawn({ LastHumanPawn.Row + NextPos[0], LastHumanPawn.Column + NextPos[1], _MachinePawn }, true).second == false);
    }
    _bHumanFlag = !_bHumanFlag;
}

void Player::Slot_MouseEvent(QMouseEvent* Event) {
    if (_bHumanFlag == true && Event->button() == Qt::LeftButton) {
        HumanPutPawn({ Event->pos().y(), Event->pos().x(), _HumanPawn });
        _bHumanFlag = true;
    }

    if (Event->button() == Qt::RightButton) {
        MachinePutPawn({});
    }
}
