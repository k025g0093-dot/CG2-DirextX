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



using component_start_fn = void(*)();

class GameScript : public MonoBehaviour {

private:
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
        TerminateProcess(m_pi.hProcess, 0);
        CloseHandle(m_pi.hProcess);
        CloseHandle(m_pi.hThread);
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
        ReloadScript();
    }

    PROCESS_INFORMATION m_pi = {};

    void ReloadScript() {
        if (m_pi.hProcess) {
            TerminateProcess(m_pi.hProcess, 0);
            CloseHandle(m_pi.hProcess);
            CloseHandle(m_pi.hThread);
            m_pi = {};
        }
        system("dotnet build externals/GameScript/GameScriptC/GameScriptC/GameScriptC.csproj -c Debug");
        STARTUPINFOW si = { sizeof(si) };
        CreateProcessW(
            L"externals/GameScript/GameScriptC/GameScriptC/bin/Debug/net10.0/GameScriptC.exe",
            NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &m_pi);
    }

    void Update() override {
       
    }

private:

};