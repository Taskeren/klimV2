#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include <windows.h>
#include <GL/GL.h>
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include "HotkeyManager.h"
#include "UserInterface.h"
#include "helperFunctions.h"
#include "ConfigFile.h"


UserInterface::UserInterface(std::vector<limit*> limit_ptr_vector, wchar_t* path_to_config_file) 
    : limit_ptr_vector(limit_ptr_vector), path_to_config_file(path_to_config_file) {
}

int UserInterface::run_gui(){
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    //void (UserInterface::*ptrWndProc)(LRESULT WINAPI)
    WNDCLASSEXW wc = { sizeof(wc), CS_OWNDC, &UserInterface::WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"klim config", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize OpenGL
    if (!CreateDeviceWGL(hwnd, &g_MainWindow))
    {
        CleanupDeviceWGL(hwnd, &g_MainWindow);
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    wglMakeCurrent(g_MainWindow.hDC, g_hRC);

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_InitForOpenGL(hwnd);
    ImGui_ImplOpenGL3_Init();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    std::vector<bool> button_clicked(limit_ptr_vector.size());
    std::vector<std::string> String;
    const char* in_progress = "in progress..";
    for (int i = 0; i < (limit_ptr_vector.size()); i++) {
        String.push_back("blank");
    }
    while (!done)
    {
        int line_of_button_clicked = -1;
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;
            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            bool use_work_area = true; // fullscreen
            static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
            ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);
            ImGuiStyle& style = ImGui::GetStyle();
            style.FrameRounding = 0.0f;
            style.WindowPadding =  ImVec2(15.0f, 5.0f);

            ImGui::Begin("config", NULL, flags);

            {
                ImGui::SeparatorText("Hotkeys");
                for (int i = 0; i < limit_ptr_vector.size(); i++) {
                    std::cout << "key list size: " << limit_ptr_vector[i]->key_list.size() << std::endl;
                    if (limit_ptr_vector[i]->key_list.size() == 0) {
						String[i] = "blank";
					}
					else {
						String[i] = "";
                        for (int j = 0; j < limit_ptr_vector[i]->key_list.size(); j++) {
							if (String[i] != "") {
								String[i].append("+");
							}
							int scan_code = MapVirtualKey(limit_ptr_vector[i]->key_list[j], 0);
							char name_buffer[256];
							GetKeyNameTextA(scan_code << 16, name_buffer, sizeof(name_buffer) / sizeof(name_buffer[0]));
							String[i] += name_buffer;
						}
					}

                    ImGui::PushID(i);
                    if (ImGui::Button("Bind")) {
                        if (String[i] != in_progress) {
						        button_clicked[i] = true;
                                std::cout << "button " << i << " clicked [callback]" << std::endl;
						}
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Reset")) {
                        String[i] = "";
                        hotkeyInstance->done = true;
                        limit_ptr_vector[i]->key_list.clear();
                        limit_ptr_vector[i]->bindingComplete = true;
                        limit_ptr_vector[i]->updateUI = true;
                    }

                    ImGui::SameLine();
					char name[50];
					size_t size;
					wcstombs_s(&size, name, limit_ptr_vector[i]->name, 50);
					ImGui::Text("%s ", name);               // Display some text (you can use a format strings too)
					ImGui::SameLine();
                    ImGui::SetCursorPosX(170);
					ImGui::Text("[%s]", String[i].data());               // Display some text (you can use a format strings too)
                    ImGui::PopID();
                }

                ImGui::SetCursorPos(ImVec2(15, 250));

                if (ImGui::Button("Save")) {
                    ConfigFile::WriteConfig(limit_ptr_vector, path_to_config_file);
                }

                ImGui::SameLine();

                if (ImGui::Button("Exit")) {
                    // this only crashes the app but this is good enough for now
                    // TODO make it actually exit
                    ImGui::DestroyContext();
                    Helper::Exitapp(false);
                }
            }

            ImGui::End();
        }


        // Rendering
        ImGui::Render();

        for (int i = 0; i < button_clicked.size(); i++) {
            if (limit_ptr_vector[i]->bindingComplete == true && String[i] == in_progress) {
                std::cout << "updating ui.." << std::endl;
                String[i] = "";
			    for (int j = 0; j < limit_ptr_vector[i]->key_list.size(); j++) {
                    if (String[i] != "") {
					    String[i].append("+");
                    }
                    int scan_code = MapVirtualKey(limit_ptr_vector[i]->key_list[j], 0);
                    char name_buffer[256];
                    GetKeyNameTextA(scan_code << 16, name_buffer, sizeof(name_buffer) / sizeof(name_buffer[0]));
                    String[i] += name_buffer;
				}
			}

            if (button_clicked[i] == true) {
                std::cout << "button " << i << " clicked [registered]" << std::endl;
                String[i] = in_progress;
				button_clicked[i] = false;
				line_of_button_clicked = i;
				std::thread([&]() {
					hotkeyInstance->asyncBindHotkey(line_of_button_clicked);
					}).detach();
            }

        }



        glViewport(0, 0, g_Width, g_Height);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Present
        ::SwapBuffers(g_MainWindow.hDC);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceWGL(hwnd, &g_MainWindow);
    wglDeleteContext(g_hRC);
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool UserInterface::CreateDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    HDC hDc = ::GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    const int pf = ::ChoosePixelFormat(hDc, &pfd);
    if (pf == 0)
        return false;
    if (::SetPixelFormat(hDc, pf, &pfd) == FALSE)
        return false;
    ::ReleaseDC(hWnd, hDc);

    data->hDC = ::GetDC(hWnd);
    if (!g_hRC)
        g_hRC = wglCreateContext(data->hDC);
    return true;
}

void UserInterface::CleanupDeviceWGL(HWND hWnd, WGL_WindowData* data)
{
    wglMakeCurrent(NULL, NULL);
    ::ReleaseDC(hWnd, data->hDC);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.

LRESULT WINAPI UserInterface::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            instance->g_Width = LOWORD(lParam);
            instance->g_Height = HIWORD(lParam);
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        hotkeyInstance->KeyboardInputHandler(static_cast<int>(wParam), true);
        break;
    case WM_KEYUP:
        hotkeyInstance->KeyboardInputHandler(static_cast<int>(wParam), false);
        break;
    case WM_GETMINMAXINFO:
        MINMAXINFO* minMaxInfo = (MINMAXINFO*)lParam;
        minMaxInfo->ptMinTrackSize.x = minMaxInfo->ptMaxTrackSize.x = 300; // size x width
        minMaxInfo->ptMinTrackSize.y = minMaxInfo->ptMaxTrackSize.y = 330; // size y height
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
