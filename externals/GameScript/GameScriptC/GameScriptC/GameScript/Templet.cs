using System;
using System.Runtime.InteropServices;

public class Templet
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

    public virtual void OnStart() { }
    public virtual void Update() { }
    public virtual void InPostion(ref float x, ref float y, ref float z, float dt) { }
}
