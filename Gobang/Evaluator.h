#pragma once

#include <atomic>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

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
    Evaluator(const Evaluator&) = delete;

    bool IsGameOver(const Board::PawnInfo& LatestPawn);
    Board::PawnInfo GetBestMove(int MaxDepth, bool bProcessCalcKill = false, int MaxVcxDepth = 0, bool bIsVct = false, int NextDepth = 0, const Board::PawnInfo& InputPoint = {});

private:
    int Minimax(int CurrentDepth, int NextDepth, int Alpha, int Beta, Board::PawnType PawnType, const Board::PawnInfo& InputPoint);
    int Evaluate(Board::PawnInfo& Pawn);
    std::vector<Board::PawnInfo> GeneratePoints(Board::PawnType PawnType);
    std::vector<Board::PawnInfo> FindVcxPoints(Board::PawnType PawnType, bool bIsVct);
    std::string GetSituation(const Board::PawnInfo& Pawn, int Direction);
    char GetPawn(const Board::PawnInfo& Pawn, int Direction, int Offset);
    PawnLayout GetPawnLayout(const std::string_view Str);
    bool HasLayout(const std::string_view Str, const std::vector<std::string>& Layout);
    bool HasLayoutNearPawn(const Board::PawnInfo& Pawn, const std::vector<std::string>& Layout);
    int EvalBoard();
    Board::PawnInfo CalcVcxKill(int NextDepth, bool bIsVct, Board::PawnType PawnType);
    Board::PawnInfo GetBestPoint(std::vector<Board::PawnInfo>& Points);
    std::vector<Board::PawnInfo> GenRandomPoints(std::size_t Amount);
    Board::PawnInfo DeepingCalcKill(int NextDepth, int MaxDepth, bool bIsVct);
    void DeepingMinimax(int NextDepth, int MaxDepth, const Board::PawnInfo& InputPoint);

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
    const std::vector<std::string> _kFiveLink;
    const std::vector<std::string> _kFour;
    const std::vector<std::string> _kBlockFour;
    const std::vector<std::string> _kThree;
    const std::vector<std::string> _kBlockThree;
    const std::vector<std::string> _kTwo;
    const std::vector<std::string> _kBlockTwo;
    const std::vector<std::string> _kOne;
    const std::vector<std::string> _kBlockOne;

    std::shared_ptr<Board>                                             _Board;
    Board::PawnInfo                                                    _BestMove;
    Board::PawnType                                                    _MachinePawn;
    double                                                             _Aggressiveness;
    std::atomic<std::vector<Board::PawnInfo>>                          _BestMoves;
    std::vector<std::pair<const std::vector<std::string>, PawnLayout>> _ScoreMap;
    std::vector<std::vector<long long>>                                _BlackZobrist;
    std::vector<std::vector<long long>>                                _WhiteZobrist;
    std::atomic<std::unordered_map<long long, LayoutCache>>            _Cache;
    std::atomic<long long>                                             _HashCode;
};
