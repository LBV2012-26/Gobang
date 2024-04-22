#include "Evaluator.h"

#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <format>
#include <iostream>
#include <limits>

#ifdef _DEBUG
#include <QDebug>
#endif // _DEBUG

Evaluator::Evaluator(std::shared_ptr<Board> Board, Board::PawnType PawnType, double Aggressiveness) :
    _Board(Board), _BestMove({}), _MachinePawn(PawnType), _Aggressiveness(Aggressiveness), _HashCode(0),
    _kFiveLink({ "XXXXX" }), // 连五
    _kFour({ "_XXXX_" }), // 活四
    _kThree({ "_XXX__", "_XX_X_", "_X_XX_", "__XXX_" }), // 活三
    _kBlockFour({ "_XXXX", "X_XXX", "XX_XX", "XXX_X", "XXXX_" }), // 冲四
    _kTwo({ "__XX__", "_XX___", "___XX_", "_X_X__", "__X_X_" }), // 活二
    _kOne({ "_X_#__", "__#_X_", "_#_X__", "__X_#_", "#_X___", "___X_#", "___#_X", "X_#___" }), // 活一
    _kBlockThree({ "#XXX__", "#XX_X_", "#X_XX_", "__XXX#", "_X_XX#", "_XX_X#" }), // 眠三
    _kBlockTwo({ "_XX#__", "__XX#_", "__#XX_", "_#XX__", "___XX#", "#XX___", "XX____", "____XX" }), // 眠二
    _kBlockOne({ "__X#__", "__#X__", "___#X_", "_X#___", "#X____", "____X#" }), // 眠一
    _BlackZobrist(kBoardSize, std::vector<long long>(kBoardSize, 0)),
    _WhiteZobrist(kBoardSize, std::vector<long long>(kBoardSize, 0))
{
    _ScoreMap.push_back({ _kFiveLink,   PawnLayout::kFiveLink });
    _ScoreMap.push_back({ _kFour,       PawnLayout::kFour });
    _ScoreMap.push_back({ _kThree,      PawnLayout::kThree });
    _ScoreMap.push_back({ _kBlockFour,  PawnLayout::kBlockFour });
    _ScoreMap.push_back({ _kTwo,        PawnLayout::kTwo });
    _ScoreMap.push_back({ _kOne,        PawnLayout::kOne });
    _ScoreMap.push_back({ _kBlockThree, PawnLayout::kBlockThree });
    _ScoreMap.push_back({ _kBlockTwo,   PawnLayout::kBlockTwo });
    _ScoreMap.push_back({ _kBlockOne,   PawnLayout::kBlockOne });

    std::mt19937_64 Engine(std::random_device{}());
    std::uniform_int_distribution<long long> Distribution;
    for (int x = 0; x != kBoardSize; ++x) {
        for (int y = 0; y != kBoardSize; ++y) {
            _BlackZobrist[x][y] = Distribution(Engine);
            _WhiteZobrist[x][y] = Distribution(Engine);
        }
    }
}

bool Evaluator::IsGameOver(const Board::PawnInfo& LatestPawn) {
    if (HasLayoutNearPawn(LatestPawn, _kFiveLink) || _Board->GetPawnCount() == 225.0) {
        return true;
    } else {
        return false;
    }
}

