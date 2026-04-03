#ifndef TRIGGEROBJECT_HPP
#define TRIGGEROBJECT_HPP

#include "World/WorldObject.hpp"

enum class TriggerType {
    PORTAL,
    PAD,
    COIN
};

class TriggerObject : public World::WorldObject {
public:
    TriggerObject(TriggerType triggerType) 
        :   World::WorldObject(ObjectType::TRIGGER),
            triggerType(triggerType) {
        

    }

    virtual ~TriggerObject() = default;

private:
    TriggerType triggerType;

};

#endif