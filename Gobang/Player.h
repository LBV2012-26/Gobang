#pragma once

#include <memory>
#include <QMouseEvent>
#include <QObject>

#include "Board.h"
#include "Evaluator.h"
#include "MainWindow.h"

class Player : public QObject {
    Q_OBJECT

public:
    Player(std::shared_ptr<Board> Board, std::shared_ptr<MainWindow> MainWindow);
    void PutPawn(bool bHumanFlag);

private:
    void InitPlayer();
    void HumanPutPawn(const Board::PawnInfo& Pawn);
    void MachinePutPawn(const Board::PawnInfo& LastHumanPawn);

private slots:
    void Slot_MouseEvent(QMouseEvent* Event);

private:
    std::shared_ptr<Board>      _Board;
    std::shared_ptr<Evaluator>  _Evaluator;
    std::shared_ptr<MainWindow> _MainWindow;
    Board::PawnType             _HumanPawn;
    Board::PawnType             _MachinePawn;
    Board::PawnInfo             _LastMachineCache;
    bool                        _bHumanFlag;

#ifdef _DEBUG
    bool                        _bSimulateMachine = false;
#endif // _DEBUG

};