int Evaluator::Minimax(int CurrentDepth, int NextDepth, int Alpha, int Beta, Board::PawnType PawnType) {
    if (NextDepth == 0) {
        return EvalBoard();
    }

    bool bMachineFlag = PawnType == _MachinePawn;

    LayoutCache Cache;
    if (_HashCode != 0 && _Cache.find(_HashCode) != _Cache.end()) {
        Cache = _Cache[_HashCode];
        if (Cache.Depth >= NextDepth) {
            return Cache.Score;
        }
    }

    std::vector<Board::PawnInfo> Points = GeneratePoints(PawnType);
    if (CurrentDepth == 0 && Points.size() == 1) {
        _BestMove = Points.front();
        return Points.front().Score;
    }

    std::vector<Board::PawnInfo> BestPoints;
    for (const auto& Point : Points) {
        int Score = 0;
        if (Point.Score >= GetScore(PawnLayout::kFiveLink)) {
            Score = bMachineFlag ? std::numeric_limits<int>::max() - 1 : std::numeric_limits<int>::min() + 1;
        } else {
            PutPawn(Point);
            Score = Minimax(CurrentDepth + 1, NextDepth - 1, Alpha, Beta, 3 - PawnType);
            RevokePawn(Point);
        }

        if (bMachineFlag) {
            if (Score > Alpha) {
                Alpha = Score;
                if (CurrentDepth == 0) {
                    BestPoints.clear();
                }
                BestPoints.push_back(Point);
            }
        } else {
            if (Score < Beta) {
                Beta = Score;
            }
        }

        if (Alpha >= Beta) {
            break;
        }
    }

    if (CurrentDepth == 0) {
        _BestMove = BestPoints.size() > 1 ? GetBestPoint(Points) : BestPoints.front();
    }

    int Result = bMachineFlag ? Alpha : Beta;
    _Cache.insert({ _HashCode, LayoutCache(Result, NextDepth) });
    return Result;
}

Board::PawnInfo Evaluator::GetBestMove(int MaxDepth, bool bProcessCalcKill, int MaxVcxDepth, bool bIsVct, int NextDepth) {
    DeepingMinimax(2, MaxDepth);
    if (!bProcessCalcKill) {
        return _BestMove;
    } else {
        if (!HasLayoutNearPawn(_BestMove, _kFiveLink)) {
            Board::PawnInfo VcxPoint = DeepingCalcKill(NextDepth, MaxVcxDepth, bIsVct);
            if (VcxPoint.Type != 0) {
                std::cout << std::format("Calculate kill: ({}, {})", VcxPoint.Row, VcxPoint.Column) << std::endl;
                return VcxPoint;
            } else {
                return _BestMove;
            }
        } else {
            return _BestMove;
        }
    }
}

int Evaluator::Evaluate(Board::PawnInfo& Pawn) {
    int Score     = 0;
    int BlockFour = 0; // 冲四
    int Three     = 0; // 活三
    int FourThree = 0; // 冲四活三
    std::string Situation;
    for (int i = 0; i != 4; ++i) {
        Situation = GetSituation(Pawn, i);
        PawnLayout Layout = GetPawnLayout(Situation);
        if (Layout != PawnLayout::kEmpty) {
            switch (Layout) {
            case PawnLayout::kThree:
                ++Three;
                if (HasLayout(Situation, _kBlockFour)) {
                    ++FourThree;
                }
                break;
            case PawnLayout::kBlockFour:
                ++BlockFour;
                break;
            }
        }

        Score += GetScore(Layout);
    }

    if (BlockFour > 1 || FourThree > 1) {
        Score += GetScore(PawnLayout::kMultiFour);
    } else if (BlockFour > 0 && Three > 0 || FourThree > 0 && Three > 1) {
        Score += GetScore(PawnLayout::kFourThree);
    } else if (Three > 1) {
        Score += GetScore(PawnLayout::kMultiThree);
    }

    Pawn.Score = Score;

    return Score;
}

