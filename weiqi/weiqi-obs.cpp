#include "weiqi-obs.h"

#include <util/base.h>
#include <graphics/vec2.h>
#include <media-io/audio-resampler.h>
#include <obs.h>
#include <obs.hpp>

#include <intrin.h>

#include <windows.h>


#include <QDebug>
#include <QtGlobal>
#include <QVBoxLayout>



static void add_source(obs_source_t* source, obs_scene_t* scene)
{
	obs_sceneitem_t* sceneitem;
	sceneitem = obs_scene_add(scene, source);
	obs_sceneitem_set_visible(sceneitem, true);
};

weiqi_obs::weiqi_obs(QWidget* parent) :QWidget(parent) {

	auto layout = new QVBoxLayout(this);
	this->setLayout(layout);

	if (!obs_startup("en-US", nullptr, nullptr))
		throw "Couldn't create OBS";

	obs_load_all_modules();
	scene = obs_scene_create("Weiqi's Scene");
	//A scene is some kide of source.
	//source = obs_scene_get_source(scene);
	window_capture_settings = obs_data_create_from_json(
R"(
{
  "window":"tmp:explorer.exe",
  "compatibility": true,
  "cursor": true
}
)"
	);

	source = obs_source_create("window_capture", "Weiqi's Window Capture", window_capture_settings, nullptr);
	if (!source) {
		qDebug() << "obs_source_create window_capture failure...";
		return;
	}
	obs_enter_graphics();
	obs_scene_atomic_update(scene, (obs_scene_atomic_update_func)add_source, source);
	obs_leave_graphics();
	/* set monitoring if source monitors by default */
	uint32_t flags = obs_source_get_output_flags(source);
	if ((flags & OBS_SOURCE_MONITOR_BY_DEFAULT) != 0) {
		obs_source_set_monitoring_type(
			source,
			OBS_MONITORING_TYPE_MONITOR_ONLY);
	}
	
	auto properties = obs_source_properties(source);
	auto property = obs_properties_first(properties);
	while (property) {
		auto name = obs_property_name(property);
		qDebug() << "property:" << name;
		obs_property_next(&property);
	};
	obs_properties_destroy(properties);



	
	
	


};
weiqi_obs::~weiqi_obs() {
}

void weiqi_obs::run() {
	
}



int main(int argc, char* argv[])
{
    QApplication qapp(argc, argv);
    weiqi_obs obs(nullptr);
    obs.show();
    return qapp.exec();
}
