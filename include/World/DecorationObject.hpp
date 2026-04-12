#ifndef DECORATIONOBJECT_HPP
#define DECORATIONOBJECT_HPP

#include "World/WorldObject.hpp"

enum class DecorationType {
    BEACON,
};

class DecorationObject : public World::WorldObject {
public:
    explicit DecorationObject(DecorationType decorationType)
        : World::WorldObject(ObjectType::DECORATION),
          m_DecorationType(decorationType) {
    }

    ~DecorationObject() override = default;

    DecorationType getDecorationType() const {
        return m_DecorationType;
    }

private:
    DecorationType m_DecorationType;
};

#endif