std::vector<Board::PawnInfo> Evaluator::GeneratePoints(Board::PawnType PawnType) {
    std::vector<Board::PawnInfo> KillPoints;
    std::vector<Board::PawnInfo> HighPriorityPoints;
    std::vector<Board::PawnInfo> MiddlePriorityPoints;
    std::vector<Board::PawnInfo> LowPriorityPoints;
    std::size_t MaxPointCount = 10;
    int ThreatLevel = 0;

    for (int x = 0; x != kBoardSize; ++x) {
        for (int y = 0; y != kBoardSize; ++y) {
            if (_Board->GetPawnsMap()[x][y] != Board::_kEmpty) {
                continue;
            }

            Board::PawnInfo NewPoint{ x, y, PawnType };
            int Score = Evaluate(NewPoint);
            if (Score >= GetScore(PawnLayout::kFiveLink)) {
                return { NewPoint };
            }
            if (ThreatLevel == 2) {
                continue;
            }
            if (Score >= GetScore(PawnLayout::kMiddleRisk)) {
                KillPoints.push_back(NewPoint);
            }

            Board::PawnInfo FoePoint{ x, y, 3 - PawnType };
            int FoeScore = Evaluate(FoePoint);
            int CurrentThreatLevel = 0;
            if (FoeScore >= GetScore(PawnLayout::kFiveLink)) {
                CurrentThreatLevel = 2;
            } else if (FoeScore >= GetScore(PawnLayout::kMiddleRisk)) {
                CurrentThreatLevel = 1;
            }

            if (CurrentThreatLevel > 0) {
                if (ThreatLevel < CurrentThreatLevel) {
                    ThreatLevel = CurrentThreatLevel;
                    HighPriorityPoints.clear();
                }
                HighPriorityPoints.push_back(NewPoint);
            }

            if (ThreatLevel > 0) {
                continue;
            }

            if (ScoreBetween(Score,    PawnLayout::kLowRisk, PawnLayout::kMiddleRisk) ||
                ScoreBetween(FoeScore, PawnLayout::kLowRisk, PawnLayout::kMiddleRisk)) {
                HighPriorityPoints.push_back(NewPoint);
                continue;
            }

            if (HighPriorityPoints.empty()) {
                if (Score >= GetScore(PawnLayout::kBlockFour) || FoeScore >= GetScore(PawnLayout::kBlockFour)) {
                    MiddlePriorityPoints.push_back(NewPoint);
                    continue;
                }
                if (MiddlePriorityPoints.empty() && Score >= GetScore(PawnLayout::kBlockOne)) {
                    LowPriorityPoints.push_back(NewPoint);
                }
            }
        }
    }

    if (ThreatLevel < 2 && !KillPoints.empty()) {
        return KillPoints;
    }

    std::vector<Board::PawnInfo> Points;
    if (HighPriorityPoints.empty()) {
        if (MiddlePriorityPoints.empty()) {
            if (LowPriorityPoints.empty()) {
                return GenRandomPoints(1);
            }
            Points = LowPriorityPoints;
        } else {
            Points = MiddlePriorityPoints;
        }
    } else {
        Points = HighPriorityPoints;
    }

    std::sort(Points.begin(), Points.end(),
        [this](const Board::PawnInfo& Point1, const Board::PawnInfo& Point2) -> bool {
            return Point1.Score > Point2.Score;
        }
    );

    return std::vector<Board::PawnInfo>(Points.begin(), Points.begin() + std::min(MaxPointCount, Points.size()));
}

