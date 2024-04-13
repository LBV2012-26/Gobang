#include "Evaluator.h"
#include <algorithm>
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
    _FiveLink({ "XXXXX" }), // 连五
    _Four({ "_XXXX_" }), // 活四
    _BlockFour({ "X_XXX", "XX_XX", "XXX_X", "#XXXX_", "#XXX_X", "#XX_XX", "#X_XXX", "_XXXX#", "X_XXX#", "XX_XX#", "XXX_X#" }), // 冲四
    _Three({ "_XXX__", "_XX_X_", "_X_XX_", "__XXX_" }), // 活三
    _BlockThree({ "#XXX__", "#XX_X_", "#X_XX_", "__XXX#", "_X_XX#", "_XX_X#" }), // 眠三
    _Two({ "__XX__", "_XX___", "___XX_", "_X_X__", "__X_X_" }), // 活二
    _BlockTwo({ "_XX#__", "__XX#_", "__#XX_", "_#XX__", "__X_X_", "_X_X__" }), // 眠二
    _One({ "__X__", "_X___", "___X_" }), // 活一
    _BlockOne({ "__X#__", "__#X__", "_#_X__", "___#X_", "___X#_", "__X_#_", "#X____", "____X#" }) // 眠一
{
    _ScoreMap[_FiveLink]   = PawnLayout::kFiveLink;
    _ScoreMap[_Four]       = PawnLayout::kFour;
    _ScoreMap[_BlockFour]  = PawnLayout::kBlockFour;
    _ScoreMap[_Three]      = PawnLayout::kThree;
    _ScoreMap[_BlockThree] = PawnLayout::kBlockThree;
    _ScoreMap[_Two]        = PawnLayout::kTwo;
    _ScoreMap[_BlockTwo]   = PawnLayout::kBlockTwo;
    _ScoreMap[_One]        = PawnLayout::kOne;
    _ScoreMap[_BlockOne]   = PawnLayout::kBlockOne;
}

bool Evaluator::IsGameOver() {
    return false;
}

