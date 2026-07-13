#pragma once
#include <Windows.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <filesystem>
#include <fstream> 
#include "externals/scripting/hostfxr.h"
#include "externals/scripting/coreclr_delegates.h"
#include "MonoBehaviour.h"
#include "Entity.h"
#include "EntityManager.h"

#ifndef UNMANAGEDCALLERSONLY_METHOD
#define UNMANAGEDCALLERSONLY_METHOD ((const char_t*)-1)
#endif
// GameScript.h の先頭あたりに追加
#define LOG(msg) { OutputDebugStringA(msg); OutputDebugStringA("\n"); }


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
	std::string scriptName = {};


	GameScript() {

		setvbuf(stdout, nullptr, _IONBF, 0);
		std::cout << "=== C# Script Console ===" << std::endl;
		strcpy_s(m_scriptNameBuf, m_scriptName.c_str());
	}

	~GameScript() {
		TerminateProcess(m_pi.hProcess, 0);
		//CloseHandle(m_hPipe);
		CloseHandle(m_pi.hProcess);
		CloseHandle(m_pi.hThread);
	}


	bool m_vsOpened = false;

	void Start() override {
		MonoBehaviour::Start();
		//ReloadScript();
	}



	PROCESS_INFORMATION m_pi = {};
	HANDLE m_hPipe = INVALID_HANDLE_VALUE;


	//csファイルがない場合に作成する
	void CreateScript(const std::string& name) {
		std::string tmplPath = "externals/GameScript/GameScriptC/GameScriptC/GameScript/Templet.cs.txt";
		std::string outPath = "externals/GameScript/GameScriptC/GameScriptC/Scripts/" + name + ".cs";
		//テンプレートなどのパスの取得

		//パスがそもそもあるかの確認
		if (std::filesystem::exists(outPath)) return;

		//ファイルシステム
		std::filesystem::create_directories(
			"externals/GameScript/GameScriptC/GameScriptC/Scripts");

		std::ifstream tmpl(tmplPath);
		if (!tmpl) {
			LOG("[GameScript] Template not found: ");
			return;
		}
		std::string content((std::istreambuf_iterator<char>(tmpl)), std::istreambuf_iterator<char>());
		tmpl.close();

		//スクリプトの名前を新しくできるクラスに変更
		{
			size_t pos = 0;
			const std::string placeholder = "#SCRIPTNAME#";
			while ((pos = content.find(placeholder, pos)) != std::string::npos) {
				content.replace(pos, placeholder.size(), name);
				pos += name.size();
			}
		}

		std::ofstream file(outPath);
		file << content;
		file.close();
		LOG("[GameScript] Connected!");
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

	void ReloadScript() {
		static bool s_csStarted = false;
		system("taskkill /f /im GameScriptC.exe 2>nul");
		if (m_pi.hProcess) {
			TerminateProcess(m_pi.hProcess, 0);
			CloseHandle(m_hPipe);
			CloseHandle(m_pi.hProcess);
			CloseHandle(m_pi.hThread);
			m_pi = {};
			s_csStarted = false;
		}

		CreateScript(m_scriptName);
		//C#のプロジェクトをビルドする
		system("dotnet build externals/GameScript/GameScriptC/GameScriptC/GameScriptC.csproj -c Debug --force");


		if (!s_csStarted) {

			//C#のプロセスを起動する
			STARTUPINFOW si = { sizeof(si) };
			if (!CreateProcessW(
				L"externals/GameScript/GameScriptC/GameScriptC/bin/Debug/net10.0/GameScriptC.exe",
				NULL, NULL, NULL, FALSE,
				0, NULL, NULL, &si, &m_pi)) {
				DWORD err = GetLastError();
				LOG("[GameScript] CreateProcessW failed: " );
			}
			s_csStarted = true;
			Sleep(2000);
		}

		std::wstring pipeName = L"\\\\.\\pipe\\GameScriptPipe";

		LOG("[GameScript] Connected!");
		if (WaitNamedPipeW(pipeName.c_str(), 5000)) {
			m_hPipe = CreateFileW(
				pipeName.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				0, NULL, OPEN_EXISTING, 0, NULL);
			if (m_hPipe == INVALID_HANDLE_VALUE)
				std::cout << "[GameScript] CreateFileW failed: " << GetLastError() << std::endl;
			else
				LOG("[GameScript] Connected!");
			DWORD written;
			WriteFile(m_hPipe, m_scriptName.c_str(), (DWORD)m_scriptName.size() + 1, &written, NULL);
		}
		else {
			LOG( "[GameScript] WaitNamedPipeW timed out" );
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