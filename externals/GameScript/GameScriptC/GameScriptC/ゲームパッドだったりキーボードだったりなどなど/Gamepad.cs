using System;
using Vortice.XInput;

public static class Gamepad
{
    static int? _index;

    static Gamepad GetState()
    {
        if (_index != null && XInput.GetState(_index.Value, out var state))
            return state.Gamepad;
        for (int i = 0; i < 4; i++)
            if (XInput.GetState(i, out var state))
            { _index = i; return state.Gamepad; }
        _index = null;
        return default;
    }

    public static bool IsConnected()
    {
        var gp = GetState();
        return gp.Buttons != GamePadButtons.None || gp.LeftThumbX != 0 || gp.LeftThumbY != 0;
    }

    public static bool IsButtonDown(GamePadButtons button)
    {
        return GetState().Buttons.HasFlag(button);
    }

    public static float LeftX() => GetState().LeftThumbX / 32768f;
    public static float LeftY() => GetState().LeftThumbY / 32768f;
    public static float RightX() => GetState().RightThumbX / 32768f;
    public static float RightY() => GetState().RightThumbY / 32768f;

    public static void DebugPrintState()
    {
        var gp = GetState();

        if (gp.Buttons.HasFlag(GamePadButtons.A)) Console.WriteLine("A");
        if (gp.Buttons.HasFlag(GamePadButtons.B)) Console.WriteLine("B");
        if (gp.Buttons.HasFlag(GamePadButtons.X)) Console.WriteLine("X");
        if (gp.Buttons.HasFlag(GamePadButtons.Y)) Console.WriteLine("Y");
        if (gp.Buttons.HasFlag(GamePadButtons.LeftShoulder)) Console.WriteLine("LB");
        if (gp.Buttons.HasFlag(GamePadButtons.RightShoulder)) Console.WriteLine("RB");
        if (gp.Buttons.HasFlag(GamePadButtons.Start)) Console.WriteLine("Start");

        if (gp.LeftThumbX > 0) Console.WriteLine("Left stick right");
        else if (gp.LeftThumbX < 0) Console.WriteLine("Left stick left");
        if (gp.LeftThumbY > 0) Console.WriteLine("Left stick up");
        else if (gp.LeftThumbY < 0) Console.WriteLine("Left stick down");
        if (gp.RightThumbX > 0) Console.WriteLine("Right stick right");
        else if (gp.RightThumbX < 0) Console.WriteLine("Right stick left");
        if (gp.RightThumbY > 0) Console.WriteLine("Right stick up");
        else if (gp.RightThumbY < 0) Console.WriteLine("Right stick down");
    }
}
