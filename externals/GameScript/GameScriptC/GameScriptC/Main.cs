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
            string pipeName = "GameScriptPipe_" + args[0];
            var server = new NamedPipeServerStream(pipeName, PipeDirection.InOut);
            Console.WriteLine("接続待機中...");
            server.WaitForConnection();
            Console.WriteLine("接続されました");
            HandleEntity(server);
        }

        static void HandleEntity(NamedPipeServerStream pipe)
        {
            byte[] nameBuffer = new byte[1024];
            int nameLen = pipe.Read(nameBuffer, 0, nameBuffer.Length);
            string scriptName = Encoding.UTF8.GetString(nameBuffer, 0, nameLen).TrimEnd('\0');
            Console.WriteLine($"Script:{scriptName}");

            Type type = Assembly.GetExecutingAssembly().GetType(scriptName);
            if (type == null) {
                Console.WriteLine($"Script '{scriptName}' not found. Loaded types:");
                foreach (var t in Assembly.GetExecutingAssembly().GetTypes())
                    Console.WriteLine($"  {t.FullName}");
                return;
            }
            Templet script = (Templet)Activator.CreateInstance(type);
            script.OnStart();

            byte[] buf = new byte[16];
            while (true)
            {
                int bytesRead = pipe.Read(buf, 0, buf.Length);
                if (bytesRead < 16) break;

                float x = BitConverter.ToSingle(buf, 0);
                float y = BitConverter.ToSingle(buf, 4);
                float z = BitConverter.ToSingle(buf, 8);
                float dt = BitConverter.ToSingle(buf, 12);

                script.InPostion(ref x, ref y, ref z, dt);

                byte[] outBuf = new byte[12];
                Buffer.BlockCopy(BitConverter.GetBytes(x), 0, outBuf, 0, 4);
                Buffer.BlockCopy(BitConverter.GetBytes(y), 0, outBuf, 4, 4);
                Buffer.BlockCopy(BitConverter.GetBytes(z), 0, outBuf, 8, 4);
                pipe.Write(outBuf, 0, 12);
            }
            pipe.Close();
    }
}

public static class KeyboardHelper
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
}
}