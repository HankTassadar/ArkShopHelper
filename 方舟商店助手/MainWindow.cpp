#include "MainWindow.h"
#include<ArkHelperUserData.h>
#include<ServiceQuery.hpp>
#include<qmessagebox.h>
#include<direct.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _ifmax(true)
    , _client(nullptr)
    , _onesecTimer(new QTimer())
    , _ItemModel(new QStandardItemModel(this))
    , _KitItemModel(new QStandardItemModel(this))
    , shoppageReady(false)
    , kitpageReady(false)
{


    //网络连接客户端客户端单例初始化
#ifdef DEBUG
    this->_client = ArkHelper::ArkHelperServerAndClient::GetInstance(16338, "127.0.0.1");
#elif RELEASE
        //从从服务注册查询服务器中查询当前服务的IP地址与端口
    ServiceQuery query({ "49.232.218.70",35996,uv::SocketAddr::Ipv4 }, 10005);
    auto ipPort = query.query("ArkHelperService");
    if (ipPort.second == 0) {
        QMessageBox msg;
        msg.setText(QString::fromStdWString(L"无法链接到服务器！\n详情咨询服务器管理员"));
        msg.exec();
        exit(0);
    }
    this->_serverIP = ipPort.first;
    this->_client = ArkHelper::ArkHelperServerAndClient::GetInstance(ipPort.second, ipPort.first);
#endif // DEBUG

    
    this->_client->waitForConnected();
    //从服务端获得商店数据
    this->_client->getShopData();
    this->shopdata = this->_client->showShopData();

    //Ark操作单例的初始化并给予命令
    std::string buycmd = shopdata["Messages"]["BuyCmd"].get<std::string>();
    std::string kitcmd = shopdata["Messages"]["KitCmd"].get<std::string>();
    std::string sellcmd = shopdata["Messages"]["SellCmd"].get<std::string>();
    std::string bdqq = shopdata["Messages"]["SetQQCmd"].get<std::string>();
    std::string setpass = shopdata["Messages"]["SetPassCmd"].get<std::string>();
    std::string kill = shopdata["Messages"]["KillCmd"].get<std::string>();
    auto arkwindow = ArkWindow::getInstance();
    arkwindow->setCmdString(buycmd, kitcmd, sellcmd, bdqq, setpass, kill);

    //主窗口的创建与初始化
    ui->setupUi(this);
    this->createUI();
    this->connectInit();
    this->setWindowFlags(Qt::WindowFlags::enum_type::WindowStaysOnTopHint | Qt::WindowFlags::enum_type::FramelessWindowHint);
    this->move(0, 0);
    this->_onesecTimer->start(1000);
   
    auto a = RegisterHotKey(HWND(winId()), 1, NULL, VK_F2);//注册全局热键F2隐藏显示窗口

}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);
    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_HOTKEY)
    {
        switch (msg->wParam) {
        case 1:
            if (this->isMinimized()) {
                this->move(0, 0);
                this->showNormal();
            }
            else {
               
                this->showMinimized();    
            }
            break;
        default:
            break;
        }
        return true;
    }
    return false;
}

void MainWindow::createUI()
{
    this->createMainUI();
    this->createServerLinkUI();  
}

void MainWindow::createMainUI()
{
    std::vector<QString> name;
    name.push_back(QString::fromStdWString(L"首页"));
    name.push_back(QString::fromStdWString(L"商店"));
    name.push_back(QString::fromStdWString(L"礼包"));
    name.push_back(QString::fromStdWString(L"直连"));
    name.push_back(QString::fromStdWString(L"我的"));
    int h = 0;
    this->setGeometry(0, 0, 400, 600);
    for (auto& i : name) {
        auto button = new QPushButton(ui->centralwidget);
        button->setObjectName(i);
        h += 31;
        button->setGeometry(QRect(0, h, 51, 31));
        button->setText(i);
        button->setStyleSheet(QString::fromUtf8("QPushButton{\n"
            "	font: 15pt \"Arial\";\n"
            "}"));
        this->_pushbutton.push_back(button);
    }
    this->_subui.push_back(ui->widget_firstpage);
    this->_subui.push_back(ui->widget_shoppage);
    this->_subui.push_back(ui->widget_kitpage);
    this->_subui.push_back(ui->widget_serverlink);
    this->_subui.push_back(ui->widget_mypage);

    for (auto& i : this->_subui) {
        // i->setParent(grid));
        i->setGeometry(50, 0, 350, 600);
        i->hide();
    }
    this->_subui[0]->show();
}

