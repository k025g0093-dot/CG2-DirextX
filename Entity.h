#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Component.h"
#include "Transform.h"
#include "Create3DObjectOBB.h"



class Entity {
public:


    uint32_t m_bodyIdRaw = UINT32_MAX;

    Entity();
    ~Entity();

    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        T* comp = new T(std::forward<Args>(args)...);
        m_components.push_back(comp);
        return comp;
    }

    template<typename T>
    T* GetComponent() {
        for (auto* c : m_components) {
            T* casted = dynamic_cast<T*>(c);
            if (casted) return casted;
        }
        return nullptr;
    }

    void SetDisplayName(const std::string& n) {
        displayName = n;
        strncpy_s(displayNameBuf, n.c_str(), sizeof(displayNameBuf));
    }

    // Entity.h
    void SetName(const std::string& newName) { name = newName; }
    const std::string& GetName() const { return name; }

    std::string name;
    std::string displayName;
    char displayNameBuf[128]{};


    Transform transform;

	bool isSelected = false;

    const std::vector<Component*>& GetComponents() const { return m_components; }

    // Entity.h に追加
    OBB obb;
    AABB localAABB;

private:
    friend class EntityManager;
    std::vector<Component*> m_components;
};
