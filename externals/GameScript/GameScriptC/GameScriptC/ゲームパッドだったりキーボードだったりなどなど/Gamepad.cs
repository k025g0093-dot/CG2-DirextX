//using System;
//using System.Collections.Generic;
//using System.Text;
//using Vortice.XInput;

//ゲームパット管理クラスです
//主な使い方は下のとうりです、確認してみてね！



//namespace GameScriptC
//{
//    internal class GamepadClass
//    {

//        public int? GamepadIndex { get; set; }

//        public State? GetKeystate()
//        {
//            // 認識済みの場合、認識済みのゲームパッドを使う
//            if (GamepadIndex != null)
//            {
//                if (XInput.GetState(GamepadIndex.Value, out var keystate))
//                    return keystate;
//                else
//                    // 認識済みのゲームパッドが無効になったとみなす
//                    GamepadIndex = null;
//            }
//            else
//                // 未認識の場合、0 ～ 3 の順で有効なゲームパッドを探す
//                for (var i = 0; i < 4; ++i)
//                    if (XInput.GetState(i, out var keystate))
//                    {
//                        GamepadIndex = i;
//                        return keystate;
//                    }

//            return null;
//        }


//        public static void GamePadUpdate()
//        {
//            // XInputの状態を取得
//            var state = GamePad.GetState(0);
//            // ボタンの状態を確認
//            if (state.IsConnected)
//            {
//                if (state.Buttons.HasFlag(GamePadButtonFlags.A))
//                {
//                    Console.WriteLine("Aボタンが押されました");
//                }
//                if (state.Buttons.HasFlag(GamePadButtonFlags.B))
//                {
//                    Console.WriteLine("Bボタンが押されました");
//                }
//                if (state.Buttons.HasFlag(GamePadButtonFlags.X))
//                {
//                    Console.WriteLine("Xボタンが押されました");
//                }
//                if (state.Buttons.HasFlag(GamePadButtonFlags.Y))
//                {
//                    Console.WriteLine("Yボタンが押されました");
//                }

//                //RLボタン関係
//                if (state.Buttons.HasFlag(GamePadButtonFlags.LeftShoulder))
//                {
//                    Console.WriteLine("LBボタンが押されました");
//                }
//                if (state.Buttons.HasFlag(GamePadButtonFlags.RightShoulder))
//                {
//                    Console.WriteLine("LBボタンが押されました");
//                }

//                //特殊ボタンの状態を確認
//                if (state.Buttons.HasFlag(GamePadButtonFlags.Start))
//                {
//                    Console.WriteLine("Startボタンが押されました");
//                }

//            }

//            // 左スティックの状態を確認
//            if (state.LeftThumbX>0)
//            {
//                Console.WriteLine("左スティックが右に倒されました");
//            }
//            else if (state.LeftThumbX < 0)
//            {
//                Console.WriteLine("左スティックが左に倒されました");
//            }

//            if (state.LeftThumbY>0)
//            {
//                Console.WriteLine("左スティックが上に倒されました");
//            }
//            else if (state.LeftThumbY < 0)
//            {
//                Console.WriteLine("左スティックが下に倒されました");
//            }


//            // 右スティックの状態を確認
//            if (state.RightThumbX>0)
//            {
//                Console.WriteLine("右スティックが右に倒されました");

//            }
//            else if (state.RightThumbX < 0)
//            {
//                Console.WriteLine("右スティックが左に倒されました");
//            }

//            if (state.RightThumbY>0)
//            {
//                Console.WriteLine("右スティックが上に倒されました");
//            }
//            else if (state.RightThumbY < 0)
//            {
//                Console.WriteLine("右スティックが下に倒されました");
//            }

//        }

//    }
//}
