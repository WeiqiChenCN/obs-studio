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
	
	source = obs_source_create(
		"window_capture", "Weiqi's Capture",
		nullptr, nullptr);
	if (!source) {
		throw "obs_source_create window_capture failure...";
	}
	auto properties = obs_source_properties(source);
	//obs_properties_destroy(properties);

	auto property = obs_properties_first(properties);
	std::cerr << "===============Properties===============\n";

	while (property) {
		const char* name = obs_property_name(property);
		obs_property_type type = obs_property_get_type(property);

		if (!obs_property_visible(property)) {
			obs_property_next(&property);
			continue;
		}

		auto prop = property;

#define HEAD() \
do{ \
	const char *a, *b, *c; \
	a = obs_property_name(prop); if (!a) a = "null"; \
	b = obs_property_description(prop); if (!b) b = "null"; \
	c = obs_property_long_description(prop); if(!c) c="null";\
	std::cerr << a << "," << b << " (" << c << ").\n";\
}while (0)


		switch (type) {
		case OBS_PROPERTY_INVALID:
			break;
		case OBS_PROPERTY_BOOL:
			HEAD();
			break;
		case OBS_PROPERTY_INT:
			HEAD();
			break;
		case OBS_PROPERTY_FLOAT:
			HEAD();
			break;
		case OBS_PROPERTY_TEXT:
			HEAD();
			break;
		case OBS_PROPERTY_PATH:
			HEAD();
			break;
		case OBS_PROPERTY_LIST:
			//HEAD();
			break;
		case OBS_PROPERTY_COLOR:
			HEAD();
			break;
		case OBS_PROPERTY_FONT:
			HEAD();
			break;
		case OBS_PROPERTY_BUTTON:
			HEAD();
			break;
		case OBS_PROPERTY_EDITABLE_LIST:
			HEAD();
			break;
		case OBS_PROPERTY_FRAME_RATE:
			HEAD();
			break;
		case OBS_PROPERTY_GROUP:
			HEAD();
		}


		obs_property_next(&property);
	}


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
