using System;

public class BoxEntity : Templet
{
    public int Id { get; set; }
    public string Name { get; set; }

    public override void OnStart() { }

    public override void Update() 
    {
        if (IsKeyDown(ConsoleKey.W))
            Console.WriteLine("W is held");
        if (IsKeyDown(ConsoleKey.A))
            Console.WriteLine("A is held");
        if (IsKeyDown(ConsoleKey.S))
            Console.WriteLine("S is held");
        if (IsKeyDown(ConsoleKey.D))
            Console.WriteLine("D is held");
        if (IsKeyPressed(ConsoleKey.Spacebar))
            Console.WriteLine("Space pressed");
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

    public override void InPostion(ref float x, ref float y, ref float z, float dt) { }
}