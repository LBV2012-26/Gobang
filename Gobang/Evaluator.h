#pragma once

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
        kMultiFour  = 800000,   // 多冲四
        kFourThree  = 500000,   // 冲四活三
        kMultiThree = 100000,   // 多活三
        kThree      = 10000,    // 活三
        kBlockFour  = 9000,     // 冲四
        kTwo        = 100,      // 活二
        kOne        = 80,       // 活一
        kBlockThree = 30,       // 眠三
        kBlockTwo   = 10,       // 眠二
        kBlockOne   = 1,        // 眠一
        kEmpty      = 0,

        kHighRisk   = kMultiFour,
        kMiddleRisk = kFourThree,
        kLowRisk    = kMultiThree
    };

public:
    Evaluator(std::shared_ptr<Board> Board, Board::PawnType PawnType);

    bool IsGameOver();
    int Minimax(int CurrentNode, int Depth, int Alpha, int Beta, bool bMachineFlag, Board::PawnType PawnType);

private:
    int Evaluate(Board::PawnInfo& Pawn);
    std::vector<Board::PawnInfo> GeneratePoints(Board::PawnType PawnType);
    QString GetSituation(const Board::PawnInfo& Pawn, int Direction);
    QChar GetPawn(const Board::PawnInfo& Pawn, int Direction, int Offset);
    PawnLayout GetPawnLayout(const QString& Str);
    int EvalBoard();
    bool HasLayout(const QString& Str, const std::vector<QString>& Layout);
    std::vector<Board::PawnInfo> GenRandomPoints();

public:
    Board::PawnInfo GetBestMove() const {
        return _BestMove;
    }

private:
    int GetScore(const PawnLayout& Layout) const {
        return static_cast<int>(Layout);
    }

    bool BetweenScore(int Score, const PawnLayout& Left, const PawnLayout& Right) const {
        return Score >= static_cast<int>(Left) && Score < static_cast<int>(Right);
    }

private:
    const std::vector<QString> _kFiveLink;
    const std::vector<QString> _kFour;
    const std::vector<QString> _kBlockFour;
    const std::vector<QString> _kThree;
    const std::vector<QString> _kBlockThree;
    const std::vector<QString> _kTwo;
    const std::vector<QString> _kBlockTwo;
    const std::vector<QString> _kOne;
    const std::vector<QString> _kBlockOne;

    std::shared_ptr<Board> _Board;
    Board::PawnInfo        _BestMove;
    Board::PawnType        _MachinePawn;
    std::vector<std::pair<const std::vector<QString>, PawnLayout>> _ScoreMap;
};
