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
            hazardType(hazardType) {
        
    }

    virtual ~HazardObject() = default;

private:
    HazardType hazardType;

};

#endif