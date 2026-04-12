#ifndef SOLIDOBJECT_HPP
#define SOLIDOBJECT_HPP

#include "World/WorldObject.hpp"

enum class SolidType {
    BLOCK,
    GROUND
};

class SolidObject : public World::WorldObject {
public:
    explicit SolidObject(SolidType solidType)
        : World::WorldObject(ObjectType::SOLID),
          m_SolidType(solidType) {
    }

    ~SolidObject() override = default;

    SolidType getSolidType() const {
        return m_SolidType;
    }

private:
    SolidType m_SolidType;
};

class BlockObject : public SolidObject {
public:
    BlockObject()
        : SolidObject(SolidType::BLOCK) {
    }

    ~BlockObject() override = default;
};

class GroundObject : public SolidObject {
public:
    GroundObject()
        : SolidObject(SolidType::GROUND) {
    }

    ~GroundObject() override = default;
};

#endif
