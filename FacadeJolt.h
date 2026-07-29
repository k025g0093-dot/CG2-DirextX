#pragma once

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max

#include "JoltAllinclude.h"

#pragma pop_macro("min")
#pragma pop_macro("max")

using namespace JPH;

class FacadeJolt
{
public:
	bool Init();                    // PhysicsSystem 作成、床設置
	void Step(float dt);            // PushTransform → Update → PullTransform
	//uint32_t AddBody(ColliderDesc); // 形状と位置から Body 作成、BodyID を返す
	void RemoveBody(uint32_t id);

private:
    // --- フィルタークラス ---
    class BPLayerInterfaceImpl : public BroadPhaseLayerInterface {
        uint GetNumBroadPhaseLayers() const override { return 1; }
        BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer) const override { return BroadPhaseLayer(0); }
        const char* GetBroadPhaseLayerName(BroadPhaseLayer) const override { return "BPLayer"; }
    };
    class ObjectVsBPLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter {
        bool ShouldCollide(ObjectLayer, BroadPhaseLayer) const override { return true; }
    };
    class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter {
        bool ShouldCollide(ObjectLayer, ObjectLayer) const override { return true; }
    };

    // --- メンバー ---
    BPLayerInterfaceImpl m_bpLayerInterface;
    ObjectVsBPLayerFilterImpl m_objectVsBpFilter;
    ObjectLayerPairFilterImpl m_objectLayerPairFilter;
    TempAllocatorImpl* m_tempAllocator = nullptr;
    JobSystemThreadPool* m_jobSystem = nullptr;
    PhysicsSystem* m_physicsSystem = nullptr;
    BodyID m_floorId;

};



