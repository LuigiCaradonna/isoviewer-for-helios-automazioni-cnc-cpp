#include "IsoViewer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    IsoViewer w;
    w.setWindowState(Qt::WindowMaximized);
    w.show();
    return a.exec();
}
