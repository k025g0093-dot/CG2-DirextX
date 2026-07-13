using System;

public class namanamako : Templet
{
    public int Id { get; set; }
    public string Name { get; set; }

    public override void OnStart() { }

    public override void Update() 
    {
        if (IsKeyTriger(ConsoleKey.W))
            Console.WriteLine("W is held");
        if (IsKeyTriger(ConsoleKey.A))
            Console.WriteLine("A is held");
        if (IsKeyTriger(ConsoleKey.S))
            Console.WriteLine("S is held");
        if (IsKeyDown(ConsoleKey.D))
            IsKeyTriger.WriteLine("D is held");
        if (IsKeyTriger(ConsoleKey.Spacebar))
            Console.WriteLine("Space pressed");
    }

    if (Gamepad.IsButtonDown(GamepadButtonFlags.A))
        {
            Console.WriteLine("プレイヤーがジャンプ！");
        }



//キーボードの処理について説明します
// public override void Update() 
    //{
//IsKeyDownは長押しの処理の際に使います
        //if (IsKeyDown(ConsoleKey.W))//
       // if (IsKeyDown(ConsoleKey.A))
        //if (IsKeyDown(ConsoleKey.S))
      //  if (IsKeyDown(ConsoleKey.D))
//IsKeyPressedは押し込んだ際に実行します
    //    if (IsKeyPressed(ConsoleKey.Spacebar))
  //  }


  //次にコントローラーの操作になります
  //if (Gamepad.IsButtonDown(GamepadButtonFlags.A))
        //{
          //  Console.WriteLine("プレイヤーがジャンプ！");
        //}
        //このような感じで使います


    public override void InPostion(ref float x, ref float y, ref float z, float dt) { }
}