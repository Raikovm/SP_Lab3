#pragma comment(lib, "d2d1")
#include <d2d1.h>
#include <windows.h>
#include "consts.h"
#include "Strategy.cpp"


void update_state(HMENU menu, UINT_PTR curr, UINT_PTR* prev, function* handler);
LRESULT CALLBACK wnd_proc(HWND, UINT, WPARAM, LPARAM);

template <class T> void safe_release(T** pp_t)
{
    if (*pp_t)
    {
        (*pp_t)->Release();
        *pp_t = NULL;
    }
}

class direct_2d_resources {
    ID2D1Factory* p_factory_;
    ID2D1HwndRenderTarget* p_render_target_;
    ID2D1SolidColorBrush* p_brush_;
public:
    direct_2d_resources() : p_factory_(nullptr), p_render_target_(nullptr), p_brush_(nullptr) {}

    HRESULT create_graphics_resources(const HWND hwnd)
    {
        HRESULT hr = S_OK;
        if (p_render_target_ == nullptr)
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            const D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
            hr = p_factory_->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(hwnd, size),
                &p_render_target_);
            if (SUCCEEDED(hr))
            {
                const D2D1_COLOR_F color = D2D1::ColorF(0.0f, 0.0f, 0.0f);
                hr = p_render_target_->CreateSolidColorBrush(color, &p_brush_);
            }
        }
        return hr;
    }

    void discard_graphics_resources()
    {
        safe_release(&p_render_target_);
        safe_release(&p_brush_);
    }

    void destroy()
    {
        discard_graphics_resources();
        safe_release(&p_factory_);
    }

    void on_paint(const HWND hwnd, const function* func)
    {
        HRESULT hr = create_graphics_resources(hwnd);
        if (SUCCEEDED(hr))
        {
	        PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            D2D1_POINT_2F apt[1000];

            p_render_target_->BeginDraw();
            p_render_target_->Clear(D2D1::ColorF(D2D1::ColorF::White));
	        const D2D1_SIZE_F size = p_render_target_->GetSize();
            p_render_target_->DrawLine(
                D2D1::Point2F(size.width / 2, 0.0f),
                D2D1::Point2F(size.width / 2, size.height),
                p_brush_,
                1.0f
            );
            p_render_target_->DrawLine(
                D2D1::Point2F(0.0f, size.height / 2),
                D2D1::Point2F(size.width, size.height / 2),
                p_brush_,
                1.0f
            );

            func->run_function(size.width, size.height, apt);
            for (int i = 0; i < NUM - 1; i++) {
                p_render_target_->DrawLine(
                    apt[i],
                    apt[i+1],
                    p_brush_,
                    2.0f
                );
            }

            hr = p_render_target_->EndDraw();
            if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
            {
                discard_graphics_resources();
            }
            EndPaint(hwnd, &ps);
        }
    }

    void resize(const HWND hwnd) const
    {
        if (p_render_target_ != nullptr)
        {
            RECT rc;
            GetClientRect(hwnd, &rc);
            const D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
            p_render_target_->Resize(size);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
    }

    BOOL create() {
        if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED, &p_factory_)))
        {
            return -1; 
        }
        return 0;
    }
};

int WINAPI WinMain(const HINSTANCE hInstance, HINSTANCE,
                   PSTR, const int iCmdShow)
{
    static TCHAR sz_app_name[] = TEXT("Lab3");
    MSG msg;
    WNDCLASS wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = wnd_proc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndclass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wndclass.lpszMenuName = nullptr;
    wndclass.lpszClassName = sz_app_name;

    RegisterClass(&wndclass);

    HMENU h_drop_down_menu = CreateMenu();

    AppendMenu(h_drop_down_menu, MF_STRING, OPT_SINE, L"Sine");
    EnableMenuItem(h_drop_down_menu, OPT_SINE, MF_GRAYED);
    AppendMenu(h_drop_down_menu, MF_STRING, OPT_SQRT, L"Square Root");
    AppendMenu(h_drop_down_menu, MF_STRING, OPT_PARA, L"Parabola");
    AppendMenu(h_drop_down_menu, MF_STRING, OPT_HYPER, L"Hyperbola");

    const HMENU hmenu = CreateMenu();

    AppendMenu(hmenu, MF_POPUP, reinterpret_cast<UINT_PTR>(h_drop_down_menu), L"Functions");

    const HWND hwnd = CreateWindow(sz_app_name, TEXT("Laboratory 3 - WinAPI"),
                             WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             NULL, hmenu, hInstance, NULL);

    ShowWindow(hwnd, iCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

LRESULT CALLBACK wnd_proc(const HWND hwnd, const UINT message, const WPARAM wParam, const LPARAM lParam)
{    
	static UINT_PTR last_menu_item = OPT_SINE;
    static auto func = new function(new Sin);

    switch (message)
    {
	    static direct_2d_resources* d2dr;
    case WM_CREATE:
        d2dr = new direct_2d_resources();
        return d2dr->create();
    case WM_COMMAND:
    {
	    const HMENU h_drop_down_menu = GetSubMenu(GetMenu(hwnd), 0);
        switch (wParam)
        {
        case OPT_SINE:
            update_state(h_drop_down_menu, OPT_SINE, &last_menu_item, func);
            break;
        case OPT_SQRT:
            update_state(h_drop_down_menu, OPT_SQRT, &last_menu_item, func);
            break;
        case OPT_PARA:
            update_state(h_drop_down_menu, OPT_PARA, &last_menu_item, func);
            break;
        case OPT_HYPER:
            update_state(h_drop_down_menu, OPT_HYPER, &last_menu_item, func);
            break;
        }
        InvalidateRect(hwnd, nullptr, TRUE);
    }
    case WM_SIZE:
        d2dr->resize(hwnd);
        return 0;

    case WM_PAINT:
    {
        d2dr->on_paint(hwnd, func);
        return 0;
    }
    case WM_DESTROY:
        d2dr->destroy();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}

void update_state(const HMENU menu, const UINT_PTR curr, UINT_PTR* prev, function* handler) {
    EnableMenuItem(menu, curr, MF_GRAYED);
    EnableMenuItem(menu, *prev, MF_ENABLED);
    *prev = curr;
    IFunctions* func = new Sin;
    switch (curr) {
    case OPT_SINE:
        func = new Sin;
        break;
    case OPT_SQRT:
        func = new Sqrt;
        break;
    case OPT_PARA:
        func = new para;
        break;
    case OPT_HYPER:
        func = new Hyper;
        break;
    }
    handler->set_func(func);
}