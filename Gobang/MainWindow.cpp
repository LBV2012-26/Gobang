#include "MainWindow.h"

#include <memory>
#include <QPainter>
#include <QPixmap>
#include <QSize>

#ifdef _DEBUG
#include <QDebug>
#endif // _DEBUG

MainWindow::MainWindow(std::shared_ptr<Board> Board, QWidget* Parent) : _Board(Board), QWidget(Parent), _bFirstDraw(true)
{
    _MainUi.setupUi(this);
    setMaximumSize(700, 700);
    setMinimumSize(700, 700);
    setMouseTracking(true);
    _MainUi.Label_Board->setMouseTracking(true);

    connect(_Board.get(), &Board::Signal_PaintEvent, this, &MainWindow::Slot_PaintEvent);

    SetupAssets();
}

void MainWindow::SetupAssets() {
    _BoardImage.load(":/Gobang/Resources/Textures/Board.jpg");
    _BlackPawnImage.load(":/Gobang/Resources/Textures/BlackPawn.png");
    _WhitePawnImage.load(":/Gobang/Resources/Textures/WhitePawn.png");

    _BoardImage = _BoardImage.scaled(_MainUi.Label_Board->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    _MainUi.Label_Board->setPixmap(QPixmap::fromImage(_BoardImage));
}

void MainWindow::mouseMoveEvent(QMouseEvent* Event) {
    Q_EMIT Signal_MouseEvent(Event);
}

void MainWindow::mousePressEvent(QMouseEvent* Event) {
    Q_EMIT Signal_MouseEvent(Event);
}

void MainWindow::paintEvent(QPaintEvent* Event) {
    if (_bFirstDraw) {
        return;
    }

    QImage PawnImage = (_Pawn.Type == Board::_kBlack ? _BlackPawnImage : _WhitePawnImage);
    _BoardImage = OverlayImage(_BoardImage, PawnImage);
    _MainUi.Label_Board->setPixmap(QPixmap::fromImage(_BoardImage));
}

QImage MainWindow::OverlayImage(const QImage& SrcImage, const QImage& FullImage) {
    QImage Image(SrcImage);
    QImage TempImage = FullImage.scaled(kPawnSize, kPawnSize, Qt::KeepAspectRatio);
    QPoint Coord     = QPoint(_Pawn.Column * kGridSize + kMargin - kPawnSize / 2,
                              _Pawn.Row    * kGridSize + kMargin - kPawnSize / 2);
    QPainter Painter(&Image);
    Painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    Painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    Painter.drawImage(Coord, TempImage);
    Painter.end();

    return Image;
}

void MainWindow::Slot_PaintEvent(const Board::PawnInfo& Pawn) {
    _bFirstDraw = false;
    _Pawn       = Pawn;
    repaint();
    _bFirstDraw = true;
}
