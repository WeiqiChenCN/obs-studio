#pragma once

#include <QWidget>
#include <QMetaObject>
#include <QApplication>

class weiqi_obs : public QWidget
{
	Q_OBJECT
public:
	weiqi_obs(QWidget* parent);
	~weiqi_obs();
	void run();
protected slots:
protected:
	OBSSource source;

};

