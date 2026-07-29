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
// FacadeJolt.cpp のどこかに追加
FacadeJolt* FacadeJolt::s_instance = nullptr;

FacadeJolt* FacadeJolt::GetInstance() {
    if (!s_instance) s_instance = new FacadeJolt();
    return s_instance;
}

bool FacadeJolt::Init() {
    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();
    // FacadeJolt.cpp のどこかに追加
    m_tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
    m_jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, -1);
    return true;
}