std::vector<Board::PawnInfo> Evaluator::FindVcxPoints(Board::PawnType PawnType, bool bIsVct) {
    std::vector<Board::PawnInfo> AttackPoints;
    std::vector<Board::PawnInfo> DefensePoints;
    std::vector<Board::PawnInfo> VcxPoints;
    bool bMachineFlag = PawnType == _MachinePawn;
    bool bHasThreat   = false;
    for (int x = 0; x != kBoardSize; ++x) {
        for (int y = 0; y != kBoardSize; ++y) {
            if (_Board->GetPawnsMap()[x][y] != Board::_kEmpty) {
                continue;
            }

            Board::PawnInfo NewPoint{ x, y, PawnType };
            int Score = Evaluate(NewPoint);
            if (Score >= GetScore(PawnLayout::kFiveLink)) {
                return { NewPoint };
            }
            if (bHasThreat) {
                continue;
            }

            Board::PawnInfo FoePoint{ x, y, 3 - PawnType };
            int FoeScore = Evaluate(FoePoint);
            if (FoeScore >= GetScore(PawnLayout::kFiveLink)) {
                bHasThreat = true;
                DefensePoints.clear();
                DefensePoints.push_back(NewPoint);
                continue;
            }

            if (Score >= GetScore(PawnLayout::kMiddleRisk)) {
                AttackPoints.push_back(NewPoint);
                continue;
            }

            if (bMachineFlag) {
                if (HasLayoutNearPawn(NewPoint, _kBlockFour)) {
                    VcxPoints.push_back(NewPoint);
                } else if (bIsVct && HasLayoutNearPawn(NewPoint, _kThree)) {
                    VcxPoints.push_back(NewPoint);
                }
            } else {
                if (bIsVct) {
                    if (HasLayoutNearPawn(NewPoint, _kBlockFour) || FoeScore >= GetScore(PawnLayout::kFour)) {
                        DefensePoints.push_back(NewPoint);
                    }
                }
            }
        }
    }

    std::vector<Board::PawnInfo> Points;
    if (!bHasThreat) {
        if (!AttackPoints.empty()) {
            std::sort(AttackPoints.begin(), AttackPoints.end(),
                [this](const Board::PawnInfo& Point1, const Board::PawnInfo& Point2) -> bool {
                    return Point1.Score > Point2.Score;
                }
            );
            if (bMachineFlag) {
                return AttackPoints;
            }

            Points.insert(Points.end(), AttackPoints.begin(), AttackPoints.end());
        }

        if (!VcxPoints.empty()) {
            Points.insert(Points.end(), VcxPoints.begin(), VcxPoints.end());
        }
    }

    if (!DefensePoints.empty()) {
        if (bMachineFlag) {
            Points.insert(Points.end(), DefensePoints.begin(), DefensePoints.end());
        } else {
            Points.insert(Points.begin(), DefensePoints.begin(), DefensePoints.end());
        }
    }

    return Points;
}

std::string Evaluator::GetSituation(const Board::PawnInfo& Pawn, int Direction) {
    std::string Line;
    for (int Offset = -4; Offset <= 4; ++Offset) {
        if (Offset == 0) {
            Line.push_back('X');
        } else {
            Line.push_back(GetPawn(Pawn, Direction, Offset));
        }
    }
    return Line;
}

char Evaluator::GetPawn(const Board::PawnInfo& Pawn, int Direction, int Offset) {
    int Row    = Pawn.Row;
    int Column = Pawn.Column;

    switch (Direction) {
    case 0: // vertical
        Row    += Offset;
        break;
    case 1: // horizonal
        Column += Offset;
        break;
    case 2: // backslash
        Row    += Offset;
        Column += Offset;
        break;
    case 3: // slash
        Row    += Offset;
        Column -= Offset;
        break;
    }

    if (Row < 0 || Column < 0 || Row >= kBoardSize || Column >= kBoardSize) {
        return '-';
    }

    int CurrentPawn = _Board->GetPawnsMap()[Row][Column];

    if (CurrentPawn == Board::_kEmpty) {
        return '_';
    }
    if (CurrentPawn == Pawn.Type) {
        return 'X';
    } else {
        return '#';
    }
}

Evaluator::PawnLayout Evaluator::GetPawnLayout(const std::string_view Str) {
    for (const auto& ScorePair : _ScoreMap) {
        if (HasLayout(Str, ScorePair.first)) {
            return ScorePair.second;
        }
    }

    return PawnLayout::kEmpty;
}

bool Evaluator::HasLayout(const std::string_view Str, const std::vector<std::string>& Layout) {
    for (const auto& Pattern : Layout) {
        if (Str.contains(Pattern)) {
            return true;
        }
    }
    return false;
}

bool Evaluator::HasLayoutNearPawn(const Board::PawnInfo& Pawn, const std::vector<std::string>& Layout) {
    for (int i = 0; i != 4; ++i) {
        if (HasLayout(GetSituation(Pawn, i), Layout)) {
            return true;
        }
    }
    return false;
}

