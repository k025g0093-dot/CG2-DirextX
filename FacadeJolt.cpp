#include "FacadeJolt.h"

// これだけは必ず include より前
#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

// ここで Jolt の include（もうしてるので不要）

#pragma pop_macro("min")
#pragma pop_macro("max")

// ---- レイヤー定義（クラス局部でOK） ----
namespace Layers {
    static constexpr JPH::ObjectLayer NON_MOVING = 0;
    static constexpr JPH::ObjectLayer MOVING = 1;
};

bool FacadeJolt::Init() {
    RegisterDefaultAllocator();
    Factory::sInstance = new Factory();
    RegisterTypes();
    return true;
}