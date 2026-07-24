#pragma once

//コンポーネントの基底クラス
//ここが基盤でここから別別に派生していく


class Component
{
public:
    virtual ~Component() = default;
    virtual void Start() {}
    virtual void Update() {}
    virtual void FixedUpdate() {}
    virtual Component* Clone() const = 0;
};


