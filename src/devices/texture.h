
#ifndef __AQUA__KOS_DEVICES_TEXTURE_H
	#define __AQUA__KOS_DEVICES_TEXTURE_H
	
	#include "../external/gl/gl_common/texture.h"

	texture_t create_texture_from_screenshot(void) {
		#if KOS_USES_JNI
			return (texture_t) CALLBACK_INT(java_create_texture_from_screenshot, kos_video_width(0), kos_video_height(0), TEXTURE_WRAP_TYPE, SHARP_TEXTURES);
		#else
			unsigned long long bpp   = 32;
			unsigned long long bytes = kos_video_width(0) * kos_video_height(0) * (bpp >> 3);
			
			unsigned char* pixels = (unsigned char*) malloc(bytes);
			glReadBuffer(GL_FRONT);
			glReadPixels(0, 0, (GLsizei) kos_video_width(0), (GLsizei) kos_video_height(0), GL_RGBA, GL_UNSIGNED_BYTE, pixels);
			
			unsigned long long* data  = (unsigned long long*) malloc(bytes);
			unsigned char*      data8 = (unsigned char*)      data;
			unsigned long long  pitch = kos_video_width(0) * (bpp / 8);
			
			int y;
			for (y = 0; y < kos_video_height(0); y++) {
				memcpy(data8 + (kos_video_height(0) - y - 1) * pitch, pixels + y * pitch, pitch);
				
			}
			
			texture_t texture = texture_create((unsigned long long) data, bpp, kos_video_width(0), kos_video_height(0));
			
			free(pixels);
			free(data);
			
			if (!texture) {
				printf("WARNING Could not create texture from screenshot\n");
				return 0;
				
			} else {
				return texture;
			}
		#endif
		
	}
	
	static void texture_device_handle(unsigned long long** result, const char* data) {
		unsigned long long* command = (unsigned long long*) data;
		
		if      (command[0] == 'c') { kos_bda_implementation.temp_value = texture_create(command[1], command[2], command[3], command[4]); *result = (unsigned long long*) &kos_bda_implementation.temp_value; }
		else if (command[0] == 'r') texture_remove(command[1]);
		
		else if (strcmp (data, "screenshot") == 0) { kos_bda_implementation.temp_value = create_texture_from_screenshot(); *result = (unsigned long long*) &kos_bda_implementation.temp_value; }
		else if (strncmp(data, "sharp ", 6)  == 0) SHARP_TEXTURES = (unsigned long long) atoi(data + 6);
		else KOS_DEVICE_COMMAND_WARNING("texture")
		
	}
	
#endif
