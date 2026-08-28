#include "EntityManager.h"
#include "TUFEngine.h"
#include "MonoBehaviour.h"

Entity* EntityManager::CreateEntity(const std::string& name) {
	auto entity = std::make_unique<Entity>();
	entity->name = name;
	entity->displayName = name;
	strncpy_s(entity->displayNameBuf, name.c_str(), sizeof(entity->displayNameBuf));

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
			if (!c)continue;
			if (auto* mb = dynamic_cast<MonoBehaviour*>(c)) {
				if (!mb->m_isStarted)mb->Start();
			}
		}
	}

	m_accumlator += dt;
	int steps = 0;
	while (m_accumlator >= kFixedTimeStep && steps < kMaxFixedSteps) {

		//FacadeJolt::GetInstance()->Step(kFixedTimeStep);未来のjolt

		for (auto& entity : m_entities) {
			for (auto* c : entity->m_components) {
				if (!c)continue;
				c->fixedDeltaTime = kFixedTimeStep;
				c->FixedUpdate();
			}
		}
		m_accumlator -= kFixedTimeStep;
		++steps;
	}

	if (steps >= kMaxFixedSteps)m_accumlator = 0.0f;

	for (auto& entity : m_entities) {
		for (auto* c : entity->m_components) {
			if (!c)continue;
			c->deltaTime = dt;
			c->Update();
		}
	}

}

void EntityManager::Clear() {
	m_entities.clear();
}

Entity* EntityManager::DuplicateEntity(Entity* src) {
	auto* entity = CreateEntity(src->name + " (コピー)");
	entity->displayName = src->displayName + " (コピー)";
	strncpy_s(entity->displayNameBuf, entity->displayName.c_str(), sizeof(entity->displayNameBuf));
	entity->transform = src->transform;
	entity->obb = src->obb;
	for (auto* c : src->GetComponents()) {
		Component* cloned = c->Clone();
		cloned->entity = entity;
		entity->m_components.push_back(c->Clone());
	}
	return entity;
}
