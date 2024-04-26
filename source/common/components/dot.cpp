#include "dot.hpp"
#include "../ecs/entity.hpp"
#include <iostream>
namespace our {
    void DotComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        dotType = data.value("Dot", dotType);

    }
}