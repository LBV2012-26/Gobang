#include "Evaluator.h"
#include <algorithm>
#include <format>
#include <iostream>
#include <limits>

#ifdef _DEBUG
#include <QDebug>
#endif // _DEBUG

#ifdef min
#undef min
#endif // min
#ifdef max
#undef max
#endif // max

Evaluator::Evaluator(std::shared_ptr<Board> Board, Board::PawnType PawnType) :
    _Board(Board), _BestMove({}), _MachinePawn(PawnType),
    _kFiveLink({ "XXXXX" }), // 连五
    _kFour({ "_XXXX_" }), // 活四
    _kThree({ "_XXX__", "_XX_X_", "_X_XX_", "__XXX_" }), // 活三
    _kBlockFour({ "_XXXX", "X_XXX", "XX_XX", "XXX_X", "XXXX_"}), // 冲四
    _kBlockThree({ "#XXX__", "#XX_X_", "#X_XX_", "__XXX#", "_X_XX#", "_XX_X#" }), // 眠三
    _kTwo({ "__XX__", "_XX___", "___XX_", "_X_X__", "__X_X_" }), // 活二
    _kBlockTwo({ "_XX#__", "__XX#_", "__#XX_", "_#XX__", "___XX#", "#XX___", "XX____", "____XX" }), // 眠二
    _kOne({ "_X_#__", "__#_X_", "_#_X__", "__X_#_", "#_X___", "___X_#", "___#_X" }), // 活一
    _kBlockOne({ "__X#__", "__#X__", "___#X_", "_X#___", "#X____", "____X#" })  // 眠一
{
    _ScoreMap.push_back({ _kFiveLink,   PawnLayout::kFiveLink });
    _ScoreMap.push_back({ _kFour,       PawnLayout::kFour });
    _ScoreMap.push_back({ _kThree,      PawnLayout::kThree });
    _ScoreMap.push_back({ _kBlockFour,  PawnLayout::kBlockFour });
    _ScoreMap.push_back({ _kBlockThree, PawnLayout::kBlockThree });
    _ScoreMap.push_back({ _kTwo,        PawnLayout::kTwo });
    _ScoreMap.push_back({ _kBlockTwo,   PawnLayout::kBlockTwo });
    _ScoreMap.push_back({ _kOne,        PawnLayout::kOne });
    _ScoreMap.push_back({ _kBlockOne,   PawnLayout::kBlockOne });
}

bool Evaluator::IsGameOver() {
    return false;
}

int Evaluator::Minimax(int CurrentDepth, int NextDepth, int Alpha, int Beta, bool bMachineFlag, Board::PawnType PawnType) {
    if (NextDepth == 0) {
        return EvalBoard();
    }

    std::vector<Board::PawnInfo> Points = GeneratePoints(PawnType);
    if (CurrentDepth == 0 && Points.size() == 1) {
        _BestMove = { Points.front().Row, Points.front().Column, PawnType };
        return Points.front().Score;
    }

    for (const auto& Point : Points) {
        int Score = 0;
        if (Score >= GetScore(PawnLayout::kFiveLink)) {
            Score = bMachineFlag ? std::numeric_limits<int>::max() - 1 : std::numeric_limits<int>::min() + 1;
        } else {
            _Board->PutPawn(Point, true, false);
            Score = Minimax(CurrentDepth + 1, NextDepth - 1, Alpha, Beta, !bMachineFlag, 3 - PawnType);
            _Board->PutPawn({ Point.Row, Point.Column, Board::_kEmpty }, true, false);
        }

        if (bMachineFlag) {
            if (Score > Alpha) {
                Alpha = Score;
                if (CurrentDepth == 0) {
                    _BestMove = { Point.Row, Point.Column, PawnType };
                }
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

    return bMachineFlag ? Alpha : Beta;
}

int Evaluator::Evaluate(Board::PawnInfo& Pawn) {
    int Score     = 0;
    int BlockFour = 0; // 冲四
    int Three     = 0; // 活三
    int FourThree = 0; // 冲四活三
    QString Situation;
    for (int i = 0; i != 4; ++i) {
        Situation = GetSituation(Pawn, i);
        PawnLayout Layout = GetPawnLayout(Situation);
        switch (Layout) {
        case PawnLayout::kThree: {
            ++Three;
            if (HasLayout(Situation, _kBlockFour)) {
                ++BlockFour;
            }
            break;
        }
        case PawnLayout::kBlockFour: {
            ++BlockFour;
            break;
        }
        }

        Score += GetScore(Layout);
    }

    if (BlockFour > 1 || FourThree > 1) {
        Score += GetScore(PawnLayout::kMultiFour);
    }
    if (BlockFour > 0 && Three > 0 || FourThree > 0 && Three > 1) {
        Score += GetScore(PawnLayout::kFourThree);
    }
    if (Three > 1) {
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
                continue;
            }

            if (BetweenScore(Score,    PawnLayout::kLowRisk, PawnLayout::kMiddleRisk) ||
                BetweenScore(FoeScore, PawnLayout::kLowRisk, PawnLayout::kMiddleRisk)) {
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
                return GenRandomPoints();
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

QString Evaluator::GetSituation(const Board::PawnInfo& Pawn, int Direction) {
    QString Line;
    for (int Offset = -4; Offset <= 4; ++Offset) {
        if (Offset == 0) {
            Line.append('X');
        } else {
            Line.append(GetPawn(Pawn, Direction, Offset));
        }
    }
    return Line;
}

QChar Evaluator::GetPawn(const Board::PawnInfo& Pawn, int Direction, int Offset) {
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

Evaluator::PawnLayout Evaluator::GetPawnLayout(const QString& Str) {
    for (const auto& ScorePair : _ScoreMap) {
        if (HasLayout(Str, ScorePair.first)) {
            return ScorePair.second;
        }
    }

    return PawnLayout::kEmpty;
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
                HumanScore += Score;
            }
        }
    }

    return MachineScore * 1.8 - HumanScore;
}

bool Evaluator::HasLayout(const QString& Str, const std::vector<QString>& Layout) {
    for (const auto& Pattern : Layout) {
        if (Str.contains(Pattern)) {
            return true;
        }
    }
    return false;
}

std::vector<Board::PawnInfo> Evaluator::GenRandomPoints() {
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

    return Points;
}
