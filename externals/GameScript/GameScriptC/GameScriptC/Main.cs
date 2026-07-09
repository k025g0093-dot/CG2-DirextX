using System;
using System.Runtime.InteropServices;
using System.IO.Pipes;
using System.Threading.Tasks;
using Vortice.XInput;

namespace GameScriptC
{
    public class MyScript
    {
        [DllImport("kernel32.dll")]
        static extern bool AllocConsole();

        public static void Main(string[] args)
        {
            AllocConsole();
            Console.WriteLine("C# Start");

            var server = new NamedPipeServerStream("GameScriptPipe", PipeDirection.InOut, NamedPipeServerStream.MaxAllowedServerInstances);
            byte[] buffer = new byte[1024];
            while (true)
            {
                Console.WriteLine("接続待機中...");
                server.WaitForConnection();
                Console.WriteLine("接続されました");
                var temp = server;
                Task.Run(() => HandleEntity(temp));
                server = new NamedPipeServerStream("GameScriptPipe", PipeDirection.InOut, NamedPipeServerStream.MaxAllowedServerInstances);
            }
        }

        static void HandleEntity(NamedPipeServerStream pipe)
        {
            Console.WriteLine("HandleEntity Start");
            byte[] buffer = new byte[16];
            while (true)
            {
                int bytesRead = pipe.Read(buffer, 0, buffer.Length);
                //Console.WriteLine("受信: " + bytesRead + " bytes");
                if (bytesRead <= 0) break;

                if (Keyboard.IsKeyTriger(ConsoleKey.Q))
                    Console.WriteLine("Qが押されてる");
                if (Keyboard.IsKeyTriger(ConsoleKey.A))
                    Console.WriteLine("Aが押されてる");
                if (Keyboard.IsKeyTriger(ConsoleKey.S))
                    Console.WriteLine("Sが押されてる");
                if (Keyboard.IsKeyTriger(ConsoleKey.D))
                    Console.WriteLine("Dが押されてる");
                if (Keyboard.IsKeyTriger(ConsoleKey.Spacebar))
                    Console.WriteLine("いざジャンプ");

                pipe.Write(new byte[12], 0, 12);
            }
            pipe.Close();
            Console.WriteLine("HandleEntity End");
        }
    }
}