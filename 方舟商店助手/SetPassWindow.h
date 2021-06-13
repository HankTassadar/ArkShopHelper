#pragma once

#include <QWidget>
#include "ui_SetPassWindow.h"

class SetPassWindow : public QWidget
{
	Q_OBJECT

public:
	SetPassWindow(QWidget *parent = Q_NULLPTR);
	~SetPassWindow();

private:
	void pushButtonCallback();
private:
	Ui::SetPassWindow ui;
};
