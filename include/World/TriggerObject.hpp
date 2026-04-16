#ifndef TRIGGEROBJECT_HPP
#define TRIGGEROBJECT_HPP

#include "World/WorldObject.hpp"
#include "World/Character.hpp"

#include <algorithm>

enum class TriggerType {
    PAD,
    COIN,
    PORTAL,
    BACKGROUND_COLOR
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

    bool isTriggered() const {
        return m_IsTriggered;
    }

    void setTriggered(const bool triggered) {
        m_IsTriggered = triggered;
    }

private:
    TriggerType m_TriggerType;
    bool m_IsTriggered = false;
};

class PortalObject : public TriggerObject { // 放置重複觸發
public:
    PortalObject()
        : TriggerObject(TriggerType::PORTAL) {
    }

    ~PortalObject() override = default;

    CharacterType getTargetCharacterType() const {
        return m_TargetCharacterType;
    }

    void setTargetCharacterType(const CharacterType targetCharacterType) {
        m_TargetCharacterType = targetCharacterType;
    }

private:
    CharacterType m_TargetCharacterType = CharacterType::CUBE;
};

class BackgroundColorObject : public TriggerObject {
public:
    BackgroundColorObject()
        : TriggerObject(TriggerType::BACKGROUND_COLOR) {
    }

    const Util::Color& getTargetColor() const { return m_TargetColor; }
    void setTargetColor(const Util::Color& color) { m_TargetColor = color; }

    float getDuration() const { return m_Duration; }
    void setDuration(const float duration) { m_Duration = duration; }

private:
    Util::Color m_TargetColor = Util::Color::FromRGB(0, 0, 0);
    float m_Duration = 0.0f;
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
