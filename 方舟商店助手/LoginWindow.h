#pragma once

#include <QWidget>
#include "ui_LoginWindow.h"

class LoginWindow : public QWidget
{
	Q_OBJECT

public:
	LoginWindow(QWidget* parent = Q_NULLPTR);
	~LoginWindow();

	void clearLoginStateText() {
		ui.stateLabel->setText("");
	}

	/**
	* 用于捕捉更新用户数据按钮
	*/
	void catchButton(QPushButton* button) {
		this->_Button = button;
	}
private:
	void loginButton();
private:
	Ui::LoginWindow ui;
	QPushButton* _Button;
};
