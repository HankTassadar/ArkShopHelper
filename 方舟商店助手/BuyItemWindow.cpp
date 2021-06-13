#include "BuyItemWindow.h"

BuyItemWindow::BuyItemWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setWindowFlags(Qt::WindowFlags::enum_type::WindowStaysOnTopHint | Qt::WindowFlags::enum_type::FramelessWindowHint);
	connect(ui.buyButton, &QPushButton::clicked, this, &BuyItemWindow::buyButtonCallback);
	connect(ui.cancelButton, &QPushButton::clicked, this, &BuyItemWindow::cancelButtonCallback);
}

BuyItemWindow::~BuyItemWindow()
{
}

void BuyItemWindow::setItemString(QString name)
{
	this->move(100, 100);
	this->name = name;
	ui.itemLabel->setText(this->name);
}

void BuyItemWindow::setMode(MODE mode)
{
	this->mode = mode;
}

void BuyItemWindow::buyButtonCallback()
{
	auto id = this->name.split(QChar::fromLatin1(' '))[0].toStdString();
	auto win = ArkWindow::getInstance();
	switch (this->mode)
	{
	case BuyItemWindow::MODE::BUY:
		win->sendMsgToArk(ArkWindow::MsgType::SHOP_BUY, id, 1);
		break;
	case BuyItemWindow::MODE::KIT:
		win->sendMsgToArk(ArkWindow::MsgType::SHOP_KIT, id, 1);
		break;
	case BuyItemWindow::MODE::SELL:
		win->sendMsgToArk(ArkWindow::MsgType::SHOP_SELL, id, 1);
		break;
	default:
		break;
	}
	
	this->hide();
}

void BuyItemWindow::cancelButtonCallback()
{
	this->hide();
}
