#include <wm/window.hpp>
#include <wm/libwm.h>

using namespace std;

namespace btos_api{
namespace wm{

	Window::Window(const gds::Point &p, uint32_t options, uint32_t subscriptions, const gds::Surface &surf, const string &title) :
		id(WM_NewWindow(p.x, p.y, options, subscriptions, surf.GetID(), title.c_str())), owned(true)
	{}
	Window::Window(const wm_WindowInfo &info) :
		id(WM_CreateWindow(info)), owned(true)
	{}
	Window::Window(Window &&w) : id(w.id), owned(w.owned){
		w.id = 0;
		w.owned = false;
	}

	Window::~Window(){
		if(id && owned){
			Select();
			WM_DestroyWindow();
		}
	}

	Window Window::Wrap(uint64_t id, bool own){
		Window ret;
		ret.id = id;
		ret.owned = own;
		return ret;
	}

	void Window::Select(){
		WM_SelectWindow(id);
	}
	uint64_t Window::GetID(){
		return id;
	}
	
	wm_WindowInfo Window::Info(){
		Select();
		return WM_WindowInfo();
	}
	void Window::SetSubscribed(uint32_t events){
		Select();
		WM_Subscribe(events);
	}
	void Window::Update(){
		Select();
		WM_Update();
	}
	void Window::Update(const gds::Rect &r){
		Select();
		WM_UpdateRect(r.x, r.y, r.w, r.h);
	}
	void Window::SetSurface(const gds::Surface &surf){
		Select();
		WM_ReplaceSurface(surf.GetID());
	}
	gds::Surface Window::GetSurface(){
		return gds::Surface::Wrap(Info().gds_id, false);
	}
	void Window::SetPosition(const gds::Point &p){
		Select();
		WM_MoveWindow(p.x, p.y);
	}
	gds::Point Window::GetPosition(){
		auto info = Info();
		return {info.x, info.y};
	}
	void Window::SetOptions(uint32_t options){
		Select();
		WM_ChangeOptions(options);
	}
	uint32_t Window::GetOptions(){
		return Info().options;
	}
	void Window::SetTitle(const std::string &title){
		Select();
		WM_SetTitle(title.c_str());
	}
	std::string Window::GetTitle(){
		return Info().title;
	}

}
}
