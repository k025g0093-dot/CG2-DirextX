using System;
using System.Collections.Generic;
using System.Text;

public class PlayerState : Templet
{
    public int Id { get; set; }
    public string Name { get; set; }

    public void OnStart() { }

    public void Update() { }

    // 位置同期のために ref + dt が必要
    public void InPostion(ref float x, ref float y, ref float z, float dt) { }
}
