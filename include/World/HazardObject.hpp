#ifndef HAZARDOBJECT_HPP
#define HAZARDOBJECT_HPP

#include "World/WorldObject.hpp"

enum class HazardType {
    SPIKE,
    ACID
};

class HazardObject : public World::WorldObject {
public:
    HazardObject(HazardType hazardType) 
        :   World::WorldObject(ObjectType::HAZARD),
            m_HazardType(hazardType) {
    }

    virtual ~HazardObject() = default;

private:
    HazardType m_HazardType;

};

#endif