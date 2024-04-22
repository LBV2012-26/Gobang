#include "Player.h"
#include "MainWindow.h"

#include <cstdlib>
#include <chrono>
#include <limits>
#include <random>

#ifdef _DEBUG
#include <QDebug>
#endif // _DEBUG

Player::Player(std::shared_ptr<Board> Board, std::shared_ptr<MainWindow> MainWindow) :
    _Board(Board), _Evaluator(nullptr), _MainWindow(MainWindow), _bHumanFlag(true), _DebugMode(false)
{
    connect(_MainWindow.get(), &MainWindow::Signal_MouseEvent, this, &Player::Slot_MouseEvent);
    InitPlayer();
}

void Player::PutPawn(bool bHumanFlag) {
    _bHumanFlag = bHumanFlag;
}

void Player::InitPlayer() {
    _HumanPawn   = Board::_kBlack;
    _MachinePawn = Board::_kWhite;

    double Aggressiveness = _MachinePawn == Board::_kBlack ? 2.5 : 0.5;

    _Evaluator = std::make_shared<Evaluator>(_Board, _MachinePawn, Aggressiveness);
}

void Player::HumanPutPawn(const Board::PawnInfo& Pawn) {
    auto Result = _Board->PutPawn(Pawn);
    if (Result.second == false) {
        return;
    }

    if (_Evaluator->IsGameOver(Pawn)) {
        std::cout << "Game over" << std::endl;
    }

    if (!_DebugMode) {
        _bHumanFlag = !_bHumanFlag;
        MachinePutPawn(Result.first);
    }
}

void Player::MachinePutPawn(const Board::PawnInfo& LastHumanPawn) {
    auto BeginTime = std::chrono::steady_clock::now();
    if (_Board->GetPawnCount() == 0 && _MachinePawn == Board::_kBlack) {
        _Board->PutPawn({ 7, 7, _MachinePawn }, true);
        return;
    }

    if (_Board->GetPawnCount() <= 6) {
        _Board->PutPawn(_Evaluator->GetBestMove(6), true);
    } else if (_Board->GetPawnCount() <= 10) {
        _Board->PutPawn(_Evaluator->GetBestMove(6, true, 8), true);
    } else if (_Board->GetPawnCount() <= 30) {
        _Board->PutPawn(_Evaluator->GetBestMove(8, true, 10), true);
    } else if (_Board->GetPawnCount() <= 60) {
        _Board->PutPawn(_Evaluator->GetBestMove(10, true, 12), true);
    } else {
        _Board->PutPawn(_Evaluator->GetBestMove(12, true, 12), true);
    }

    _bHumanFlag = !_bHumanFlag;
    auto   EndTime  = std::chrono::steady_clock::now();
    double Duration = std::chrono::duration<double>(EndTime - BeginTime).count();
    std::cout << "Duration time: " << Duration << "s" << std::endl;
}

void Player::Slot_MouseEvent(QMouseEvent* Event) {
    if (!_DebugMode) {
        if (_bHumanFlag == true && Event->button() == Qt::LeftButton) {
            HumanPutPawn({ Event->pos().y(), Event->pos().x(), _HumanPawn });
            _bHumanFlag = true;
        }

        if (Event->button() == Qt::RightButton) {
            MachinePutPawn({});
        }
    } else {
        if (Event->button() == Qt::LeftButton) {
            HumanPutPawn({ Event->pos().y(), Event->pos().x(), Board::_kBlack });
        } else if (Event->button() == Qt::RightButton) {
            HumanPutPawn({ Event->pos().y(), Event->pos().x(), Board::_kWhite });
        }
    }

    if (Event->button() == Qt::MiddleButton) {
        if (_DebugMode) {
            _bHumanFlag = false;
        }
        _DebugMode = !_DebugMode;
    }
}
