#pragma once

#include "../ecs/world.hpp"
#include "../components/movement.hpp"
#include "application.hpp"
#include "../components/covered-cube.hpp"
#include "../components/enemy.hpp"
#include "../systems/area-coverage.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace our
{

    // The movement system is responsible for moving every entity which contains a MovementComponent.
    // This system is added as a simple example for how use the ECS framework to implement logic. 
    // For more information, see "common/components/movement.hpp"
    class MovementSystem {
    public:
        // This should be called every frame to update all entities containing a MovementComponent.
        void update(World* world, float deltaTime) {
            if(deltaTime > 0.1) return;
            // For each entity in the world
            for(auto entity : world->getEntities()){
                // Get the movement component if it exists
                auto* movement = entity->getComponent<MovementComponent>();

                // If the movement component exists
                if(movement) {
                    // Enemy rotation around itself
                    if(entity->getComponent<EnemyComponent>()){
                        auto forward_direction = glm::normalize(movement->linearVelocity);
                        glm::vec3 up_direction = {0, 1, 0};

                        // Compute the right direction by taking the cross product of forward and up directions
                        glm::vec3 right_direction = glm::normalize(glm::cross(forward_direction, up_direction));
                        double speed = sqrt(pow(movement->linearVelocity.x, 2) + pow(movement->linearVelocity.z, 2));

                        glm::mat4 rotationMatrix = glm::yawPitchRoll(0.0, -(double)(entity->localTransform.rotation.x + deltaTime * speed), 0.0);
                        entity->localTransform.rotation.x = (entity->localTransform.rotation.x - deltaTime * 10);
                        glm::mat4 new_basis = glm::mat4(glm::mat3(right_direction, up_direction, forward_direction));
                        entity->selfRotation = glm::transpose(new_basis) * rotationMatrix * new_basis;
                    }
                    else
                        entity->localTransform.rotation += deltaTime * movement->angularVelocity;

                    // Change the position based on the linear velocity and delta time.
                    entity->localTransform.position += deltaTime * movement->linearVelocity;
                }
            }
        }
    };
}