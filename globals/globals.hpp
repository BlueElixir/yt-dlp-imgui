#include <string>
#include <vector>
#include "../dx9_imgui/dx9_imgui.hpp"
#include "../lib/imgui/imgui.h"

struct globals_t {

	struct window_t {

		int width = 600;
		int height = 400;

		struct states_t {

			bool should_exit = false; // set to true when you want to close program

		} state;

	} window;

	struct user_interface_t {

		std::vector<std::string> url_list;
		int selected_url;

		bool add_metadata;
		bool embed_thumbnail;
		bool max_quality;
		std::string title_format = "%(title)s.%(ext)s";
		
		std::vector<std::string> file_formats = { "m4a", "mp3", "flac" };
		std::string file_format = file_formats[0];

		std::string output_folder = std::string(getenv("USERPROFILE")) + "\\Documents";

		std::string exec_string;

	} ui;

	/*struct textures_t {

		std::vector<LPDIRECT3DTEXTURE9> images;
		std::vector<std::vector<char>> files;

		struct texture_data_t {

			int width;
			int height;
			float aspect_ratio;
			std::string filename;

		}; std::vector<texture_data_t> data;

	} textures;*/

};

extern globals_t *gvars;