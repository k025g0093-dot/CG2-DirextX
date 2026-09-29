using System;

// ==============================================
// PlayerController
// ==============================================
// C++側のGameScriptコンポーネントで Script Name を
// 「PlayerController」にすると、このクラスが呼ばれます。
//
// よく使う機能:
//
// 自分の座標:
//   Position.X / Position.Y / Position.Z
//
// 他Entityを名前で探す:
//   var player = FindEntity("Player");
//   if (player.HasValue)
//   {
//       var playerPos = player.Value.Position;
//   }
//
// 移動:
//   GetMoveVelocity() で vx, vz を設定する。
//   物理Bodyを持つEntityなら、C++ / Jolt側に速度として反映される。
//
// 衝突・Trigger:
//   OnTriggerEnter / OnCollisionEnter を override する。
//   other.Name で接触相手の名前を調べられる。
// ==============================================

public class PlayerController : Templet
{
    // ゲーム開始時に一度だけ呼ばれる
    public override void OnStart()
    {
    }

    // 毎フレーム呼ばれる。
    // 入力、状態遷移、攻撃クールダウンなどを書く。
    public override void Update()
    {
    }

    // 移動速度を決める。
    // vx はX方向、vzはZ方向の速度。
    // WASDキーで移動する。
    public override void GetMoveVelocity(ref float vx, ref float vz, float dt)
    {
        const float speed = 4.0f;

        float inputX = 0.0f;
        float inputZ = 0.0f;

        if (IsKeyDown(ConsoleKey.W)) inputZ += 1.0f;
        if (IsKeyDown(ConsoleKey.S)) inputZ -= 1.0f;
        if (IsKeyDown(ConsoleKey.D)) inputX += 1.0f;
        if (IsKeyDown(ConsoleKey.A)) inputX -= 1.0f;

        // 入力が無ければ移動しない
        if (inputX == 0.0f && inputZ == 0.0f)
        {
            return;
        }

        // 斜め移動でも速度が伸びないように正規化
        float length = MathF.Sqrt(inputX * inputX + inputZ * inputZ);

        vx = inputX / length * speed;
        vz = inputZ / length * speed;
    }

    // Triggerに入った瞬間
    public override void OnTriggerEnter(CollisionInfo other)
    {
        if (other.EntityName == "Player")
        {
            // プレイヤーが索敵範囲に入った時の処理
        }
    }

    // Triggerから出た瞬間
    public override void OnTriggerExit(CollisionInfo other)
    {
    }

    // 物理衝突した瞬間
    public override void OnCollisionEnter(CollisionInfo other)
    {
        if (other.EntityName == "Player")
        {
            // プレイヤーと接触した時の処理
        }
    }

    // 物理衝突が終わった瞬間
    public override void OnCollisionExit(CollisionInfo other)
    {
    }
}
