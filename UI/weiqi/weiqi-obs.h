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
	static void render_window(void* data, uint32_t cx, uint32_t cy);
protected slots:
	void start_obs();
	void displayResize();
protected:
	obs_scene_t* scene;
	obs_source_t* source;
	obs_source_t* scene_source;
	obs_data_t* window_capture_settings;
	OBSQTDisplay* display;
};

