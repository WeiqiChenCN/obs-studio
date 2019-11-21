#include "weiqi-obs.h"

#include <util/base.h>
#include <graphics/vec2.h>
#include <media-io/audio-resampler.h>
#include <obs.h>
#include <obs.hpp>

#include <intrin.h>

#include <windows.h>


#include <QDebug>
#include <QTimer>
#include <QtGlobal>
#include <QVBoxLayout>

#include <iostream>

static void init_global() {
	if (!obs_startup("en-US", nullptr, nullptr))
		throw "Couldn't create OBS";
	auto width = 1920;
	auto height = 1080;
	struct obs_video_info ovi;
	ovi.adapter = 0;
	ovi.gpu_conversion = false;
	ovi.base_width = width;
	ovi.base_height = height;
	ovi.fps_num = 30;
	ovi.fps_den = 1;
	ovi.graphics_module = "libobs-d3d11";
	ovi.output_format = VIDEO_FORMAT_RGBA;
	ovi.output_width = width;
	ovi.output_height = height;


	if (obs_reset_video(&ovi) != 0)
		throw "Couldn't initialize video";

	obs_load_all_modules();
	obs_log_loaded_modules();
	obs_post_load_modules();
	obs_service_create("rtmp_common", "default_service", nullptr, nullptr);
}

static void dump_properties_list(obs_properties_t* property)
{
	//auto settings = obs_data_create();
	auto settings = obs_data_create_from_json(R"({"window":[]})");
	
	obs_properties_apply_settings(property, settings);
	obs_data_save_json(settings, "dump_propertites.txt");
	//auto window = obs_data_get_array(settings, "window");
	obs_data_release(settings);
}

static void print_properties(const char* id)
{
	auto properties = obs_get_source_properties(id);
	if (!properties) {
		std::cout<< "Failure in" << __FUNCTION__;
		throw __FUNCTION__;
	}
	dump_properties_list(properties);
#if 0
	auto property = obs_properties_first(properties);
	while (property) {
		auto name = obs_property_name(property);
		qDebug() << "property[name]:" << name;
		auto description = obs_property_description(property);
		qDebug() << "property[description]:" << description;
		auto type = obs_property_get_type(property);
		switch (type) {
		case OBS_PROPERTY_BOOL: qDebug() << "property[value]:" << obs_property_enabled(property); break;
		case OBS_PROPERTY_LIST:

			break;
		case OBS_PROPERTY_INT:; break;
		case OBS_PROPERTY_TEXT:; break;
		default: break;
		}
		obs_property_next(&property);
	};
#endif
	obs_properties_destroy(properties);


}

static void add_source(obs_source_t* source, obs_scene_t* scene)
{
	std::cerr<< "add_source."<<std::endl;
	auto sceneitem = obs_scene_add(scene, source);
	if (!sceneitem) {
		throw "obs_scene_add failure.";
	}
	obs_sceneitem_set_visible(sceneitem, true);
	//obs_sceneitem_release(sceneitem);
};

weiqi_obs::weiqi_obs(QWidget* parent) :QWidget(parent) {
	this->setMinimumSize(800, 600);

	auto layout = new QVBoxLayout(this);
	this->setLayout(layout);
	display = new OBSQTDisplay(this);
	layout->addWidget(display);
	connect(display, &OBSQTDisplay::DisplayResized, this, &weiqi_obs::displayResize);
	QMetaObject::invokeMethod(this, "start_obs");
};

weiqi_obs::~weiqi_obs() {
}


void weiqi_obs::start_obs()
{
	scene = obs_scene_create("Weiqi's Scene");
	if (!scene) {
		throw "Can't create scene!!";
	}
	
	//A scene is some kide of source.
	//source = obs_scene_get_source(scene);
	window_capture_settings = obs_get_source_defaults("window_capture");
	//window_capture_settings = obs_data_create_from_json(
	//	R"({"window":"Untitled - Notepad:Notepad:notepad.exe"})"
	//);
	obs_data_set_string(window_capture_settings, "window",
		"*Untitled - Notepad:Notepad:notepad.exe");
#if 1
	source = obs_source_create(
		"window_capture", "Weiqi's Capture",
		window_capture_settings, nullptr);
	if (!source) {
		throw "obs_source_create window_capture failure...";
	}
	//obs_data_release(window_capture_settings);

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
#if 1
	obs_enter_graphics();
	obs_scene_atomic_update(scene, (obs_scene_atomic_update_func)add_source, source);
	obs_leave_graphics();
	{
		auto _s = obs_scene_find_source(scene, "Weiqi's Capture");
		if (!_s) {
			throw "source is not been added to the scene.";
		}
	}
#else
	obs_sceneitem_t* item = obs_scene_add(scene, source);
	if (!item) {
		qDebug() << "obs_scene_add failure..";
		throw "obs_scene_add failure..";
	}
#if 0
	struct vec2 vec2;
	vec2_set(&vec2, 20.0f, 20.0f);
	obs_sceneitem_set_scale(item, &vec2);
	vec2_set(&vec2, 0, 0);
	obs_sceneitem_set_pos(item, &vec2);
	obs_sceneitem_set_alignment(item, OBS_ALIGN_CENTER);
#endif
	obs_sceneitem_set_alignment(item, OBS_ALIGN_CENTER);
	obs_sceneitem_set_visible(item, true);
	obs_transform_info t_info;
	obs_sceneitem_get_info(item, &t_info);
	t_info.bounds_type = OBS_BOUNDS_STRETCH;
	obs_sceneitem_set_info(item, &t_info);


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

	obs_display_set_enabled(display->GetDisplay(), true);
	obs_display_add_draw_callback(
		display->GetDisplay(),
		weiqi_obs::render_window,
		this);

	scene_source = obs_scene_get_source(scene);
	obs_source_inc_showing(scene_source);
	//obs_source_release(scene_source);

	//This line keep source and scene alive.
	obs_set_output_source(0, scene_source);

	//obs_source_release(scene_source);
	obs_source_release(source);
	obs_scene_release(scene);

}

void weiqi_obs::run() {
	
}

void weiqi_obs::render_window(void* data, uint32_t cx, uint32_t cy)
{
	std::cerr << __FUNCTION__<<std::endl;
	auto me = (weiqi_obs*)data;
	//obs_source_video_render(me->scene_source);
	obs_render_main_texture();
	UNUSED_PARAMETER(cx);
	UNUSED_PARAMETER(cy);
}

void weiqi_obs::displayResize()
{
	//std::cerr << "displayResize" << std::endl;
}



int main(int argc, char* argv[])
{
    QApplication qapp(argc, argv);
    init_global();
    //print_properties("window_capture");
    weiqi_obs obs(nullptr);
    obs.show();
    return qapp.exec();
}
