#ifndef TRIGGEROBJECT_HPP
#define TRIGGEROBJECT_HPP

#include "World/WorldObject.hpp"

#include <algorithm>

enum class TriggerType {
    PORTAL,
    PAD,
    COIN
};

class TriggerObject : public World::WorldObject {
public:
    explicit TriggerObject(TriggerType triggerType)
        : World::WorldObject(ObjectType::TRIGGER),
          m_TriggerType(triggerType) {
    }

    ~TriggerObject() override = default;

    TriggerType getTriggerType() const {
        return m_TriggerType;
    }

private:
    TriggerType m_TriggerType;
};

class PortalObject : public TriggerObject {
public:
    PortalObject()
        : TriggerObject(TriggerType::PORTAL) {
    }

    ~PortalObject() override = default;
};

class PadObject : public TriggerObject {
public:
    PadObject()
        : TriggerObject(TriggerType::PAD) {
    }

    ~PadObject() override = default;
};

class CoinObject : public TriggerObject {
public:
    CoinObject()
        : TriggerObject(TriggerType::COIN) {
    }

    ~CoinObject() override = default;

    World::CollisionGeometry getCollisionGeometry() const override {
        World::CircleGeometry circle;
        circle.center = getPosition();
        circle.radius = std::min(getSize().x, getSize().y) * 0.5f;
        return circle;
    }
};

#endif