void MainWindow::createServerLinkUI()
{
    std::vector<std::pair<QString,int>> name;
    name.push_back({ QString::fromStdWString(L"创世纪2"), 16602 });
    name.push_back({ QString::fromStdWString(L"畸变"), 16612 });
    name.push_back({ QString::fromStdWString(L"水晶岛"), 16622 });
    name.push_back({ QString::fromStdWString(L"灭绝"), 16632 });
    name.push_back({ QString::fromStdWString(L"创世纪1"), 16642 });
    name.push_back({ QString::fromStdWString(L"仙境"), 16652 });
    name.push_back({ QString::fromStdWString(L"焦土"), 16662 });
    name.push_back({ QString::fromStdWString(L"中心"), 16672 });
    name.push_back({ QString::fromStdWString(L"孤岛"), 16682 });
    name.push_back({ QString::fromStdWString(L"瓦尔盖罗"), 16692 });

    int h = 50;
    for (auto& i : name) {
        auto button = new QPushButton(ui->widget_serverlink);
        button->setObjectName(i.first);
        h += 40;
        button->setGeometry(QRect(100, h, 120, 31));
        button->setText(i.first);
        button->setStyleSheet(QString::fromUtf8("QPushButton{\n"
            "	font: 15pt \"Arial\";\n"
            "}"));
        connect(button, &QPushButton::clicked, this, [=]() {
            std::string cmd = "start steam://connect/" + this->_serverIP + ":" + std::to_string(i.second);
            system(cmd.c_str());
            });
        //this->_pushbutton.push_back(button);
    }

}

void MainWindow::connectInit()
{
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::closeButton);
    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::loginButton);
    connect(this->_onesecTimer, &QTimer::timeout, this, &MainWindow::onesecTimerCallback);
    connect(ui->updateDataButton, &QPushButton::clicked, this, &MainWindow::updateDataButton);
    connect(ui->setPassButton, &QPushButton::clicked, this, &MainWindow::setPassButton);
    connect(ui->bdqqButton, &QPushButton::clicked, this, &MainWindow::setQQButton);

    for (size_t i = 0; i < this->_subui.size(); i++) {
        connect(this->_pushbutton[i], &QPushButton::clicked, this, [=] {buttonCallback(i); });
    }
}

void MainWindow::closeButton()
{
    this->closeAllSubUi();
    this->close();
}

void MainWindow::loginButton()
{
    this->_loginWindow.catchButton(ui->updateDataButton);
    this->_loginWindow.move(400, 0);
    this->_loginWindow.clearLoginStateText();
    this->_loginWindow.show();
}

void MainWindow::setPassButton()
{
    this->_setPassWindow.move(400, 210);
    this->_setPassWindow.show();
}

void MainWindow::setQQButton()
{
    this->_setQQWindow.move(400, 100);
    this->_setQQWindow.show();
}

void MainWindow::shopItemCliek(QModelIndex index)
{
    auto str = index.data().toString();
    //auto list = str.split(QChar::fromLatin1(' '));
    this->_buyWindow.setMode(BuyItemWindow::MODE::BUY);
    this->_buyWindow.setItemString(str);
    this->_buyWindow.show();
}

void MainWindow::kitItemCliek(QModelIndex index)
{
    auto str = index.data().toString();
    this->_buyWindow.setMode(BuyItemWindow::MODE::KIT);
    this->_buyWindow.setItemString(str);
    this->_buyWindow.show();
}

//更新个人信息按钮
void MainWindow::updateDataButton()
{
    if (this->_client->showConnectState() && this->_client->showLoginState()) {
        this->_client->getPlayerData();
        auto playerdata = this->_client->showPlayerData();
        this->ui->steamidLabel->setText(QString::fromStdString(playerdata.steamid));
        this->ui->steamidLabel->setStyleSheet(QString::fromUtf8("font: 12pt \"Microsoft YaHei UI\";\n"
            "background-color: rgb(0, 255, 0);"));
        this->ui->pointsLabel->setText(QString::fromStdString(std::to_string(playerdata.points)));
        this->ui->vipLabel->setText(QString::fromStdString(playerdata.group));
    }
}

