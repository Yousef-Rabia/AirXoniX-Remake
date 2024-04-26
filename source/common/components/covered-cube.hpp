
#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

namespace our {

    class CoveredCubeComponent : public Component {
    public:
        std::string cubeType = "Covered";
        glm::vec3 position;

        static std::string getID() { return "CoveredCube"; }

        void deserialize(const nlohmann::json& data) override;
    };

}