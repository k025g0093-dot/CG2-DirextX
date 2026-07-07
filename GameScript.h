#pragma once
#include <Windows.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <filesystem>
#include "externals/scripting/hostfxr.h"
#include "externals/scripting/coreclr_delegates.h"
#include "MonoBehaviour.h"

#ifndef UNMANAGEDCALLERSONLY_METHOD
#define UNMANAGEDCALLERSONLY_METHOD ((const char_t*)-1)
#endif

#ifndef UNMANAGEDCALLSONLY_METHOD
#define UNMANAGEDCALLSONLY_METHOD UNMANAGEDCALLERSONLY_METHOD
#endif

using component_start_fn = void(*)();

class GameScript : public MonoBehaviour {
private:
    component_start_fn  csharp_start = nullptr;
    component_start_fn  csharp_update = nullptr;
    hostfxr_close_fn    m_close_fptr = nullptr;
    hostfxr_handle      m_runtime_handle = nullptr;
    bool m_loaded = false;

public:
    GameScript() {
        // Show a console even in GUI apps so std::cout / Console.WriteLine is visible
        if (!AllocConsole()) {
            // If one already exists, try AttachConsole instead
            AttachConsole(ATTACH_PARENT_PROCESS);
        }
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        setvbuf(stdout, nullptr, _IONBF, 0);
        std::cout << "=== C# Script Console ===" << std::endl;
    }

    ~GameScript() {
        Cleanup();
    }

    void LoadCSharpRuntime() {
        if (m_loaded) return;

        // Find the correct hostfxr.dll (avoid the fake one from Windows Performance Toolkit)
        std::string fxr_path = FindHostFxr();
        if (fxr_path.empty()) {
            std::cout << "[Error] hostfxr.dll not found. The .NET SDK may not be installed." << std::endl;
            return;
        }
        HMODULE lib = LoadLibraryA(fxr_path.c_str());
        if (!lib) {
            std::cout << "[Error] Failed to load hostfxr.dll: " << fxr_path << std::endl;
            return;
        }

        auto init_fptr = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress(lib, "hostfxr_initialize_for_runtime_config");
        auto get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)GetProcAddress(lib, "hostfxr_get_runtime_delegate");
        m_close_fptr = (hostfxr_close_fn)GetProcAddress(lib, "hostfxr_close");

        if (!init_fptr || !get_delegate_fptr || !m_close_fptr) {
            std::cout << "[Error] Failed to retrieve hostfxr function pointers." << std::endl;
            return;
        }

        const wchar_t* config_path = L"bin/Debug/net10.0_script/GameScriptC.runtimeconfig.json";

        int rc = init_fptr(config_path, nullptr, &m_runtime_handle);
        if (rc != 0 || m_runtime_handle == nullptr) {
            std::cout << "[Error] Failed to initialize the .NET runtime. Code: " << rc << std::endl;
            return;
        }

        load_assembly_and_get_function_pointer_fn get_csharp_fptr = nullptr;
        rc = get_delegate_fptr(m_runtime_handle, hdt_load_assembly_and_get_function_pointer, (void**)&get_csharp_fptr);
        if (rc != 0 || get_csharp_fptr == nullptr) {
            std::cout << "[Error] Failed to retrieve the delegate function." << std::endl;
            return;
        }

        // Full path to the compiled C# DLL
        const wchar_t* assembly_path = L"bin/Debug/net10.0_script/GameScriptC.dll";

        // Get the Start function
        rc = get_csharp_fptr(
            assembly_path,
            L"GameScriptC.MyScript, GameScriptC",
            L"Start",
            UNMANAGEDCALLSONLY_METHOD,
            nullptr,
            (void**)&csharp_start
        );

        // Get the Update function
        rc = get_csharp_fptr(
            assembly_path,
            L"GameScriptC.MyScript, GameScriptC",
            L"Update",
            UNMANAGEDCALLSONLY_METHOD,
            nullptr,
            (void**)&csharp_update
        );

        if (!csharp_start || !csharp_update) {
            std::cout << "[Error] Failed to obtain C# function pointers. Start=" << (void*)csharp_start
                      << " Update=" << (void*)csharp_update << std::endl;
            return;
        }

        m_loaded = true;
        std::cout << "[OK] C# runtime initialized. Function pointers OK." << std::endl;
    }

    bool m_vsOpened = false;

    void Start() override {
        MonoBehaviour::Start();
        if (!m_vsOpened) {
            m_vsOpened = true;
            ShellExecuteA(NULL, "open", "devenv.exe",
                "\"externals/GameScript/GameScriptC/GameScriptC/GameScriptC.csproj\"",
                NULL, SW_SHOWNORMAL);
        }
        ReloadScript(); // 最初のビルド＆起動もこれでOK
    }

	void ReloadScript() {

        system("dotnet build externals/GameScript/GameScriptC/GameScriptC/GameScriptC.csproj -c Debug -o bin\\Debug\\net10.0_script");
        LoadCSharpRuntime();
		if (csharp_start) csharp_start();
	}

    void Update() override {
        if (csharp_update) { csharp_update(); };
    }

private:
    static std::string FindHostFxr() {
        // Look for the latest version of hostfxr.dll under C:\Program Files\dotnet\host\fxr\
        
        namespace fs = std::filesystem;
        std::string dotnet_root = "C:\\Program Files\\dotnet";
        fs::path fxr_dir = dotnet_root + "\\host\\fxr";
        if (!fs::exists(fxr_dir)) {
            // Also try the DOTNET_ROOT environment variable if it's set
            char* env_root = nullptr;
            size_t len = 0;
            if (_dupenv_s(&env_root, &len, "DOTNET_ROOT") == 0 && env_root) {
                fxr_dir = std::string(env_root) + "\\host\\fxr";
                free(env_root);
            }
            if (!fs::exists(fxr_dir)) return "";
        }

        std::string best;
        for (auto& entry : fs::directory_iterator(fxr_dir)) {
            if (entry.is_directory()) {
                fs::path dll = entry.path() / "hostfxr.dll";
                if (fs::exists(dll)) {
                    // Simple string comparison as version string (e.g. "10.0.9" > "8.0.28")
                    std::string ver = entry.path().filename().string();
                    if (ver > best) best = ver;
                }
            }
        }
        if (best.empty()) return "";
        return (fxr_dir / best / "hostfxr.dll").string();
    }

    void Cleanup() {
        if (m_runtime_handle && m_close_fptr) {
            m_close_fptr(m_runtime_handle);
            m_runtime_handle = nullptr;
        }
        csharp_start = nullptr;
        csharp_update = nullptr;
        m_loaded = false;
    }
};