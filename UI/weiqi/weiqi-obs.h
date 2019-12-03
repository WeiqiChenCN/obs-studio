#pragma once


#include <obs.hpp>
#include <qt-display.hpp>

#include <QWidget>
#include <QComboBox>
#include <QFormLayout>
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
	QWidget* view;
	QLayout* myLayout;
};

typedef obs_properties_t* (*PropertiesReloadCallback)(void* obj);
typedef void (*PropertiesUpdateCallback)(void* obj, obs_data_t* settings);

class PropertiesView : public QWidget {
	Q_OBJECT
	using properties_delete_t = decltype(&obs_properties_destroy);
	using properties_t =
		std::unique_ptr<obs_properties_t, properties_delete_t>;
public:
	PropertiesView(	OBSData settings_, void* obj_,
			PropertiesReloadCallback reloadCallback,
			PropertiesUpdateCallback callback_);
public slots:
	void reloadProperties();
private:
	void addProperty(obs_property_t* property, QFormLayout* layout);
	QWidget* addCheckbox(obs_property_t* prop);
	QWidget* addList(obs_property_t* prop);
	QWidget* newWidget(obs_property_t* prop, QWidget* widget,
		const char* signal);
	void addComboItem(QComboBox* combo, obs_property_t* prop,
		obs_combo_format format, size_t idx);
private:
	OBSData settings;
	void* obj = nullptr;
	QWidget* widget = nullptr;
	properties_t properties;
	PropertiesReloadCallback reloadCallback;
	PropertiesUpdateCallback callback = nullptr;
	bool deferUpdate;

};

