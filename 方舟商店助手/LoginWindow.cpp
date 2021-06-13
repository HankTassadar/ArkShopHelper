#include "LoginWindow.h"
#include<qmessagebox.h>
#include"ArkHelperUserData.h"
#include<ArkHelperServerAndClient.h>
LoginWindow::LoginWindow(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.loginButton, &QPushButton::clicked, this, &LoginWindow::loginButton);
	auto idandpass = ArkHelperUserData::GetInstance()->ReadSteamidAndPass();
	ui.steamidEdit->setText(QString::fromStdString(idandpass.first));
	ui.passwordEdit->setText(QString::fromStdString(idandpass.second));
}

LoginWindow::~LoginWindow()
{
	
}

void LoginWindow::loginButton()
{
	auto client = ArkHelper::ArkHelperServerAndClient::GetInstance();
	if (!client->showConnectState()) {
		ui.stateLabel->setText(QString::fromStdWString(L"与服务器失去连接"));
		return;
	}
	if (!client->showLoginState()) {

		auto steamid = ui.steamidEdit->text().toStdString();
		auto pass = ui.passwordEdit->text().toStdString();
		auto re = client->login(steamid, pass);
		if (re.first) {
			this->_Button->clicked();
			ArkHelperUserData::GetInstance()->WriteSteamidAndPass(steamid, pass);
			this->close();
		}
		else {
			if (re.second == "Login Pass Error") {
				ui.stateLabel->setText(QString::fromStdWString(L"密码错误"));
			}
			else if (re.second == "SteamId Not Exist!") {
				ui.stateLabel->setText(QString::fromStdWString(L"SteamId不存在"));
			}
			else if (re.second == "no pass") {
				QMessageBox msg;
				msg.setText(QString::fromStdWString(L"该steamid未设置密码\n在游戏中发送如 /密码 123456 完成登录密码设置\n忘记密码可直接游戏中重新设置密码"));
				msg.exec();
			}
			else if (re.second == "TimeOut") {
				ui.stateLabel->setText(QString::fromStdWString(L"连接超时"));
			}
		}
	}
	else {
		ui.stateLabel->setText(QString::fromStdWString(L"已登录，无需再次登录"));
	}

}
