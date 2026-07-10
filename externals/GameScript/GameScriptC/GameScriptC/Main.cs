using System;
using System.Runtime.InteropServices;
using System.IO.Pipes;
using System.Threading.Tasks;
using Vortice.XInput;
using System.Text;
using System.Reflection;
using System.Runtime.CompilerServices;

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
            byte[] nameBuffer = new byte[1024];
            int nameLen = pipe.Read(nameBuffer, 0, nameBuffer.Length);
            string scriptName = Encoding.UTF8.GetString(nameBuffer, 0, nameLen).TrimEnd('\0');
            Console.WriteLine($"Script:{scriptName}");

            Type type = Assembly.GetExecutingAssembly().GetType(scriptName);
            if (type == null) { Console.WriteLine("Script not found"); return; }
            Templet script = (Templet)Activator.CreateInstance(type);
            script.OnStart();

            byte[] buffer = new byte[1024];
            while (true)
            {
                int bytesRead = pipe.Read(buffer, 0, buffer.Length);
                if (bytesRead <= 0) break;

                script.Update();

                pipe.Write(new byte[12], 0, 12);
            }
            pipe.Close();
        }


    }
}