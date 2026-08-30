using System;
using System.Runtime.InteropServices;

public enum PhysicsEventType
{
    TriggerEnter = 0,
    TriggerExit = 1,
    CollisionEnter = 2,
    CollisionExit = 3,
}

// C++ の物理イベントで接触相手について受け取る情報です。
public readonly struct CollisionInfo
{
    public int EntityId { get; }
    public string EntityName { get; }

    public CollisionInfo(int entityId, string entityName)
    {
        EntityId = entityId;
        EntityName = entityName;
    }
}

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
    public virtual void GetMoveVelocity(ref float vx, ref float vz, float dt) { }

    // 物理イベント。必要なスクリプトだけ override してください。
    public virtual void OnTriggerEnter(CollisionInfo other) { }
    public virtual void OnTriggerExit(CollisionInfo other) { }
    public virtual void OnCollisionEnter(CollisionInfo other) { }
    public virtual void OnCollisionExit(CollisionInfo other) { }
}
