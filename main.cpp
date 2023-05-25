#include <iostream>
#include <fstream>
#include <Windows.h>
#include <thread>

#include "dx9_imgui/dx9_imgui.hpp"
#include "globals/globals.hpp"
#include "yt-dlp.h"


void create_exe() {

	std::ofstream write_exe("_ytdlp_helper_.exe", std::ios::binary);
	write_exe.write(reinterpret_cast<const char*>(ytdlp), sizeof(ytdlp));
	write_exe.close();

}


#define DEBUG
#ifdef DEBUG
int main() {
#else
int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
#endif
	
	std::thread t(create_exe);
	t.detach();

	dx9::create_window("window", "window class");
	dx9::create_device();
	gui::create_imgui();	

	while (!gvars->window.state.should_exit) {

		gui::begin_render();
		gui::render();
		gui::end_render();

	}

	gui::destroy_imgui();
	dx9::destroy_device();
	dx9::destroy_window();

	t.join();

	return EXIT_SUCCESS;
}