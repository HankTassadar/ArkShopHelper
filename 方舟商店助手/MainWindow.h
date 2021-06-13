#pragma once

#include <QtWidgets/QMainWindow>

#include "ui_MainWindow.h"
#include<ArkHelperServerAndClient.h>
#include"LoginWindow.h"
#include<qtimer.h>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QModelIndex>
#include<qevent.h>
#include<Windows.h>
#include"BuyItemWindow.h"
#include"SetPassWindow.h"
#include"SetQQWindow.h"

using json = nlohmann::json;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();


protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
private:
    void createUI();
    void createMainUI();
    void createServerLinkUI();
    void connectInit();
private:
    //button callback
    void closeButton();
    void loginButton();
    void setPassButton();
    void setQQButton();
    void shopItemCliek(QModelIndex index);
    void kitItemCliek(QModelIndex index);
    void updateDataButton();
private:
    void onesecTimerCallback();
private:
    //shop item callback
    void buttonCallback(size_t);
private:
    void closeAllSubUi();
private:
    Ui::MainWindow* ui;
    std::vector<QWidget*> _subui;
    bool _ifmax;
    std::vector<QPushButton*> _pushbutton;
    ArkHelper::ArkHelperServerAndClient* _client;
    LoginWindow _loginWindow;
    BuyItemWindow _buyWindow;
    SetQQWindow _setQQWindow;
    SetPassWindow _setPassWindow;

    QStringListModel* _Model;
    QStandardItemModel* _ItemModel;

    QStringListModel* _KitModel;
    QStandardItemModel* _KitItemModel;
private:
    QTimer* _onesecTimer;
    int _timerCount;
    std::string _serverIP;
    json shopdata;
private:
    bool shoppageReady;
    bool kitpageReady;
private:
    HWND _hwnd;
};
