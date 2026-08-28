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
#define LOG(msg) { OutputDebugStringA(msg); OutputDebugStringA("\n"); } // デバッグ出力用の簡易ログマクロ


using component_start_fn = void(*)();

// C#スクリプトをホストしてEntityの座標をやり取りするコンポーネント
class GameScript : public MonoBehaviour {

private:
	hostfxr_close_fn    m_close_fptr = nullptr; // hostfxrのクローズ関数ポインタ(現状未使用)
	hostfxr_handle      m_runtime_handle = nullptr; // .NETランタイムのハンドル(現状未使用)
	bool m_loaded = false; // ロード済みフラグ(現状未使用)
	Entity* entity = nullptr; // 座標同期対象のEntity
public:

	std::string m_scriptName = "PlayerController"; // デフォルトのスクリプト名(未設定時の初期値)
	char m_scriptNameBuf[256] = {}; // スクリプト名編集用バッファ(UI等で使用想定)
	std::string scriptName = {}; // 予備の変数(現状未使用)


	GameScript() {
		// コンソール出力をバッファリングなしに設定(即座に表示するため)
		setvbuf(stdout, nullptr, _IONBF, 0);
		std::cout << "=== C# Script Console ===" << std::endl;
		strcpy_s(m_scriptNameBuf, m_scriptName.c_str());
	}

	~GameScript() {
		// C#プロセスと関連ハンドルを終了・解放
		TerminateProcess(m_pi.hProcess, 0);
		//CloseHandle(m_hPipe);
		CloseHandle(m_pi.hProcess);
		CloseHandle(m_pi.hThread);
	}


	Component* Clone()const override {
		auto* gs = new GameScript();
		gs->m_scriptName = m_scriptName;
		return gs;
	}

	bool m_vsOpened = false; // Visual Studioを既に開いたかどうかのフラグ

	void Start() override {
		MonoBehaviour::Start();
		//ReloadScript();
	}



	PROCESS_INFORMATION m_pi = {}; // 起動したC#プロセスの情報
	HANDLE m_hPipe = INVALID_HANDLE_VALUE; // C#プロセスとの名前付きパイプハンドル


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

		// テンプレートファイルを開いて読み込む
		std::ifstream tmpl(tmplPath);
		if (!tmpl) {
			LOG("[GameScript] Template not found: ");
			return;
		}
		std::string content((std::istreambuf_iterator<char>(tmpl)), std::istreambuf_iterator<char>());
		tmpl.close();

		//スクリプトの名前を新しくできるクラスに変更
		{
			// プレースホルダーをスクリプト名に全置換
			size_t pos = 0;
			const std::string placeholder = "#SCRIPTNAME#";
			while ((pos = content.find(placeholder, pos)) != std::string::npos) {
				content.replace(pos, placeholder.size(), name);
				pos += name.size();
			}
		}

		// 置換後の内容を出力先ファイルに書き込む
		std::ofstream file(outPath);
		file << content;
		file.close();
		LOG("[GameScript] Connected!");
	}


	void StartScript() {
		MonoBehaviour::Start();
		if (!m_vsOpened) {
			m_vsOpened = true;
			// このコンポーネントを持つEntityを探して保持しておく
			for (auto& e : EntityManager::GetInstance()->GetEntities()) {
				if (e->GetComponent<GameScript>() == this) {
					entity = e.get();
					break;
				}
			}
			// Visual Studioでcsprojを開く
			ShellExecuteA(NULL, "open", "devenv.exe",
				"\"externals/GameScript/GameScriptC/GameScriptC/GameScriptC.csproj\"",
				NULL, SW_SHOWNORMAL);
		}
		ReloadScript();
	}

	void ReloadScript() {

		for (auto& e : EntityManager::GetInstance()->GetEntities()) {
			if (e->GetComponent<GameScript>() == this) {
				entity = e.get();
				break;
			}
		}

		static bool s_built = false;

		if (m_pi.hProcess) {
			// 前回起動したプロセス・ハンドルを後始末
			TerminateProcess(m_pi.hProcess, 0);
			//CloseHandle(m_hPipe);
			CloseHandle(m_pi.hProcess);
			CloseHandle(m_pi.hThread);
			m_pi = {};
		}

		CreateScript(m_scriptName);

		system("dotnet build externals/GameScript/GameScriptC/GameScriptC/GameScriptC.csproj -c Debug --force");

		std::wstring cmdLine = L"GameScriptC.exe " + std::wstring(m_scriptName.begin(), m_scriptName.end());


		//C#のプロセスを起動する
		STARTUPINFOW si = { sizeof(si) };
		if (!CreateProcessW(
			L"externals/GameScript/GameScriptC/GameScriptC/bin/Debug/net10.0/GameScriptC.exe",
			&cmdLine[0], NULL, NULL, FALSE,
			0, NULL, NULL, &si, &m_pi)) {
			DWORD err = GetLastError();
			LOG("[GameScript] CreateProcessW failed: ");
		}
		Sleep(2000); // C#プロセスの起動待ち


		std::wstring pipeName = L"\\\\.\\pipe\\GameScriptPipe_" + std::wstring(m_scriptName.begin(), m_scriptName.end());

		LOG("[GameScript] Connected!");
		// パイプが使えるようになるまで待って接続
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
			// 接続直後にスクリプト名をC#側へ送信
			WriteFile(m_hPipe, m_scriptName.c_str(), (DWORD)m_scriptName.size() + 1, &written, NULL);
		}
		else {
			LOG("[GameScript] WaitNamedPipeW timed out");
		}

	}

	void Update() override {
		if (m_hPipe == INVALID_HANDLE_VALUE || !entity) return;
		float dt = 0.016f; // 固定デルタタイム
		DWORD written;
		// 現在の座標とdtをC#側に送信
		float sendData[4] = {
			entity->transform.position.x,
			entity->transform.position.y,
			entity->transform.position.z,
			dt
		};
		WriteFile(m_hPipe, sendData, sizeof(sendData), &written, NULL);
		float recvData[3];
		DWORD read;
		// C#側で更新された座標を受信して反映
		if (ReadFile(m_hPipe, recvData, sizeof(recvData), &read, NULL) && read == sizeof(recvData)) {
			entity->transform.position.x = recvData[0];
			entity->transform.position.y = recvData[1];
			entity->transform.position.z = recvData[2];
		}
	}
private:
};