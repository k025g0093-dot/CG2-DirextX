#include "EntityManager.h"
#include "TUFEngine.h"

Entity* EntityManager::CreateEntity(const std::string& name) {
    auto entity = std::make_unique<Entity>();
    entity->name = name;
    Entity* ptr = entity.get();
    m_entities.push_back(std::move(entity));
    return ptr;
}

void EntityManager::DestroyEntity(Entity* entity) {
    auto it = std::find_if(m_entities.begin(), m_entities.end(),
        [entity](const auto& e) { return e.get() == entity; });
    if (it != m_entities.end()) {
        m_entities.erase(it);
    }
}

void EntityManager::UpdateAll(float dt) {
    for (auto& entity : m_entities) {
        for (auto* c : entity->m_components) {
            if (!c) continue;
            c->Update();
        }
    }
}

void EntityManager::Clear() {
    m_entities.clear();
}
