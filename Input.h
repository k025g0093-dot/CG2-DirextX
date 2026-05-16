#pragma once
#include <windows.h>
#include <vector>

class Input {
public:
    static void Update() {
        // キーボード
        memcpy(m_keysPrev, m_keysCurr, 256);
        GetKeyboardState(m_keysCurr);

        // ★マウス：現在位置を取得
        POINT point;
        GetCursorPos(&point);

        // 前フレームとの差分を計算
        m_mouseDeltaX = point.x - m_mouseX;
        m_mouseDeltaY = point.y - m_mouseY;

        m_mouseX = point.x;
        m_mouseY = point.y;
    }

    // キーボード
    static bool GetKey(int vKey) {
        return (m_keysCurr[vKey] & 0x80) != 0;
    }
    static bool GetKeyDown(int vKey) {
        return ((m_keysCurr[vKey] & 0x80) != 0) && ((m_keysPrev[vKey] & 0x80) == 0);
    }
    static bool GetKeyUp(int vKey) {
        return ((m_keysCurr[vKey] & 0x80) == 0) && ((m_keysPrev[vKey] & 0x80) != 0);
    }

    // ★マウス：前フレームからの移動量
    static int GetMouseDeltaX() { return m_mouseDeltaX; }
    static int GetMouseDeltaY() { return m_mouseDeltaY; }

    // ★マウスボタン
    static bool GetMouseButton(int button) {
        // 0=左, 1=右, 2=中
        int vKey = (button == 0) ? VK_LBUTTON : (button == 1) ? VK_RBUTTON : VK_MBUTTON;
        return (m_keysCurr[vKey] & 0x80) != 0;
    }

private:
    static inline BYTE m_keysCurr[256] = { 0 };
    static inline BYTE m_keysPrev[256] = { 0 };

    // ★追加：マウス管理
    static inline int m_mouseX = 0;
    static inline int m_mouseY = 0;
    static inline int m_mouseDeltaX = 0;
    static inline int m_mouseDeltaY = 0;
};