#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ArkHelper_Server_QT_MainWindow.h"

class ArkHelper_Server_QT_MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ArkHelper_Server_QT_MainWindow(QWidget *parent = Q_NULLPTR);

private:
    Ui::ArkHelper_Server_QT_MainWindowClass ui;
};
