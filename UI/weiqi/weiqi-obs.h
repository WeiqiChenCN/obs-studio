#pragma once


#include <obs.hpp>
#include <qt-display.hpp>

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
	OBSScene scene;
	OBSSource scene_source;
	OBSData window_capture_settings;


};

