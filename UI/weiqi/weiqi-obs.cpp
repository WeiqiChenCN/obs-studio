#include "weiqi-obs.h"

#include <obs.h>
#include <obs.hpp>

#include <util/base.h>
#include <graphics/vec2.h>
#include <media-io/audio-resampler.h>

#include <intrin.h>

#include <windows.h>


#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QtGlobal>
#include <QCheckbox>
#include <QCombobox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QStandardItem>
#include <QStandardItemModel>

#include <iostream>

#define QT_UTF8(str) QString::fromUtf8(str)
#define QT_TO_UTF8(str) str.toUtf8().constData()

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

	myLayout = new QVBoxLayout(this);
	this->setLayout(myLayout);
	display = new OBSQTDisplay(this);
	myLayout->addWidget(display);
	//connect(display, &OBSQTDisplay::DisplayResized, this, &weiqi_obs::displayResize);
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

	auto property = obs_properties_first(properties);
	std::cerr << "===============Properties===============\n";
#if 0
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
			HEAD();
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

#endif

	obs_properties_destroy(properties);

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
	//obs_source_release(source);
	//obs_scene_release(scene);

	OBSData settings = obs_source_get_settings(source);
	obs_data_release(settings);
	myLayout->addWidget(new PropertiesView(
		settings, source,
		(PropertiesReloadCallback)obs_source_properties,
		(PropertiesUpdateCallback)obs_source_update));

	

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

PropertiesView::PropertiesView(
	OBSData settings_, void* obj_,
	PropertiesReloadCallback reloadCallback, PropertiesUpdateCallback callback_)
:	settings(settings_),
	properties(nullptr, obs_properties_destroy),
	reloadCallback(reloadCallback),
	callback(callback_),
	obj(obj_)

{
	if (obj) {
		properties.reset(reloadCallback(obj));
	}

	uint32_t flags = obs_properties_get_flags(properties.get());
	deferUpdate = (flags & OBS_PROPERTIES_DEFER_UPDATE) != 0;

	reloadProperties();
}

void PropertiesView::reloadProperties()
{
#if 0
	if (widget)
		widget->deleteLater();
	widget = new QWidget();
#endif
	QFormLayout* layout = new QFormLayout;
	layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
	this->setLayout(layout);

	QSizePolicy mainPolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	//widget->setSizePolicy(policy);

	layout->setLabelAlignment(Qt::AlignRight);

	obs_property_t* property = obs_properties_first(properties.get());
	bool hasNoProperties = !property;

	while (property) {
		addProperty(property, layout);
		obs_property_next(&property);
	}

}


void PropertiesView::addProperty(
	obs_property_t* property,
	QFormLayout* layout)
{
	const char* name = obs_property_name(property);
	obs_property_type type = obs_property_get_type(property);

	if (!obs_property_visible(property))
		return;

	QLabel* label = nullptr;
	QWidget* widget = nullptr;
	bool warning = false;
	switch (type) {
	case OBS_PROPERTY_INVALID:
		return;
	case OBS_PROPERTY_BOOL:
		widget = addCheckbox(property);
		break;
	case OBS_PROPERTY_LIST:
		widget = addList(property);
		break;
	}

	if (widget && !obs_property_enabled(property))
		widget->setEnabled(false);

	if (!label && type != OBS_PROPERTY_BOOL &&
		type != OBS_PROPERTY_BUTTON && type != OBS_PROPERTY_GROUP)
		label = new QLabel(obs_property_description(property));

	if (warning && label) //TODO: select color based on background color
		label->setStyleSheet("QLabel { color: red; }");
	
	/*if (label && minSize) {
		label->setMinimumWidth(minSize);
		label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	}*/

	if (label && !obs_property_enabled(property))
		label->setEnabled(false);

	if (!widget)
		return;

	layout->addRow(label, widget);

}

QWidget* PropertiesView::addCheckbox(obs_property_t* prop)
{
	const char* name = obs_property_name(prop);
	const char* desc = obs_property_description(prop);
	bool val = obs_data_get_bool(settings, name);

	QCheckBox* checkbox = new QCheckBox(QT_UTF8(desc));
	checkbox->setCheckState(val ? Qt::Checked : Qt::Unchecked);
	return checkbox;
}

QWidget* PropertiesView::addList(obs_property_t* prop)
{
	const char* name = obs_property_name(prop);
	QComboBox* combo = new QComboBox();
	obs_combo_type type = obs_property_list_type(prop);
	obs_combo_format format = obs_property_list_format(prop);
	size_t count = obs_property_list_item_count(prop);
	int idx = -1;

	for (size_t i = 0; i < count; i++)
		addComboItem(combo, prop, format, i);

	if (type == OBS_COMBO_TYPE_EDITABLE)
		combo->setEditable(true);

	combo->setMaxVisibleItems(40);
	combo->setToolTip(QT_UTF8(obs_property_long_description(prop)));
#if 0
	string value = from_obs_data(settings, name, format);

	if (format == OBS_COMBO_FORMAT_STRING &&
		type == OBS_COMBO_TYPE_EDITABLE) {
		combo->lineEdit()->setText(QT_UTF8(value.c_str()));
	}
	else {
		idx = combo->findData(QByteArray(value.c_str()));
	}

	if (type == OBS_COMBO_TYPE_EDITABLE)
		return NewWidget(prop, combo,
			SIGNAL(editTextChanged(const QString&)));

	if (idx != -1)
		combo->setCurrentIndex(idx);

	if (obs_data_has_autoselect_value(settings, name)) {
		string autoselect =
			from_obs_data_autoselect(settings, name, format);
		int id = combo->findData(QT_UTF8(autoselect.c_str()));

		if (id != -1 && id != idx) {
			QString actual = combo->itemText(id);
			QString selected = combo->itemText(idx);
			QString combined = QTStr(
				"Basic.PropertiesWindow.AutoSelectFormat");
			combo->setItemText(idx,
				combined.arg(selected).arg(actual));
		}
	}

	QAbstractItemModel* model = combo->model();
	warning = idx != -1 &&
		model->flags(model->index(idx, 0)) == Qt::NoItemFlags;

	WidgetInfo* info = new WidgetInfo(this, prop, combo);
	connect(combo, SIGNAL(currentIndexChanged(int)), info,
		SLOT(ControlChanged()));
	children.emplace_back(info);

	/* trigger a settings update if the index was not found */
	if (idx == -1)
		info->ControlChanged();
#endif
	return combo;
}

void PropertiesView::addComboItem(QComboBox* combo, obs_property_t* prop, obs_combo_format format, size_t idx)
{
	const char* name = obs_property_list_item_name(prop, idx);
	QVariant var;

	if (format == OBS_COMBO_FORMAT_INT) {
		long long val = obs_property_list_item_int(prop, idx);
		var = QVariant::fromValue<long long>(val);

	}
	else if (format == OBS_COMBO_FORMAT_FLOAT) {
		double val = obs_property_list_item_float(prop, idx);
		var = QVariant::fromValue<double>(val);

	}
	else if (format == OBS_COMBO_FORMAT_STRING) {
		var = QByteArray(obs_property_list_item_string(prop, idx));
	}

	combo->addItem(QT_UTF8(name), var);

	if (!obs_property_list_item_disabled(prop, idx))
		return;

	int index = combo->findText(QT_UTF8(name));
	if (index < 0)
		return;

	QStandardItemModel* model =
		dynamic_cast<QStandardItemModel*>(combo->model());
	if (!model)
		return;

	QStandardItem* item = model->item(index);
	item->setFlags(Qt::NoItemFlags);
}
