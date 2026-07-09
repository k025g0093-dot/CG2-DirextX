using System;
using System.Runtime.InteropServices;


//キーボード操作などで使う者たちです

//ここは特殊キー（WASDとは別）のなんか異常者たちの宣言方法
//ここからコピッテ使って下さい
//{
//ConsoleKey.asdasdSpacebar;    // スペース
//ConsoleKey.Escape;      // ESC
//ConsoleKey.Enter;       // Enter
//ConsoleKey.Tab;         // Tab
//ConsoleKey.Backspace;   // Backspace
//ConsoleKey.Delete;      // Delete
//ConsoleKey.Insert;      // Insert
//ConsoleKey.Home;        // Home
//ConsoleKey.End;         // End
//ConsoleKey.PageUp;      // PageUp
//ConsoleKey.PageDown;    // PageDown
//}


namespace GameScriptC
{
    internal class Keyboard
    {
        [DllImport("user32.dll")]
        static extern short GetAsyncKeyState(int vKey);

        public static bool IsKeyDown(ConsoleKey key)
        {
            return (GetAsyncKeyState((int)key) & 0x8000) != 0;
        }

        static Dictionary<ConsoleKey, bool> prev = new();
        public static bool IsKeyTriger(ConsoleKey key)
        {
            bool now = (GetAsyncKeyState((int)key) & 0x8000)!=0;

            prev.TryGetValue(key, out bool was);
            prev[key]=now;
            return now&&!was;

        }
    }
}
