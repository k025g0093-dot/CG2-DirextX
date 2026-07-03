#pragma once
#include <vector>
#include <memory>
#include "Entity.h"

class EntityManager {
public:
    static EntityManager* GetInstance() {
        static EntityManager instance;
        return &instance;
    }

    Entity* CreateEntity(const std::string& name);
    void DestroyEntity(Entity* entity);
    void UpdateAll(float dt);
    void Clear();

    const std::vector<std::unique_ptr<Entity>>& GetEntities() const { return m_entities; }

private:
    EntityManager() = default;
    ~EntityManager() = default;
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;

    std::vector<std::unique_ptr<Entity>> m_entities;
};
