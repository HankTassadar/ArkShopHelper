#include "SetQQWindow.h"
#include<ArkWindowOperate.h>

SetQQWindow::SetQQWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setWindowFlags(Qt::WindowFlags::enum_type::WindowStaysOnTopHint);
	connect(ui.pushButton, &QPushButton::clicked, this, &SetQQWindow::pushButtonCallback);
}

SetQQWindow::~SetQQWindow()
{
}

void SetQQWindow::pushButtonCallback()
{
	auto instance = ArkWindow::getInstance();
	auto data = this->ui.lineEdit->text();
	auto cmd = instance->getCmd(ArkWindow::MsgType::SETQQ);
	std::string fullcmd = cmd + " " + data.toStdString();
	this->ui.lineEdit_2->setText(QString::fromUtf8(fullcmd.c_str(), fullcmd.size()));
	instance->sendMsgToArk(ArkWindow::MsgType::SETQQ, data.toStdString());
}