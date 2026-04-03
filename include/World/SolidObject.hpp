#ifndef SOLIDOBJECT_HPP
#define SOLIDOBJECT_HPP

#include "World/WorldObject.hpp"

enum class SolidType {
    BLOCK,
    GROUND
};

class SolidObject : public World::WorldObject {
public:
    SolidObject(SolidType solidType) 
        :   World::WorldObject(ObjectType::SOLID),
            m_SolidType(solidType) {
        

    }

    virtual ~SolidObject() = default;

private:
    SolidType m_SolidType;

};

#endif