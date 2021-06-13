#pragma once

#include <QWidget>
#include "ui_BuyItemWindow.h"
#include"ArkWindowOperate.h"
class BuyItemWindow : public QWidget
{
	Q_OBJECT

public:
	enum class MODE{
		BUY,
		KIT,
		SELL
	};
public:
	BuyItemWindow(QWidget* parent = Q_NULLPTR);
	~BuyItemWindow();

	void setItemString(QString name);

	void setMode(MODE);
private:
	void buyButtonCallback();
	void cancelButtonCallback();
private:
	QString name;
	Ui::BuyItemWindow ui;
	MODE mode;
};
