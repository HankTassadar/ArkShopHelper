#pragma once

#include <QWidget>
#include "ui_SetQQWindow.h"

class SetQQWindow : public QWidget
{
	Q_OBJECT

public:
	SetQQWindow(QWidget *parent = Q_NULLPTR);
	~SetQQWindow();
private:
	void pushButtonCallback();
private:
	Ui::SetQQWindow ui;
};
