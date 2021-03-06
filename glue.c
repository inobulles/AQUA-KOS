
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#include <sys/stat.h>

#include <iar.h>

iar_file_t boot_package;
char* unique = (char*) 0;
char* cwd_path = (char*) 0;

char* root_path = (char*) 0;
char* boot_path = (char*) 0;

#include "src/kos.h"

// functions left to implement

void kos_get_platform    (void) { printf("IMPLEMENT %s\n", __func__); }
void kos_platform_command(void* zvm, const char* command) { system(command); } /// REMME
void kos_get_requests    (void* zvm, const char* command) { system(command); } /// REMME
void kos_native          (void) { printf("IMPLEMENT %s\n", __func__); }

#include "zvm/zvm.h"
static zvm_program_t* de_program;

int main(int argc, char** argv) {
	root_path = KOS_DEFAULT_ROOT_PATH;
	boot_path = KOS_DEFAULT_BOOT_PATH;
	device_path = KOS_DEFAULT_DEVICES_PATH;

	video_width  = 800;
	video_height = 480;
	video_msaa   = 1;
	video_vsync  = 1;
	
	printf("[AQUA KOS] Parsing arguments ...\n");
	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "--", 2) == 0) { // argument is option
			char* option = argv[i] + 2;
			
			if (strcmp(option, "root") == 0) root_path = argv[++i];
			else if (strcmp(option, "boot") == 0) boot_path = argv[++i];
			else if (strcmp(option, "devices") == 0) device_path = argv[++i];

			else if (strcmp(option, "width" ) == 0) video_width  = atoi(argv[++i]);
			else if (strcmp(option, "height") == 0) video_height = atoi(argv[++i]);

			else if (strcmp(option, "msaa") == 0) video_msaa = atoi(argv[++i]);
			else if (strcmp(option, "no-vsync") == 0) video_vsync = 0;
			
			else {
				fprintf(stderr, "[AQUA KOS] ERROR Option '--%s' is unknown. Check README.md or go to https://github.com/inobulles/aqua-kos/blob/master/README.md to see a list of available options\n", option);
				return 1;
			}

		} else {
			fprintf(stderr, "[AQUA KOS] ERROR Unexpected argument '%s'\n", argv[i]);
			return 1;
		}
	}

	if (strcmp(root_path, "NO_ROOT") == 0) {
		root_path = (char*) 0;
	}
	
	printf("[AQUA KOS] Reading the boot package (%s) ...\n", boot_path);
	if (iar_open_read(&boot_package, boot_path)) return 1;
	
	printf("[AQUA KOS] Finding start node ...\n");
	
	iar_node_t start_node;
	if (iar_find_node(&boot_package, &start_node, "start", &boot_package.root_node) < 0) {
		fprintf(stderr, "[AQUA KOS] ERROR Failed to find start node in boot package\n");
		iar_close(&boot_package);
		return 1;
	}

	printf("[AQUA KOS] Reading start node ...\n");
	if (!start_node.data_bytes) {
		fprintf(stderr, "[AQUA KOS] ERROR Start node empty\n");
		return 1;
	}
	
	char* start_command = (char*) malloc(start_node.data_bytes);
	if (iar_read_node_content(&boot_package, &start_node, start_command)) {
		return 1;
	}
	
	if (root_path) {
		printf("[AQUA KOS] Making copy of current working directory path ...\n");
		cwd_path = getcwd((char*) 0, 0);
		
		printf("[AQUA KOS] Finding unique node ...\n");
		
		iar_node_t unique_node;
		if (iar_find_node(&boot_package, &unique_node, ZPK_UNIQUE_PATH, &boot_package.root_node) == -1) {
			printf("[AQUA KOS] WARNING Boot package doesn't contain any unique node; the data drive won't be accessible by the application\n");
			goto end_unique;
		}
		
		printf("[AQUA KOS] Reading unique node ...\n");
		if (!unique_node.data_bytes) {
			printf("[AQUA KOS] WARNING Unique node empty\n");
			goto end_unique;
		}
		
		unique = (char*) malloc(unique_node.data_bytes);
		if (iar_read_node_content(&boot_package, &unique_node, unique)) {
			free(unique);
			goto end_unique;
		}
		
		printf("[AQUA KOS] Unique is %s\n", unique);
		
		chdir(root_path);
		mkdir("data", 0700);
		chdir("data");
		mkdir(unique, 0700);
		chdir(cwd_path);
	}

end_unique:
	
	printf("[AQUA KOS] Finding feature_set node ...\n");

	iar_node_t feature_set_node;
	if (iar_find_node(&boot_package, &feature_set_node, "feature_set", &boot_package.root_node) == -1) {
		printf("[AQUA KOS] WARNING Boot package doesn't contain any feature_set node; enabling all features just to be safe ...\n");

		feature_video = 1;
		feature_devices = 1;

		goto end_feature_set;
	}

	printf("[AQUA KOS] Reading feature_set node ...\n");
	if (!feature_set_node.data_bytes) {
		goto end_feature_set;
	}

	char* feature_set = (char*) malloc(feature_set_node.data_bytes);
	if (iar_read_node_content(&boot_package, &feature_set_node, feature_set)) {
		free(feature_set);
		goto end_feature_set;
	}

	for (char* feature = strtok(feature_set, " "); feature; feature = strtok((char*) 0, " ")) {
		if (strcmp(feature, "video") == 0) feature_video = 1;
		else if (strcmp(feature, "devices") == 0) feature_devices = 1;
		else printf("[AQUA KOS] WARNING '%s' feature is unknown\n", feature);
	}

	free(feature_set);
	
end_feature_set:

	if (strncmp(start_command, "zed", 3) == 0) {
		printf("[AQUA KOS] Start command is zed, finding ROM node ...\n");
		
		iar_node_t rom_node;
		if (iar_find_node(&boot_package, &rom_node, ZPK_ROM_PATH, &boot_package.root_node) == -1) {
			fprintf(stderr, "[AQUA KOS] ERROR Failed to find ROM node (rom.zed) in boot package\n");
			return 1;
		}

		printf("[AQUA KOS] Reading rom node ...\n");
		if (!rom_node.data_bytes) {
			fprintf(stderr, "[AQUA KOS] ERROR ROM node empty\n");
			return 1;
		}
		
		void* rom = malloc(rom_node.data_bytes);
		if (iar_read_node_content(&boot_package, &rom_node, rom)) {
			return 1;
		}

		printf("[AQUA KOS] Loading the KOS ...\n");
		load_kos();

		if (root_path) {
			printf("[AQUA KOS] Changing into root directory ...\n");
			chdir(root_path);
		}

		printf("[AQUA KOS] Loading the DE ...\n");

		de_program = (zvm_program_t*) malloc(sizeof(zvm_program_t));
		memset(de_program, 0, sizeof(zvm_program_t));
		de_program->rom = rom;

		printf("[AQUA KOS] Starting run setup phase ...\n");
		if (zvm_program_run_setup_phase(de_program)) {
			fprintf(stderr, "[AQUA KOS] ERROR The ZVM's program setup phase failed\n");
			return 1;
		}
		
		while (!zvm_program_run_loop_phase(de_program));
		int error_code = de_program->error_code;
		
		zvm_program_free(de_program);
		free(rom);

		printf("[AQUA KOS] DE return code is %d\n", error_code);

		printf("[AQUA KOS] Quitting KOS ...\n");
		quit_kos();
		free(de_program);
		
		printf("[AQUA KOS] Done\n");
		return error_code;
	}

	printf("[AQUA KOS] ERROR Unknown start command '%s'\n", start_command);
	return 1;
}
