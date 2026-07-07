using System;
using System.Runtime.InteropServices; 

namespace GameScriptC
{
    public class MyScript
    {
        // C++から「Start」ボタンが押されたら呼び出される関数
        [UnmanagedCallersOnly(EntryPoint = "Start")]
        public static void Start()
        {
            Console.WriteLine("【C#】Start関数がC++から呼び出されました！成功といったがあれは嘘だ！");
        }

        // C++から「Update」ボタンが押されたら呼び出される関数
        [UnmanagedCallersOnly(EntryPoint = "Update")]
        public static void Update()
        {
           // Console.WriteLine("【C#】再度確認これでいけすか確認");
        }
    }
}