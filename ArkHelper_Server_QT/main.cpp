#include "ArkHelper_Server_QT_MainWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ArkHelper_Server_QT_MainWindow w;
    w.show();
    return a.exec();
}
