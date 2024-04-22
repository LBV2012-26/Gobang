#pragma once

#include <memory>
#include <QtWidgets/QWidget>
#include <QImage>
#include <QMouseEvent>
#include <QPaintEvent>

#include "Board.h"
#include "ui_MainWindow.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(std::shared_ptr<Board> Board, QWidget* Parent = nullptr);
    void SetupAssets();

protected:
    // virtual functions override
    void mouseMoveEvent(QMouseEvent* Event) override;
    void mousePressEvent(QMouseEvent* Event) override;
    void paintEvent(QPaintEvent* Event) override;

private:
    QImage OverlayImage(const QImage& SrcImage, const QImage& FullImage);

signals:
    void Signal_MouseEvent(QMouseEvent* Event);

private slots:
    void Slot_PaintEvent(const Board::PawnInfo& Pawn);

private:
    Ui::MainWindowClass    _MainUi;
    QImage                 _BoardImage;
    QImage                 _BlackPawnImage;
    QImage                 _WhitePawnImage;
    std::shared_ptr<Board> _Board;
    Board::PawnInfo        _Pawn;
    bool                   _bFirstDraw;
};
