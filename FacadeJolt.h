#pragma once

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

#include "JoltAllinclude.h"

#pragma pop_macro("min")
#pragma pop_macro("max")


class FacadeJolt
{
public:

    static FacadeJolt* GetInstance();


	bool Init();                    // PhysicsSystem 作成、床設置
	void Step(float dt);            // PushTransform → Update → PullTransform
	//uint32_t AddBody(ColliderDesc); // 形状と位置から Body 作成、BodyID を返す
	void RemoveBody(uint32_t id);

private:

    static FacadeJolt* s_instance;

    // --- フィルタークラス ---
    class BPLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface {
        JPH::uint GetNumBroadPhaseLayers() const override { return 1; }
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer) const override { return JPH::BroadPhaseLayer(0); }
        const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer) const override { return "BPLayer"; }
    };
    class ObjectVsBPLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
        bool ShouldCollide(JPH::ObjectLayer, JPH::BroadPhaseLayer) const override { return true; }
    };
    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
        bool ShouldCollide(JPH::ObjectLayer, JPH::ObjectLayer) const override { return true; }
    };

    // --- メンバー ---
    BPLayerInterfaceImpl m_bpLayerInterface;
    ObjectVsBPLayerFilterImpl m_objectVsBpFilter;
    ObjectLayerPairFilterImpl m_objectLayerPairFilter;
    JPH::TempAllocatorImpl* m_tempAllocator = nullptr;
    JPH::JobSystemThreadPool* m_jobSystem = nullptr;
    JPH::PhysicsSystem* m_physicsSystem = nullptr;
    JPH::BodyID m_floorId;

};



