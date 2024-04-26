
#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

namespace our {

    class DotComponent : public Component {
    public:
        std::string dotType = "Dot";
        glm::vec3 position;

        static std::string getID() { return "Dot"; }

        void deserialize(const nlohmann::json& data) override;
    };

}