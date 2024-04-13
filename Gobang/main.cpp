#include <QtWidgets/QApplication>
#include "GameBase.h"

int main(int argc, char** argv) {
    QApplication App(argc, argv);
    GameBase     MainWindow;
    return       App.exec();
}
