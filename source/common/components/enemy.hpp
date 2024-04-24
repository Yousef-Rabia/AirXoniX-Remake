
#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

namespace our {

    class EnemyComponent : public Component {
    public:
        // Enemy types are:
        // Ball: which moves at the bottom layer
        // Mine: which moves at the top layer
        // Enemies colliding with the player will cause death
        std::string enemyType = "Ball";

        // The ID of this component type is "Enemy"
        static std::string getID() { return "Enemy"; }

        void deserialize(const nlohmann::json& data) override;
    };

}