//每一秒的计时器回调
void MainWindow::onesecTimerCallback()
{
    this->_timerCount++;
    auto connectstate = this->_client->showConnectState();
    auto loginstate = this->_client->showLoginState();

    if (!connectstate) {
        this->ui->steamidLabel->setText(QString::fromStdWString(L"链接断开,请尝试退出程序重新打开"));
        this->ui->linkState->setText(QString::fromStdWString(L"服务器断开，请尝试退出程序重新打开"));
        this->ui->steamidLabel->setStyleSheet(QString::fromUtf8("font: 12pt \"Microsoft YaHei UI\";\n"
            "background-color: rgb(255, 0, 0);"));
        this->ui->pointsLabel->setText(QString::fromStdString(""));
        this->ui->vipLabel->setText(QString::fromStdString(""));
    }
    
    if (this->_timerCount % 10 == 0) {

    }
    if (this->_timerCount == 3600)this->_timerCount = 0;
}

//左侧分栏按钮
void MainWindow::buttonCallback(size_t i)
{
    if (this->_subui[i]->isHidden()) {
        this->closeAllSubUi();
        this->_subui[i]->show();
    }
    switch (i)
    {
    case 0:     //首页
        break;
    case 1: {   //商店
        if (this->shoppageReady)break;
        if (!this->_client->showConnectState()) {
            ui->shopState->setText(QString::fromStdWString(L"与服务器失去链接"));
            break;
        }
        
        auto shop = shopdata["ShopItems"];
        QStringList strList;
        for (json::iterator it = shop.begin(); it != shop.end(); it++) {
            auto id = it.key();
            auto desc = it.value()["Description"].get<std::string>();
            auto point = it.value()["Price"].get<int>();
            std::string des = id + "     " + desc + "     " + std::to_string(point);
            strList.append(QString::fromStdString(des) + QString::fromStdWString(L"金币"));
        }

        for (auto& i : strList) {
            QStandardItem* item = new QStandardItem(i);
            item->setEditable(false);
            this->_ItemModel->appendRow(item);
        }

        ui->listView->setModel(this->_ItemModel);
        ui->shopState->setText(QString::fromStdWString(L"已获取最新商店数据"));
        connect(ui->listView, &QListView::clicked, this, &MainWindow::shopItemCliek);
        this->shoppageReady = true;
    }
        break;
    case 2: {   //礼包
        if (this->kitpageReady)break;
        if (!this->_client->showConnectState()) {
            ui->kitState->setText(QString::fromStdWString(L"与服务器失去链接"));
            break;
        }
        if (!this->_client->showLoginState()) {
            ui->kitState->setText(QString::fromStdWString(L"未登录steamid"));
            break;
        }
        ui->kitState->setText(QString::fromStdWString(L""));
        this->_client->getPlayerKitData();
        auto playerkitdata = this->_client->showKitData();
        auto kitdata = shopdata["Kits"];

        QStringList strList;

        for (auto i = kitdata.begin(); i != kitdata.end(); i++) {
            auto id = i.key();
            int defaultamount = i.value()["DefaultAmount"].get<int>();
            auto description = i.value()["Description"].get<std::string>();

            int amount = 0;
            if (playerkitdata.contains(id)) {
                amount = playerkitdata[id]["Amount"].get<int>();
                int l = 1;
                int k = 2;
            }
            if (amount > 0 || (defaultamount != 0 && amount != 0)) {
                strList.push_back(QString::fromStdString(id + " " + description + " " + std::to_string(amount)) + QString::fromStdWString(L"个"));
            }
        }

        this->_KitItemModel->clear();
        for (auto& i : strList) {
            QStandardItem* item = new QStandardItem(i);
            item->setEditable(false);
            this->_KitItemModel->appendRow(item);
        }

        ui->kitViewList->setModel(this->_KitItemModel);
        //ui->shopState->setText(QString::fromStdWString(L"已获取最新商店数据"));
        connect(ui->kitViewList, &QListView::clicked, this, &MainWindow::kitItemCliek);

    }
        break;
    case 3: {   //直连

    }
        break;
    case 4: {   //我的

    }
          break;
    default:
        break;
    }
}

void MainWindow::closeAllSubUi()
{
    for (auto& i : this->_subui) {
        i->hide();
    }
}