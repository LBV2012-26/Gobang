#pragma once

#include <algorithm>
#include <cstdlib>
#include <chrono>
#include <limits>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include <QtWidgets>
#include <QApplication>
#include <QChar>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPixmap>
#include <QPoint>
#include <QSize>
#include <QString>

#ifdef _DEBUG
#include <QDebug>
#endif // _DEBUG

#include <Windows.h>

#include "Board.h"
#include "Evaluator.h"
#include "GameBase.h"
#include "MainWindow.h"
#include "Player.h"
