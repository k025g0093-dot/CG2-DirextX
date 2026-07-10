using System;
using System.Collections.Generic;
using System.Text;

public class Templet
{
    public virtual void OnStart() { }
    public virtual void Update() { }
    public virtual void InPostion(ref float x, ref float y, ref float z, float dt) { }
}
