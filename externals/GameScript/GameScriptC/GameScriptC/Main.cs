using System;
using System.Runtime.InteropServices;
using System.IO.Pipes;
using Vortice.XInput;

//基礎クラス、ここに各更新処理を書いてね
//ちなみにクラスを新しく作っても平気。
//最後にMain関数にもってきてくれればなんでもOK

namespace GameScriptC
{
    public class MyScript
    {

        public static void Main(string[] args)
        {
            // C++からのメッセージを受信するためのNamedPipeClientStreamを作成
            var pipeClient = new NamedPipeClientStream(".", "GameScriptPipe", PipeDirection.InOut);
            pipeClient.Connect();

            // C++からのメッセージを受信するループ
            byte[] buffer = new byte[1024];
            while (true)
            {

                if (Keyboard.IsKeyTriger(ConsoleKey.Q))
                {
                    Console.WriteLine("Qが押されてる");
                }
                if (Keyboard.IsKeyTriger(ConsoleKey.A))
                {
                    Console.WriteLine("Aが押されてる");
                }
                if (Keyboard.IsKeyTriger(ConsoleKey.S))
                {
                    Console.WriteLine("Sが押されてる");
                }
                if (Keyboard.IsKeyTriger(ConsoleKey.D))
                {
                    Console.WriteLine("Dが押されてる");
                }

                if (Keyboard.IsKeyTriger(ConsoleKey.Spacebar))
                {
                    Console.WriteLine("いざジャンプ");
                }


                

            }

            //基本はいらない
            //Console.ReadLine();
        }

        private static void Start()
        {
            //Console.WriteLine("【C#】Start関数がC++から呼び出されました！ああいったがそれは嘘だ");
        }


        private static void Update()
        {
            // Console.WriteLine("【C#】再度確認これでいけすか確認");
        }


        public static void DemoFunction()
        { }


    }
}