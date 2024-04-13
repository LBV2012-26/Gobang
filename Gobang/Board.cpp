#include "Board.h"

#ifdef _DEBUG
#include <QDebug>
#endif // _DEBUG

const Board::PawnType Board::_kEmpty = 0;
const Board::PawnType Board::_kBlack = 1;
const Board::PawnType Board::_kWhite = 2;

Board::Board() : _PawnsMap(kBoardSize, std::vector<int>(kBoardSize, 0)), _PawnCount(0) {}

std::pair<Board::PawnInfo, bool> Board::PutPawn(const PawnInfo& Pawn, bool bIsNormalized, bool bDrawPawn) {
    PawnInfo Final{};

    if (!bIsNormalized) {
        if (Pawn.Row < kMargin || Pawn.Column < kMargin ||
            Pawn.Row > kTexSize - kMargin || Pawn.Column > kTexSize - kMargin) {
            return { {}, false };
        }

        PawnInfo TopLeft{ (Pawn.Row - kMargin) / kGridSize,     (Pawn.Column - kMargin) / kGridSize,     Pawn.Type };
        PawnInfo TopRight{ (Pawn.Row - kMargin) / kGridSize,     (Pawn.Column - kMargin) / kGridSize + 1, Pawn.Type };
        PawnInfo BottomLeft{ (Pawn.Row - kMargin) / kGridSize + 1, (Pawn.Column - kMargin) / kGridSize,     Pawn.Type };
        PawnInfo BottomRight{ (Pawn.Row - kMargin) / kGridSize + 1, (Pawn.Column - kMargin) / kGridSize + 1, Pawn.Type };

        int LeftMod = (Pawn.Row    - kMargin) % kGridSize;
        int TopMod  = (Pawn.Column - kMargin) % kGridSize;

        double GridSizeDouble = static_cast<double>(kGridSize);

        if (LeftMod < GridSizeDouble / 2 && TopMod < GridSizeDouble / 2) {
            Final = TopLeft;
        } else if (LeftMod < GridSizeDouble / 2 && TopMod > GridSizeDouble / 2) {
            Final = TopRight;
        } else if (LeftMod > GridSizeDouble / 2 && TopMod < GridSizeDouble / 2) {
            Final = BottomLeft;
        } else if (LeftMod > GridSizeDouble / 2 && TopMod > GridSizeDouble / 2) {
            Final = BottomRight;
        }
    } else {
        Final = Pawn;
    }

    if (Pawn.Type == 0) {
        PawnConfirm(Final);
        return { {}, true };
    }

    if (_PawnsMap[Final.Row][Final.Column] == 0 &&
        _PawnsMap[Final.Row][Final.Column] != Pawn.Type) {
        if (bDrawPawn) {
            Q_EMIT Signal_PaintEvent(Final);
        }
        PawnConfirm(Final);
        return { Final, true };
    } else {
        return { {}, false };
    }
}

void Board::PawnConfirm(const PawnInfo& Pawn) {
    _PawnsMap[Pawn.Row][Pawn.Column] = Pawn.Type;
    if (Pawn.Type != 0) {
        ++_PawnCount;
    } else {
        --_PawnCount;
    }
}
