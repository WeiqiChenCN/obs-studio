#include "weiqi-obs.h"

#include <util/base.h>
#include <graphics/vec2.h>
#include <media-io/audio-resampler.h>
#include <obs.h>
#include <obs.hpp>

#include <intrin.h>


#include <windows.h>



weiqi_obs::weiqi_obs(QWidget* parent) :QWidget(parent) {

	if (!obs_startup("en-US", nullptr, nullptr))
		throw "Couldn't create OBS";

	obs_load_all_modules();

	source = obs_source_create("");
	


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
