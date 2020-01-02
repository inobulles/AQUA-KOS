
// include all platform specific external stuff

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <GL/gl.h>

// include all platform specific kos stuff

static SDL_Window* sdl_window;
static SDL_GLContext sdl_context;

#include "desktop/video.h"
#include "desktop/events.h"

// platform specific kos functions

void load_platform_kos(void) {
	// attributes
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,      8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,    8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,     8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,    8);
	
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,  1);
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	
	if (KOS_MSAA > 1) {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, KOS_MSAA);
	}
	
	if (KOS_VSYNC && SDL_GL_SetSwapInterval(-1) < 0) {
		SDL_GL_SetSwapInterval(1);
	}
	
	// window creation
	
	assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
	assert(sdl_window = SDL_CreateWindow("AQUA", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, KOS_VIDEO_WIDTH, KOS_VIDEO_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE));
	sdl_context = SDL_GL_CreateContext(sdl_window);
	
	// set common variables
	
	window_based_mouse = 1;
	
	video_width  = KOS_VIDEO_WIDTH;
	video_height = KOS_VIDEO_HEIGHT;
}

void quit_platform_kos(void) {
	SDL_Quit();
}