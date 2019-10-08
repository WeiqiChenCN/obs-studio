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

#include <iostream>

static void render_window(void* data, uint32_t cx, uint32_t cy)
{
	std::cerr << __FUNCTION__;
	obs_render_main_texture();

	UNUSED_PARAMETER(data);
	UNUSED_PARAMETER(cx);
	UNUSED_PARAMETER(cy);
}

static void init_global() {
	if (!obs_startup("en-US", nullptr, nullptr))
		throw "Couldn't create OBS";

	struct obs_video_info ovi;
	ovi.adapter = 0;
	ovi.gpu_conversion = false;
	ovi.base_width = 1280;
	ovi.base_height = 720;
	ovi.fps_num = 30;
	ovi.fps_den = 1;
	ovi.graphics_module = "libobs-d3d11";
	ovi.output_format = VIDEO_FORMAT_RGBA;
	ovi.output_width = 1280;
	ovi.output_height = 720;


	if (obs_reset_video(&ovi) != 0)
		throw "Couldn't initialize video";

	obs_load_all_modules();
}

static void add_source(obs_source_t* source, obs_scene_t* scene)
{
	obs_sceneitem_t* sceneitem;
	sceneitem = obs_scene_add(scene, source);
	obs_sceneitem_set_visible(sceneitem, true);
	struct vec2 scale;
	vec2_set(&scale, 20.0f, 20.0f);
	obs_sceneitem_set_scale(sceneitem, &scale);
};

weiqi_obs::weiqi_obs(QWidget* parent) :QWidget(parent) {
	this->setMinimumSize(800, 600);

	auto layout = new QVBoxLayout(this);
	this->setLayout(layout);

	scene = obs_scene_create("Weiqi's Scene");
	//A scene is some kide of source.
	//source = obs_scene_get_source(scene);

	window_capture_settings = obs_data_create_from_json(
R"(
{
  "window":"64bit:*:explorer.exe",
  "compatibility": true,
  "cursor": true
}
)"
	);
#if 1
	source = obs_source_create("window_capture", "Weiqi's Window Capture", window_capture_settings, nullptr);
	if (!source) {
		throw "obs_source_create window_capture failure...";
	}
#else
	/* create source */
	source = obs_source_create(
		"random", "some randon source", NULL, nullptr);
	if (!source)
		throw "Couldn't create random test source";
#endif
#if 0
	/* create filter */
	OBSSource filter = obs_source_create(
		"test_filter", "a nice green filter", NULL, nullptr);
	if (!filter)
		throw "Couldn't create test filter";
	obs_source_filter_add(source, filter);
#endif
#if 0
	obs_enter_graphics();
	obs_scene_atomic_update(scene, (obs_scene_atomic_update_func)add_source, source);
	obs_leave_graphics();
#else
	obs_sceneitem_t* item = NULL;
	item = obs_scene_add(scene, source);
	struct vec2 vec2;
	vec2_set(&vec2, 20.0f, 20.0f);
	obs_sceneitem_set_scale(item, &vec2);
	vec2_set(&vec2, 0, 0);
	obs_sceneitem_set_pos(item, &vec2);

#endif
#if 0
	/* set monitoring if source monitors by default */
	uint32_t flags = obs_source_get_output_flags(source);
	if ((flags & OBS_SOURCE_MONITOR_BY_DEFAULT) != 0) {
		obs_source_set_monitoring_type(
			source,
			OBS_MONITORING_TYPE_MONITOR_ONLY);
	}
#endif
#if 0
	auto properties = obs_source_properties(source);
	auto property = obs_properties_first(properties);
	while (property) {
		auto name = obs_property_name(property);
		qDebug() << "property:" << name;
		obs_property_next(&property);
	};
	obs_properties_destroy(properties);
#endif

	scene_source = obs_scene_get_source(scene);
	obs_set_output_source(0, scene_source);
	obs_source_inc_showing(scene_source);
	obs_source_inc_showing(source);
	
	auto display = new OBSQTDisplay(this);
	layout->addWidget(display);
	obs_display_set_enabled(display->GetDisplay(), true);
	obs_display_add_draw_callback(display->GetDisplay(), render_window, nullptr);

};
weiqi_obs::~weiqi_obs() {
}

void weiqi_obs::run() {
	
}



int main(int argc, char* argv[])
{
    QApplication qapp(argc, argv);
    init_global();
    weiqi_obs obs(nullptr);
    obs.show();
    return qapp.exec();
}