int Evaluator::Minimax(int CurrentDepth, int NextDepth, int Alpha, int Beta, bool bMachineFlag, Board::PawnType PawnType) {
    if (NextDepth == 0) { // 到达叶子节点，不需要继续往深搜索了，开始评估局面分数
        return EvalBoard();
    }

    std::vector<Board::PawnInfo> Points = GenBestPoint(PawnType);
    if (CurrentDepth == 0 && Points.size() == 1) {
        return Points.front().Score;
    }

    for (const auto& Point : Points) {
        int Score = 0;
        if (Point.Score >= static_cast<int>(_ScoreMap[_FiveLink])) {
            // 最大值减一和最小值加一是为了防止剪枝失败
            Score = bMachineFlag ? std::numeric_limits<int>::max() - 1 : std::numeric_limits<int>::min() + 1;
        } else {
            _Board->PutPawn(Point, true, false); // 落子（最后一个参数为 false 代表不在棋盘上绘制）
            Score = Minimax(CurrentDepth + 1, NextDepth - 1, Alpha, Beta, !bMachineFlag, 3 - PawnType); // 向下一层递归搜索
            _Board->PutPawn({ Point.Row, Point.Column, Board::_kEmpty }, true, false); // 撤回落子（直接调用落子函数落一个空位下去即可）
        }

        if (bMachineFlag) {
            if (Score > Alpha) {
                Alpha = Score;
                if (CurrentDepth == 0) { // 在根节点更新最佳落子点位
                    _BestMove = { Point.Row, Point.Column, PawnType };
                }
            }
        } else {
            if (Score < Beta) {
                Beta = Score;
            }
        }

        if (Alpha >= Beta) { // α-β 剪枝
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
        Score += GetScore(Situation);
        if (HasLayout(Situation, _Three)) {
            ++Three;
            if (HasLayout(Situation, _BlockFour)) {
                ++FourThree;
            }
        } else if (HasLayout(Situation, _BlockFour)) {
            ++BlockFour;
        }
    }

    if (BlockFour > 1 || FourThree > 1) {
        Score += static_cast<int>(EdgeScore::kHigh);
    }
    if (BlockFour > 0 && Three > 0 || FourThree > 0 && Three > 1) {
        Score += static_cast<int>(EdgeScore::kMiddle);
    }
    if (Three > 1) {
        Score += static_cast<int>(EdgeScore::kLow);
    }

    Pawn.Score = Score;

    return Score;
}

std::vector<Board::PawnInfo> Evaluator::GenBestPoint(Board::PawnType PawnType) {
    std::vector<Board::PawnInfo> KillPoints; // 杀棋队列
    std::vector<Board::PawnInfo> HighPriorityPoints;
    std::vector<Board::PawnInfo> MiddlePriorityPoints;
    std::vector<Board::PawnInfo> LowPriorityPoints;
    std::size_t MaxPointCount = 10;
    int DangerLevel = 0; // 危险等级，数字越高局势对电脑越不利，0 代表当前局势没什么大问题

    for (int x = 0; x != kBoardSize; ++x) {
        for (int y = 0; y != kBoardSize; ++y) {
            if (_Board->GetPawnsMap()[x][y] != Board::_kEmpty) {
                continue;
            }

            Board::PawnInfo NewPoint{ x, y, PawnType };
            int Score = Evaluate(NewPoint);
            if (Score >= static_cast<int>(_ScoreMap[_FiveLink])) { // 如果电脑可以成连五，还搜集贸，直接返回
                return { NewPoint };
            }
            if (DangerLevel == 2) { // 如果局势对电脑非常不利，跳过所有剩余判断，只搜索能成连五的落子点
                continue;
            }
            if (Score >= static_cast<int>(EdgeScore::kMiddle)) {
                KillPoints.push_back(NewPoint);
            }

            // 检查同样点位，看看如果对手落子会发生什么
            Board::PawnInfo FoePoint{ x, y, 3 - PawnType };
            int FoeScore = Evaluate(FoePoint);
            int CurrentDangerLevel = 0; // 当前局势危险等级
            if (FoeScore >= static_cast<int>(_ScoreMap[_FiveLink])) { // 对手能成连五
                CurrentDangerLevel = 2;
            } else if (FoeScore >= static_cast<int>(EdgeScore::kMiddle)) { // 对手能成活四
                CurrentDangerLevel = 1;
            }

            if (CurrentDangerLevel > 0) { // 当前局势存在危险
                if (DangerLevel < CurrentDangerLevel) { // 危险等级升级
                    DangerLevel = CurrentDangerLevel; // 更新当前危险等级
                    // 丢弃之前搜索到的所有高优先级点位，开始封堵对手
                    HighPriorityPoints.clear();
                    HighPriorityPoints.push_back(NewPoint);
                }
                // 由于危险等级升高，导致之前搜索到的高优先级点位被全部丢弃，下面的逻辑判断已经失效了，直接跳过
                continue;
            }

            if (BetweenScore(Score, EdgeScore::kLow, EdgeScore::kMiddle) || BetweenScore(FoeScore, EdgeScore::kLow, EdgeScore::kMiddle)) {
                HighPriorityPoints.push_back(NewPoint);
            }

            if (HighPriorityPoints.empty()) {
                if (Score >= static_cast<int>(_ScoreMap[_BlockFour]) || FoeScore >= static_cast<int>(_ScoreMap[_BlockFour])) {
                    MiddlePriorityPoints.push_back(NewPoint);
                    continue;
                }

                if (MiddlePriorityPoints.empty() && Score >= static_cast<int>(_ScoreMap[_BlockOne])) {
                    LowPriorityPoints.push_back(NewPoint);
                }
            }
        }
    }

    if (DangerLevel <= 1 && !KillPoints.empty()) { // 直接返回杀棋点位
        // 注意如果危险等级高于 1，杀棋将无法挽回局势，因此只判断小于等于 1 的情况
        return KillPoints;
    }

    std::vector<Board::PawnInfo> Points;
    if (HighPriorityPoints.empty()) {
        if (MiddlePriorityPoints.empty()) {
            if (LowPriorityPoints.empty()) {
                return GenRandomPoints();
            }
            if (LowPriorityPoints.size() > MaxPointCount) {
                Points = ShuffleAndSlice(LowPriorityPoints, MaxPointCount);
            } else {
                Points = LowPriorityPoints;
            }
        } else {
            if (MiddlePriorityPoints.size() > MaxPointCount) {
                Points = ShuffleAndSlice(MiddlePriorityPoints, MaxPointCount);
            } else {
                Points = MiddlePriorityPoints;
            }
        }
    } else {
        Points = HighPriorityPoints;
    }

    std::sort(Points.begin(), Points.end(),
        [this](const Board::PawnInfo& Point1, const Board::PawnInfo Point2) -> bool {
            return Point1.Score > Point2.Score;
        }
    );

    return Points;
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
    case 0: { // vertical
        Row += Offset;
        break;
    }
    case 1: { // horizonal
        Column += Offset;
        break;
    }
    case 2: { // backslash
        Row += Offset;
        Column += Offset;
        break;
    }
    case 3: { // slash
        Row += Offset;
        Column -= Offset;
    }
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
        for (const auto& Pattern : ScorePair.first) {
            if (Str.contains(Pattern)) {
                return _ScoreMap[ScorePair.first];
            }
        }
    }

    return PawnLayout::kEmpty;
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

    return MachineScore * 0.5 - HumanScore;
}
