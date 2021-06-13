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
		ui.stateLabel->setText(QString::fromStdWString(L"�������ʧȥ����"));
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
				ui.stateLabel->setText(QString::fromStdWString(L"�������"));
			}
			else if (re.second == "SteamId Not Exist!") {
				ui.stateLabel->setText(QString::fromStdWString(L"SteamId������"));
			}
			else if (re.second == "no pass") {
				QMessageBox msg;
				msg.setText(QString::fromStdWString(L"��steamidδ��������\n����Ϸ�з����� /���� 123456 ��ɵ�¼��������\n���������ֱ����Ϸ��������������"));
				msg.exec();
			}
			else if (re.second == "TimeOut") {
				ui.stateLabel->setText(QString::fromStdWString(L"���ӳ�ʱ"));
			}
		}
	}
	else {
		ui.stateLabel->setText(QString::fromStdWString(L"�ѵ�¼�������ٴε�¼"));
	}

}
