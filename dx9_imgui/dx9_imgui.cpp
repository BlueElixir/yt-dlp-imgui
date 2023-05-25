#include "dx9_imgui.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <iostream>

#include "../lib/imgui/imgui.h"
#include "../lib/imgui/imgui_impl_dx9.h"
#include "../lib/imgui/imgui_impl_win32.h"
#include "../lib/imgui/imgui_stdlib.h"

#include "../globals/globals.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wide_parameter, LPARAM long_parameter);

long __stdcall Window_process(HWND window, UINT message, WPARAM wide_parameter, LPARAM long_parameter) {
	if (ImGui_ImplWin32_WndProcHandler(window, message, wide_parameter, long_parameter))
		return true;

	switch (message) {
	case WM_SIZE: {
		if (dx9::device && wide_parameter != SIZE_MINIMIZED) {
			dx9::present_parameters.BackBufferWidth = LOWORD(long_parameter);
			dx9::present_parameters.BackBufferHeight = HIWORD(long_parameter);
			dx9::reset_device();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wide_parameter & 0xFFF0) == SC_KEYMENU)
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		dx9::position = MAKEPOINTS(long_parameter);
	}return 0;

	case WM_MOUSEMOVE: {
		if (wide_parameter == MK_LBUTTON) {
			const auto points = MAKEPOINTS(long_parameter);
			auto rect = ::RECT{ };

			GetWindowRect(dx9::window, &rect);

			rect.left += points.x - dx9::position.x;
			rect.top += points.y - dx9::position.y;

			auto maxy = gvars->window.height * 0.05;

			if (dx9::position.x > -0 && dx9::position.x <= gvars->window.width && dx9::position.y >= 0 && dx9::position.y <= maxy) {
				SetWindowPos(dx9::window, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
			}
		}
	}
	}

	return DefWindowProcA(window, message, wide_parameter, long_parameter);
}

void dx9::create_window(const char* window_name, const char* class_name) noexcept {

	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = CS_DROPSHADOW;
	window_class.lpfnWndProc = Window_process;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = GetModuleHandleA(0);
	window_class.hIcon = 0;
	window_class.hCursor = 0;
	window_class.hbrBackground = 0;
	window_class.lpszMenuName = 0;
	window_class.lpszClassName = class_name;
	window_class.hIconSm = 0;

	RegisterClassEx(&window_class);

	window = CreateWindowEx(0, class_name, window_name, WS_POPUP, 200, 200, gvars->window.width, gvars->window.height, 0, 0, window_class.hInstance, 0);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void dx9::destroy_window() noexcept {
	DestroyWindow(window);
	UnregisterClass(window_class.lpszClassName, window_class.hInstance);
}

bool dx9::create_device() noexcept {
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&present_parameters, sizeof(present_parameters));

	present_parameters.Windowed = TRUE;
	present_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	present_parameters.BackBufferFormat = D3DFMT_UNKNOWN;
	present_parameters.EnableAutoDepthStencil = TRUE;
	present_parameters.AutoDepthStencilFormat = D3DFMT_D16;
	present_parameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &present_parameters, &device) < 0)
		return false;

	return true;
}

void dx9::reset_device() noexcept {
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&present_parameters);

if (result == D3DERR_INVALIDCALL)
IM_ASSERT(0);

ImGui_ImplDX9_CreateDeviceObjects();
}

