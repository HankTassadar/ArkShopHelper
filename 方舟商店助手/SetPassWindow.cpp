#include "SetPassWindow.h"
#include<ArkWindowOperate.h>
SetPassWindow::SetPassWindow(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setWindowFlags(Qt::WindowFlags::enum_type::WindowStaysOnTopHint);
	connect(ui.pushButton, &QPushButton::clicked, this, &SetPassWindow::pushButtonCallback);
}

SetPassWindow::~SetPassWindow()
{
	
}

void SetPassWindow::pushButtonCallback()
{
	auto instance = ArkWindow::getInstance();
	auto data = this->ui.lineEdit->text();
	auto cmd = instance->getCmd(ArkWindow::MsgType::SETPASS);
	std::string fullcmd = cmd + " " + data.toStdString();
	this->ui.lineEdit_2->setText(QString::fromUtf8(fullcmd.c_str(), fullcmd.size()));
	instance->sendMsgToArk(ArkWindow::MsgType::SETPASS, data.toStdString());
}
