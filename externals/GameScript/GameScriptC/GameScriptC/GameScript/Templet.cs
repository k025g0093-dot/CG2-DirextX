using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public enum PhysicsEventType
{
    TriggerEnter = 0,
    TriggerExit = 1,
    CollisionEnter = 2,
    CollisionExit = 3,
}

public readonly struct ScriptVector3
{
    public float X { get; }
    public float Y { get; }
    public float Z { get; }

    public ScriptVector3(float x, float y, float z)
    {
        X = x;
        Y = y;
        Z = z;
    }

    public static ScriptVector3 operator -(ScriptVector3 a, ScriptVector3 b)
        => new ScriptVector3(a.X - b.X, a.Y - b.Y, a.Z - b.Z);

    public float LengthXZ()
        => MathF.Sqrt(X * X + Z * Z);
}

public readonly struct EntityInfo
{
    public string Name { get; }
    public ScriptVector3 Position { get; }

    public EntityInfo(string name, ScriptVector3 position)
    {
        Name = name;
        Position = position;
    }
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

public static class World
{
    // 名前からEntity情報を探す。見つからなければ null。
    public static EntityInfo? FindByName(string name)
    {
        return s_entities.TryGetValue(name, out var entity) ? entity : null;
    }

    // Main.csだけが呼ぶ。C++から届いた一覧で毎フレーム更新する。
    internal static void UpdateEntity(string name, float x, float y, float z)
    {
        s_entities[name] = new EntityInfo(name, new ScriptVector3(x, y, z));
    }

    internal static void BeginFrame()
    {
        s_entities.Clear();
    }

    private static readonly Dictionary<string, EntityInfo> s_entities = new();
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

    // ── 接触したときに「上に飛ぶ」ための仕組み ──
    // スクリプト側から Jump(速度) を呼ぶとフラグが立ち、
    // Main.cs がそれを mode 2 コマンドとして C++ に送り、Jolt の Y 速度が上書きされる。
    private float m_pendingJumpVelocityY = 0.0f;
    private bool m_hasPendingJump = false;

    /// <summary>Y方向の速度を指定して上に飛ばす。指定した値がそのまま Jolt の速度Yになる。</summary>
    protected void Jump(float velocityY)
    {
        m_pendingJumpVelocityY = velocityY;
        m_hasPendingJump = true;
    }

    /// <summary>Main.cs が使う。ジャンプ要求が溜まっているか。</summary>
    public bool HasPendingJump => m_hasPendingJump;

    /// <summary>Main.cs が使う。ジャンプ要求を取り出してクリアする。</summary>
    public float ConsumeJump()
    {
        float v = m_pendingJumpVelocityY;
        m_hasPendingJump = false;
        m_pendingJumpVelocityY = 0.0f;
        return v;
    }

    // C++ 側から毎フレーム渡される、このスクリプトを持つ Entity 自身の座標です。
    // 読み取り用として使い、移動は GetMoveVelocity() / Jump() で要求してください。
    public ScriptVector3 Position { get; internal set; }

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