int Evaluator::EvalBoard() {
    int HumanScore   = 0;
    int MachineScore = 0;
    for (int x = 0; x != kBoardSize; ++x) {
        for (int y = 0; y != kBoardSize; ++y) {
            if (_Board->GetPawnsMap()[x][y] == Board::_kEmpty) {
                continue;
            }
            Board::PawnType CurrentType = _Board->GetPawnsMap()[x][y];
            bool bMachineFlag = false;
            if (CurrentType == _MachinePawn) {
                bMachineFlag = true;
            }
            Board::PawnInfo Pawn{ x, y, CurrentType };
            int Score = Evaluate(Pawn);
            if (bMachineFlag) {
                MachineScore += Score;
            } else {
                HumanScore   += Score;
            }
        }
    }

    return MachineScore * _Aggressiveness - HumanScore;
}

Board::PawnInfo Evaluator::CalcVcxKill(int NextDepth, bool bIsVct, Board::PawnType PawnType) {
    if (NextDepth == 0) {
        return {};
    }

    bool bMachineFlag = PawnType == _MachinePawn;

    LayoutCache Cache;
    if (_HashCode != 0 && _Cache.find(_HashCode) != _Cache.end()) {
        Cache = _Cache[_HashCode];
        if (Cache.Depth >= NextDepth) {
            return Cache.VcxPoint;
        }
    }

    Board::PawnInfo BestVcxPawn{};
    std::vector<Board::PawnInfo> Points = FindVcxPoints(PawnType, bIsVct);
    for (const auto& Point : Points) {
        if (Point.Score >= GetScore(PawnLayout::kHighRisk)) {
            return bMachineFlag ? Point : Board::PawnInfo{};
        }

        _Board->PutPawn(Point, true, false);
        BestVcxPawn = CalcVcxKill(NextDepth - 1, bIsVct, 3 - PawnType);
        _Board->PutPawn({ Point.Row, Point.Column, Board::_kEmpty }, true, false);

        if (BestVcxPawn.Type == Board::_kEmpty) {
            if (bMachineFlag) {
                continue;
            }

            return {};
        }

        BestVcxPawn = Point;
        if (bMachineFlag) {
            break;
        }
    }

    _Cache.insert({ _HashCode, LayoutCache(BestVcxPawn, NextDepth) });

    return BestVcxPawn;
}

Board::PawnInfo Evaluator::GetBestPoint(std::vector<Board::PawnInfo>& Points) {
    Board::PawnInfo BestPoint{};
    int BestScore = std::numeric_limits<int>::min();

    for (auto& Point : Points) {
        Board::PawnInfo FoePoint = { Point.Row, Point.Column, 3 - Point.Type };
        int Score = std::round(Evaluate(Point) * _Aggressiveness) + Evaluate(FoePoint);
        if (Score > BestScore) {
            BestScore = Score;
            BestPoint = Point;
        }
    }

    return BestPoint;
}

std::vector<Board::PawnInfo> Evaluator::GenRandomPoints(std::size_t Amount) {
    std::vector<Board::PawnInfo> Points;
    for (int x = 0; x != kBoardSize; ++x) {
        for (int y = 0; y != kBoardSize; ++y) {
            if (_Board->GetPawnsMap()[x][y] == 0) {
                Points.push_back({ x, y, _MachinePawn });
            }
        }
    }

    std::mt19937 Engine(std::random_device{}());
    std::shuffle(Points.begin(), Points.end(), Engine);

    return std::vector<Board::PawnInfo>(Points.begin(), Points.begin() + std::min(Amount, Points.size()));
}

Board::PawnInfo Evaluator::DeepingCalcKill(int NextDepth, int MaxDepth, bool bIsVct) {
    Board::PawnInfo VcxPoint{};
    while (NextDepth <= MaxDepth) {
        VcxPoint = CalcVcxKill(NextDepth, bIsVct, _MachinePawn);
        if (VcxPoint.Type != Board::_kEmpty) {
            break;
        }

        NextDepth += 2;
    }

    return VcxPoint;
}

void Evaluator::DeepingMinimax(int NextDepth, int MaxDepth) {
    while (NextDepth <= MaxDepth) {
        int Score = Minimax(0, NextDepth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), _MachinePawn);
        if (std::abs(Score) >= GetScore(PawnLayout::kFiveLink)) {
            break;
        }

        NextDepth += 2;
    }
}
