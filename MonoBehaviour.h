#pragma once
#include "Component.h"

class MonoBehaviour : public Component
{
public:
    MonoBehaviour();
    virtual ~MonoBehaviour();

    virtual void Start();
    virtual void Update();
    virtual void FixedUpdate();

private:
    bool m_isStarted;
};
