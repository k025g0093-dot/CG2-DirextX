#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

class GameScript;

class ScriptRuntime {
public:
    static ScriptRuntime* GetInstance();

    // GameScript から呼ぶ
    int  Register(GameScript* gs, const std::string& scriptName);
    void Unregister(int instanceId);

    // 毎フレーム1回
    void Tick(float dt);

    // 「C#のビルド」ボタン
    void RebuildAndRestart();

    void Shutdown();

    bool IsConnected() const { return m_hPipe != INVALID_HANDLE_VALUE; }
    int  GetInstanceCount() const { return (int)m_instances.size(); }

private:
    ScriptRuntime() = default;
    static ScriptRuntime* s_instance;

    bool EnsureStarted();       // プロセス起動 + パイプ接続（初回のみ）
    void KillProcess();

    static bool WriteAll(HANDLE h, const void* d, DWORD len);
    static bool ReadAll(HANDLE h, void* d, DWORD len);

    struct Entry {
        GameScript* script = nullptr;
        std::string name;
        bool  spawnPending = true;   // 次の Tick で spawn を送る
    };

    std::unordered_map<int, Entry> m_instances;
    std::vector<int>               m_destroyPending;

    int    m_nextId = 1;
    HANDLE m_hPipe = INVALID_HANDLE_VALUE;
    PROCESS_INFORMATION m_pi{};
    bool   m_startFailed = false;     // 一度失敗したら毎フレーム試さない

    std::vector<char> m_send;         // 送信バッファ（使い回し）
    std::vector<char> m_recv;
};