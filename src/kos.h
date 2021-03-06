
// manage platforms

#define KOS_PLATFORM_DESKTOP  0
#define KOS_PLATFORM_BROADCOM 1
#define KOS_PLATFORM_ANDROID  2

#ifndef KOS_PLATFORM
	#error "No KOS platform specified"
#endif

#define UMAX 0xFFFFFFFFFFFFFFFF
#define PRECISION 1000000

#include "common.h"

#if   KOS_PLATFORM == KOS_PLATFORM_DESKTOP
	#include "platforms/desktop.h"
#elif KOS_PLATFORM == KOS_PLATFORM_BROADCOM
	#include "platforms/broadcom.h"
#elif KOS_PLATFORM == KOS_PLATFORM_ANDROID
	#include "platforms/android.h"
#else
	#error "KOS platform specified does not exist"
#endif

#include "functions/video.h"
#include "functions/events.h"
#include "functions/device.h"

// kos functions

void load_kos(void) {
	// load platform specific kos
	
	load_platform_kos();
	
	// setup signals
	
	doomed = 0; // not doomed... yet
	struct sigaction signal_interrupt_handler;
	signal_interrupt_handler.sa_handler = catch_signal;
	sigemptyset(&signal_interrupt_handler.sa_mask);
	sigaction(SIGINT, &signal_interrupt_handler, NULL);
	
	if (feature_video) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);
	}
	
	if (feature_devices) {
		load_devices();
	}
}

void quit_kos(void) {
	if (feature_devices) {
		quit_devices();
	}
	
	quit_platform_kos(); // quit platform specific kos
}
