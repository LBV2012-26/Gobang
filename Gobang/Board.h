#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <QObject>

const int kMargin    = 30;
const int kGridSize  = 46;
const int kPawnSize  = 50;
const int kBoardSize = 15;
const int kTexSize   = 700;

class Board : public QObject {
    Q_OBJECT

public:
    using PawnType = int;

    struct PawnInfo {
        int      Row    = 0;
        int      Column = 0;
        PawnType Type   = 0; // empty = 0, black = 1, white = 2
        int      Score  = 0;
    };

public:
    Board();
    std::pair<PawnInfo, bool> PutPawn(const PawnInfo& Pawn, bool bIsNormalized = false, bool bDrawPawn = true);

private:
    void PawnConfirm(const PawnInfo& Pawn);

public:
    const std::vector<std::vector<int>>& GetPawnsMap() const {
        return _PawnsMap;
    }

    const std::size_t GetPawnCount() const {
        return _PawnCount;
    }

signals:
    void Signal_PaintEvent(const PawnInfo& Pawn);

public:
    static const PawnType _kEmpty;
    static const PawnType _kBlack;
    static const PawnType _kWhite;

private:
    std::vector<std::vector<int>> _PawnsMap;
    std::size_t                   _PawnCount;
};
