#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <windows.h>
#include "MonoBehaviour.h"
#include "Entity.h"

#define LOG(msg) { OutputDebugStringA(msg); OutputDebugStringA("\n"); } // デバッグ出力用の簡易ログマクロ

class GameScript : public MonoBehaviour {
public:
    std::string m_scriptName = "PlayerController";
    char        m_scriptNameBuf[256] = {};

    GameScript() {
        strcpy_s(m_scriptNameBuf, m_scriptName.c_str());
    }

    ~GameScript();

    Component* Clone() const override {
        auto* gs = new GameScript();
        gs->m_scriptName = m_scriptName;
        strcpy_s(gs->m_scriptNameBuf, gs->m_scriptName.c_str());
        return gs;
    }

    Entity* GetOwner() const { return Component::entity; }

    void Start() override;          // ここで ScriptRuntime に登録する
    void CreateScript(const std::string& name);   // 中身は今のまま
    void StartScript();             // .cs 生成 + VS 起動
    void ReloadScript();            // ScriptRuntime::RebuildAndRestart を呼ぶだけ

    // Update() は不要になった（ScriptRuntime が一括で回す）

private:
    int  m_instanceId = -1;
    bool m_vsOpened = false;
};