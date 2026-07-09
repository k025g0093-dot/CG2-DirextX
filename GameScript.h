#pragma once
#include <Windows.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <filesystem>
#include "externals/scripting/hostfxr.h"
#include "externals/scripting/coreclr_delegates.h"
#include "MonoBehaviour.h"
#include "Entity.h"
#include "EntityManager.h"

#ifndef UNMANAGEDCALLERSONLY_METHOD
#define UNMANAGEDCALLERSONLY_METHOD ((const char_t*)-1)
#endif



using component_start_fn = void(*)();

class GameScript : public MonoBehaviour {

private:
    hostfxr_close_fn    m_close_fptr = nullptr;
    hostfxr_handle      m_runtime_handle = nullptr;
    bool m_loaded = false;
    Entity* entity = nullptr;
public:

    std::string m_scriptName = "PlayerController"; // デフォルトのスクリプト名(未設定時の初期値)
    char m_scriptNameBuf[256] = {};



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
        strcpy_s(m_scriptNameBuf, m_scriptName.c_str());
    }

    ~GameScript() {
        TerminateProcess(m_pi.hProcess, 0);
        CloseHandle(m_hPipe);
        CloseHandle(m_pi.hProcess);
        CloseHandle(m_pi.hThread);
    }


    bool m_vsOpened = false;

    void Start() override {
        MonoBehaviour::Start();
        ReloadScript();
    }

    void StartScript() {
        MonoBehaviour::Start();

        if (!m_vsOpened) {
            m_vsOpened = true;

            for (auto& e : EntityManager::GetInstance()->GetEntities()) {
                if (e->GetComponent<GameScript>() == this) {
                    entity = e.get();
                    break;
                }
            }

            ShellExecuteA(NULL, "open", "devenv.exe",
                "\"externals/GameScript/GameScriptC/GameScriptC/GameScriptC.csproj\"",
                NULL, SW_SHOWNORMAL);

        }
        ReloadScript();

    }

    PROCESS_INFORMATION m_pi = {};
    HANDLE m_hPipe = INVALID_HANDLE_VALUE;


    void ReloadScript() {
        if (m_pi.hProcess) {
            TerminateProcess(m_pi.hProcess, 0);
            CloseHandle(m_hPipe);
            CloseHandle(m_pi.hProcess);
            CloseHandle(m_pi.hThread);
            m_pi = {};
        }

        //C#のプロジェクトをビルドする
        system("dotnet build externals/GameScript/GameScriptC/GameScriptC/GameScriptC.csproj -c Debug --force");


        static bool s_csStarted = false;
        if (!s_csStarted) {

            //C#のプロセスを起動する
            STARTUPINFOW si = { sizeof(si) };
            if (!CreateProcessW(
                L"externals/GameScript/GameScriptC/GameScriptC/bin/Debug/net10.0/GameScriptC.exe",
                NULL, NULL, NULL, FALSE,
                0, NULL, NULL, &si, &m_pi)) {
                DWORD err = GetLastError();
                std::cout << "[GameScript] CreateProcessW failed: " << err << std::endl;
            }
            s_csStarted = true;
            Sleep(2000);
        }

        std::wstring pipeName = L"\\\\.\\pipe\\GameScriptPipe" + std::to_wstring(entity->name.begin()+entity->name.end());

        std::cout << "[GameScript] Connecting..." << std::endl;
        if (WaitNamedPipeW(pipeName.c_str(), 5000)) {
            m_hPipe = CreateFileW(
                pipeName.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                0, NULL, OPEN_EXISTING, 0, NULL);
            if (m_hPipe == INVALID_HANDLE_VALUE)
                std::cout << "[GameScript] CreateFileW failed: " << GetLastError() << std::endl;
            else
                std::cout << "[GameScript] Connected!" << std::endl;
        }
        else {
            std::cout << "[GameScript] WaitNamedPipeW timed out" << std::endl;
        }

    }

    void Update() override {
        if (m_hPipe == INVALID_HANDLE_VALUE) return;
        DWORD written;
        float sendData[4] = { 0 };
        WriteFile(m_hPipe, sendData, sizeof(sendData), &written, NULL);
        float recvData[3];
        DWORD read;
        ReadFile(m_hPipe, recvData, sizeof(recvData), &read, NULL);
    }





private:

};