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

    public static bool IsKeyPressed(ConsoleKey key)
    {
        return (GetAsyncKeyState((int)key) & 0x0001) != 0;
    }

    public virtual void OnStart() { }
    public virtual void Update() { }
    public virtual void InPostion(ref float x, ref float y, ref float z, float dt) { }
}