void dx9::destroy_device() noexcept {
	if (device) {
		device->Release();
		device = nullptr;
	}

	if (d3d) {
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::create_imgui() noexcept {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	ImGui_ImplWin32_Init(dx9::window);
	ImGui_ImplDX9_Init(dx9::device);

	ImFontConfig font_config;
	font_config.FontDataOwnedByAtlas = false;
}

void gui::destroy_imgui() noexcept {
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::begin_render() noexcept {
	MSG message;

	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::end_render() noexcept {
	ImGui::EndFrame();

	dx9::device->SetRenderState(D3DRS_ZENABLE, FALSE);
	dx9::device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	dx9::device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	dx9::device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (dx9::device->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		dx9::device->EndScene();
	}

	const auto result = dx9::device->Present(0, 0, 0, 0);

	if (result == D3DERR_DEVICELOST && dx9::device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		dx9::reset_device();
}

void download() {

	std::cout << "current exec:" + gvars->ui.exec_string << std::endl;
	system(gvars->ui.exec_string.c_str());

}

void gui::render() noexcept {
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ (float)gvars->window.width, (float)gvars->window.height });

	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;
	ImDrawList* draw = ImGui::GetBackgroundDrawList();

	draw->AddRectFilled(ImVec2(0, 0), ImVec2(gvars->window.width, gvars->window.height), ImColor(20, 20, 20)); // main bg
	draw->AddRectFilled(ImVec2(0, 0), ImVec2(gvars->window.width, 20), ImColor(15, 15, 15)); // top bg

	draw->AddRect(ImVec2(0, 0), ImVec2(gvars->window.width, gvars->window.height), ImColor(50, 50, 50)); // main outline
	draw->AddRect(ImVec2(0, 0), ImVec2(gvars->window.width, 20), ImColor(50, 50, 50)); // top outline

	draw->AddText(ImVec2(5, 3), ImColor(220, 220, 220), "yt-dlp imgui implementation");
	draw->AddText(ImVec2(gvars->window.width - ImGui::CalcTextSize("x").x - 5, 3), ImColor(220, 220, 220), "x");

	ImVec2 mousepos = ImGui::GetMousePos();

	if (mousepos.x >= gvars->window.width - ImGui::CalcTextSize("x").x - 5 && mousepos.x <= gvars->window.width - 5 && mousepos.y >= 3 && mousepos.y <= 14)
		if (ImGui::IsKeyPressed(ImGuiKey_MouseLeft))
			gvars->window.state.should_exit = true;

	if (ImGui::Begin("welcome", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse)) {

		ImGui::SetCursorPos(ImVec2(5, 25));

		if (ImGui::BeginChild("url management", ImVec2((float)(gvars->window.width - 15) / 2, 77), true, 0)) {

			ImGui::Text("URL management");

			static std::string buf;
			ImGui::SetNextItemWidth((float)(gvars->window.width - 15) / 2 - 15);
			ImGui::InputText("##urlmanagement", &buf);

			if (ImGui::Button("insert new", ImVec2((float)(gvars->window.width - 15) / 4.35, 20))) {
				if (!buf.empty()) {
					if (std::find(buf.begin(), buf.end(), ' ') == buf.end()) {
						if (!gvars->ui.url_list.empty()) {
							for (size_t i = 0; i < gvars->ui.url_list.size(); ++i) {
								if (std::find(gvars->ui.url_list.begin(), gvars->ui.url_list.end(), buf) == gvars->ui.url_list.end()) {
									gvars->ui.url_list.push_back(buf);
								}
							}
						} else {
							gvars->ui.url_list.push_back(buf);
						}
					}
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("remove selected", ImVec2((float)(gvars->window.width - 15) / 4.35, 20)) && !gvars->ui.url_list.empty()) {
				if (gvars->ui.selected_url >= gvars->ui.url_list.size() && gvars->ui.url_list.size() != 0)
					gvars->ui.selected_url = gvars->ui.url_list.size() - 1;

				gvars->ui.url_list.erase(gvars->ui.url_list.begin() + gvars->ui.selected_url);
			}


			ImGui::EndChild();
		}



		ImGui::SetCursorPos(ImVec2(5 + (float)(gvars->window.width - 15) / 2 + 5, 25));
		if (ImGui::BeginChild("additional options", ImVec2((float)(gvars->window.width - 15) / 2, 220), true, 0)) {
			
			ImGui::Text("additional options");

			ImGui::Checkbox("add metadata", &gvars->ui.add_metadata);
			ImGui::Checkbox("embed thumbnail", &gvars->ui.embed_thumbnail);
			ImGui::Checkbox("prefer max quality", &gvars->ui.max_quality);
		
			ImGui::Text("output file format");
			if (ImGui::BeginCombo("##combo", gvars->ui.file_format.c_str())) { // The second parameter is the label previewed before opening the combo.

				for (int n = 0; n < gvars->ui.file_formats.size(); ++n) {

					bool is_selected = (gvars->ui.file_format == gvars->ui.file_formats[n].c_str()); // You can store your selection however you want, outside or inside your objects

					if (ImGui::Selectable(gvars->ui.file_formats[n].c_str(), is_selected))
						gvars->ui.file_format = gvars->ui.file_formats[n].c_str();

					if (is_selected)
						ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)

				}

				ImGui::EndCombo();
			}

			ImGui::Text("title format");
			ImGui::SetNextItemWidth((float)(gvars->window.width - 15) / 2 - 15);
			ImGui::InputText("##titleformat", &gvars->ui.title_format);

			ImGui::Text("output directory");
			ImGui::InputText("##outputdir", &gvars->ui.output_folder);

			ImGui::EndChild();
		}



		ImGui::SetCursorPos(ImVec2(5, (gvars->window.height - 30) / 1.48));
		if (ImGui::BeginListBox("", ImVec2(gvars->window.width - 10, gvars->window.height - (gvars->window.height - 30) / 1.45 ))) {
			for (int n = 0; n < gvars->ui.url_list.size(); n++) {
				const bool is_selected = (gvars->ui.selected_url == n);
				if (ImGui::Selectable(gvars->ui.url_list[n].c_str(), is_selected))
					gvars->ui.selected_url = n;

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}


		ImGui::SetCursorPos(ImVec2(5, 25 + 77 + 5));
		if (ImGui::BeginChild("download", ImVec2((float)(gvars->window.width - 15) / 2, 77), true, 0)) {

			if (ImGui::Button("download", ImVec2((float)(gvars->window.width - 15) / 4.35, 20)) && !gvars->ui.url_list.empty()) {

				static std::string exec_string;

				for (size_t i = 0; i < gvars->ui.url_list.size(); ++i) {

					gvars->ui.exec_string += "_ytdlp_helper_.exe";

					gvars->ui.exec_string += " -x --audio-format " + gvars->ui.file_format;

					if (gvars->ui.max_quality)
						gvars->ui.exec_string += " --audio-quality 0";

					if (gvars->ui.add_metadata)
						gvars->ui.exec_string += " --add-metadata";

					if (gvars->ui.embed_thumbnail)
						gvars->ui.exec_string += " --embed-thumbnail";

					gvars->ui.exec_string += " -o \"" + gvars->ui.output_folder + "\\" + gvars->ui.title_format + "\"";

					gvars->ui.exec_string += " \"" + gvars->ui.url_list[i] + "\"";

					std::thread t(download);
					t.detach();
					std::this_thread::sleep_for(std::chrono::milliseconds(50));

					gvars->ui.exec_string.clear();

				}

			}

			ImGui::SameLine();
			if (ImGui::Button("cancel", ImVec2((float)(gvars->window.width - 15) / 4.35, 20))) {

				system("taskkill /F /T /IM _ytdlp_helper_.exe");

			}

			ImGui::EndChild();
		}

	}

	//ImGui::ShowDemoWindow();

}