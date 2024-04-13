#pragma once

#include <map>
#include <memory>
#include <vector>
#include <QChar>
#include <QString>

#include "Board.h"

class Evaluator {
private:
    enum class PawnLayout : int {
        kFiveLink   = 10000000, // 连五
        kFour       = 1000000,  // 活四
        kBlockFour  = 2500,     // 冲四
        kThree      = 2000,     // 活三
        kBlockThree = 250,      // 眠三
        kDoubleTwo  = 300,      // 双活二
        kTwo        = 200,      // 活二
        kBlockTwo   = 50,       // 眠二
        kOne        = 25,       // 活一
        kBlockOne   = 1,        // 眠一
        kEmpty      = 0
    };

public:
    Evaluator(std::shared_ptr<Board> Board, Board::PawnType PawnType);

    bool IsGameOver();
    int Minimax(int CurrentNode, int Depth, int Alpha, int Beta, bool bMachineFlag, Board::PawnType PawnType);

private:
    int Evaluate(const Board::PawnInfo& Pawn);
    std::vector<Board::PawnInfo> GenBestPoint(Board::PawnType PawnType);
    QString GetSituation(const Board::PawnInfo& Pawn, int Direction);
    QChar GetPawn(const Board::PawnInfo& Pawn, int Direction, int Offset);
    PawnLayout GetPawnLayout(const QString& Str);
    int EvalBoard();
    bool HasLayout(const QString& Str, const std::vector<QString>& Layout);
    bool HasLayoutNearPawn(const Board::PawnInfo& Pawn, const std::vector<QString>& Layout);
    bool VerifyHighPriority(const Board::PawnInfo& Pawn);
    std::vector<Board::PawnInfo> GenRandomPoints();

public:
    // impl in class for inline
    const Board::PawnInfo GetBestMove() const {
        return _BestMove;
    }

    const int GetScore(const QString& Str) {
        return static_cast<int>(GetPawnLayout(Str));
    }

private:
    std::vector<QString> _FiveLink;
    std::vector<QString> _Four;
    std::vector<QString> _BlockFour;
    std::vector<QString> _Three;
    std::vector<QString> _BlockThree;
    std::vector<QString> _Two;
    std::vector<QString> _BlockTwo;
    std::vector<QString> _One;
    std::vector<QString> _BlockOne;

    std::shared_ptr<Board>                     _Board;
    Board::PawnInfo                            _BestMove;
    Board::PawnType                            _MachinePawn;
    std::map<std::vector<QString>, PawnLayout> _ScoreMap;

    // special layout multipler
    const int _kFourThree;   // 冲四活三
    const int _kDoubleThree; // 双三
    const int _kFourTwo;     // 冲四活二
    const int _kThreeTwo;    // 冲三活二
};
