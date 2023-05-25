#pragma once

#pragma comment(lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
#include <d3d9.h>
#include <d3dx9.h>

namespace dx9 {

	inline HWND window = nullptr;
	inline WNDCLASSEXA window_class = { };

	inline POINTS position = { };

	inline PDIRECT3D9 d3d = nullptr;
	inline LPDIRECT3DDEVICE9 device = nullptr;
	inline D3DPRESENT_PARAMETERS present_parameters = { };

	void create_window(const char* window_name, const char* class_name) noexcept;
	void destroy_window() noexcept;

	bool create_device() noexcept;
	void reset_device() noexcept;
	void destroy_device() noexcept;

};

namespace gui {

	void create_imgui() noexcept;
	void destroy_imgui() noexcept;

	void begin_render() noexcept;
	void render() noexcept;
	void end_render() noexcept;

}