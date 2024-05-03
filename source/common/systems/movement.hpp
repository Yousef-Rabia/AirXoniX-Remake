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
#include <chrono>

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
                    if(entity->getComponent<EnemyComponent>()){
                        auto linear = movement->linearVelocity;
                        movement->angularVelocity = glm::vec3{0, linear.x * 2, 0};
                    }

                    // Change the position and rotation based on the linear & angular velocity and delta time.
                    entity->localTransform.position += deltaTime * movement->linearVelocity;
                    entity->localTransform.rotation += deltaTime * movement->angularVelocity;
                }
            }
        }
    };
}