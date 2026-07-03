#include "Entity.h"

Entity::~Entity() {
    for (auto* c : m_components) {
        delete c;
    }
    m_components.clear();
}
