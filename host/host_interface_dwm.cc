#include "common/log.h"
#include "common/strings.h"
#include "host/host_interface_dwm.h"

host::dwm_interface::ptr g_host_interface{ nullptr };

set_log_channel("DWM");

using namespace common;

namespace host
{
  dwm_interface::dwm_interface() = default;

  dwm_interface::~dwm_interface() = default;

  LRESULT CALLBACK dwm_interface::winproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
  {
    switch (msg)
    {
    case WM_SIZE:
    {
      const u32 width = LOWORD(lparam);
      const u32 height = HIWORD(lparam);

      g_host_interface->handle_resize(width, height);
    } break;
    case WM_CLOSE:
    case WM_QUIT:
      g_host_interface->quit_message_pump();
      break;
    default:
      return DefWindowProcW(hwnd, msg, wparam, lparam);
    }

    return 0;
  }

  bool dwm_interface::create(std::string_view name)
  {
    log_info("Creating DWM interface");

    auto host_interface = std::make_unique<dwm_interface>();

    host_interface->set_window_name(name);
    host_interface->set_module(GetModuleHandleW(nullptr));

    if (!host_interface->register_window_class(name))
    {
      log_error("Failed to register window class");

      return false;
    }

    g_host_interface = std::move(host_interface);

    return true;
  }

  bool dwm_interface::register_window_class(std::string_view class_name)
  {
    log_info("Registering window class");

    const auto wide_class_name = strings::to_wstring(class_name);

    if (!wide_class_name)
      panicf("Failed to convert window class name");

    const WNDCLASSEXW window_class_descriptor =
    {
      .cbSize = sizeof(WNDCLASSEXW),
      .style = 0,
      .lpfnWndProc = &dwm_interface::winproc,
      .cbClsExtra = 0,
      .cbWndExtra = 0,
      .hInstance = m_module_base,
      .hIcon = NULL,
      .hCursor = LoadCursorW(NULL, IDC_ARROW),
      .hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1),
      .lpszMenuName = NULL,
      .lpszClassName = wide_class_name->c_str(),
      .hIconSm = NULL
    };

    if (!RegisterClassExW(&window_class_descriptor))
      return false;

    return true;
  }

  bool dwm_interface::create_render_window()
  {
    log_info("Creating render window");

    constexpr DWORD window_style = WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_SIZEBOX;
    constexpr DWORD window_style_ex = WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE;

    const auto wide_window_name = strings::to_wstring(m_window_title);

    if (!wide_window_name)
      panicf("Failed to convert window title");

    auto window_handle = CreateWindowExW(window_style_ex, wide_window_name->c_str(), wide_window_name->c_str(),
      window_style, CW_USEDEFAULT, CW_USEDEFAULT, 640, 448, nullptr, nullptr, m_module_base, nullptr);

    if (!window_handle)
    {
      error_dialog("Failed to create render window");

      return false;
    }

    m_window_handle.reset(window_handle);

    ShowWindow(m_window_handle.get(), SW_SHOW);

    if (!UpdateWindow(m_window_handle.get()))
    {
      error_dialog("Failed to update window");

      return false;
    }

    log_info("Render window created");

    return true;
  }

  int dwm_interface::run_message_pump()
  {
    log_info("Starting Windows message pump");

    while (!m_message_pump_quit_requested)
    {
      MSG msg;
      if (!GetMessageW(&msg, nullptr, 0, 0))
        continue;

      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    return EXIT_SUCCESS;
  }

  void dwm_interface::error_dialog(std::string_view message)
  {
    const auto wide_message = strings::to_wstring(message);
    if (!wide_message)
      panicf("Failed to convert error message");

    MessageBoxW(m_window_handle.get(), wide_message->c_str(), L"Error", MB_ICONERROR | MB_OK);
  }

  void dwm_interface::handle_resize(u32 width, u32 height)
  {
    log_info("Window resize event width: {}, height: {}", width, height);
  }
}