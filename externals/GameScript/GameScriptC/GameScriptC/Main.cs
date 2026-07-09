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
            var server = new NamedPipeServerStream("GameScriptPipe", PipeDirection.InOut, NamedPipeServerStream.MaxAllowedServerInstances);

            // C++からのメッセージを受信するループ
            byte[] buffer = new byte[1024];
            while (true)
            {

                server.WaitForConnection();
                var temp = server;
                Task.Run(() => HandleEntity(temp));
                server = new NamedPipeServerStream("GameScriptPipe", PipeDirection.InOut, NamedPipeServerStream.MaxAllowedServerInstances);


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

        static void HandleEntity(NamedPipeServerStream pipe)
        {
            while (true)
            {
                byte[] buffer = new byte[1024];
                int bytesRead = pipe.Read(buffer, 0, buffer.Length);
                // C++ から受け取ったデータ（float[4]: pos + dt）を処理
                // ...

                // C++ に返すデータ（float[3]: 新しい位置）を送信
                // pipe.Write(...)
            }
        }


    }
}