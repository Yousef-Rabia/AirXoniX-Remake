#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

namespace our {
    // This component denotes that the KeyboardMovementComponent will move the owning entity using user inputs.
    // It will also control the camera to move slightly with the player
    // We also can use this component to know the entity of the player is rather than creating a separate component
    class KeyboardMovementComponent : public Component {
    public:
        // The sensitivity parameter
        glm::vec3 positionSensitivity = {3.0f, 3.0f, 3.0f}; // The unity per second of camera movement if WASD is pressed

        // The ID of this component type is "Free Camera Controller"
        static std::string getID() { return "Keyboard Movement"; }

        // Reads sensitivities & speedupFactor from the given json object
        void deserialize(const nlohmann::json& data) override;
    };

}