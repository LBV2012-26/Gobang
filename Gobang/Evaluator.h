#pragma once

#include <memory>
#include <unordered_map>
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

    struct LayoutCache {
    public:
        LayoutCache() = default;
        LayoutCache(int Score, int Depth) : Score(Score), Depth(Depth) {}
        LayoutCache(const Board::PawnInfo& VcxPoint, int Depth) : VcxPoint(VcxPoint), Depth(Depth) {}

    public:
        Board::PawnInfo VcxPoint;
        int Score = 0;
        int Depth = 0;
    };

public:
    Evaluator(std::shared_ptr<Board> Board, Board::PawnType PawnType, double Aggressiveness);

    bool IsGameOver(const Board::PawnInfo& LatestPawn);
    int Minimax(int CurrentNode, int Depth, int Alpha, int Beta, Board::PawnType PawnType);
    Board::PawnInfo GetBestMove(bool bProcessCalcKill, bool bIsVct = false, int NextDepth = 0);

private:
    int Evaluate(Board::PawnInfo& Pawn);
    std::vector<Board::PawnInfo> GeneratePoints(Board::PawnType PawnType);
    std::vector<Board::PawnInfo> FindVcxPoints(Board::PawnType PawnType, bool bIsVct);
    QString GetSituation(const Board::PawnInfo& Pawn, int Direction);
    QChar GetPawn(const Board::PawnInfo& Pawn, int Direction, int Offset);
    PawnLayout GetPawnLayout(const QString& Str);
    int EvalBoard();
    bool HasLayout(const QString& Str, const std::vector<QString>& Layout);
    bool HasLayoutNearPawn(const Board::PawnInfo& Pawn, const std::vector<QString>& Layout);
    Board::PawnInfo CalcVcxKill(int NextDepth, bool bIsVct, Board::PawnType PawnType);
    Board::PawnInfo GetBestPoint(std::vector<Board::PawnInfo>& Points);
    std::vector<Board::PawnInfo> GenRandomPoints(std::size_t Amount);
    Board::PawnInfo DeepingCalcKill(int NextDepth, int MaxDepth, bool bIsVct);
    Board::PawnInfo DeepingMinimax(int NextDepth, int MaxDepth);

private:
    void PutPawn(const Board::PawnInfo& Point) {
        _Board->PutPawn(Point, true, false);
        CalcHash(Point);
    }

    void RevokePawn(const Board::PawnInfo& Point) {
        _Board->PutPawn({ Point.Row, Point.Column, Board::_kEmpty }, true, false);
        CalcHash(Point);
    }

    int GetScore(const PawnLayout& Layout) const {
        return static_cast<int>(Layout);
    }

    bool ScoreBetween(int Score, const PawnLayout& Left, const PawnLayout& Right) const {
        return Score >= static_cast<int>(Left) && Score < static_cast<int>(Right);
    }

    long long CalcHash(const Board::PawnInfo& Pawn) {
        int x = Pawn.Row;
        int y = Pawn.Column;
        _HashCode ^= Pawn.Type == Board::_kBlack ? _BlackZobrist[x][y] : _WhiteZobrist[x][y];
        return _HashCode;
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
    double                 _Aggressiveness;
    std::vector<std::pair<const std::vector<QString>, PawnLayout>> _ScoreMap;

    std::vector<std::vector<long long>> _BlackZobrist;
    std::vector<std::vector<long long>> _WhiteZobrist;
    std::unordered_map<long long, LayoutCache> _Cache;
    long long _HashCode;
};
