#pragma once

#include "../ecs/world.hpp"
#include "../components/movement.hpp"
#include "application.hpp"
#include "../components/covered-cube.hpp"
#include "../components/enemy.hpp"

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
        // Define a map to store the last wall collision time for each entity (separate from entity x entity collisions)
        std::unordered_map<Entity*, std::chrono::steady_clock::time_point> lastCollisionTimes;
        std::unordered_map<Entity*, Entity*> nearestCube;

        // This should be called every frame to update all entities containing a MovementComponent.
        void update(World* world, float deltaTime) {
            if(deltaTime > 0.1) return;
            // For each entity in the world
            for(auto entity : world->getEntities()){
                // Get the movement component if it exists
                MovementComponent* movement = entity->getComponent<MovementComponent>();
                EnemyComponent* enemy = entity->getComponent<EnemyComponent>();
                // If the movement component exists
                if(movement) {
                    glm::vec3& entityPosition =  entity->localTransform.position;
                    // Change the position and rotation based on the linear & angular velocity and delta time.
                    entityPosition += deltaTime * movement->linearVelocity;
                    entity->localTransform.rotation += deltaTime * movement->angularVelocity;

                    // outer wall collision
                    if(enemy){
                        // Collision with created walls
                        double leastDistance = 100;
                        double distance;
                        bool cubeCollision = false;
                        for(auto cube : world->getEntities()){
                            if (cube->getComponent<CoveredCubeComponent>())
                            {
                                glm::vec3& cubePosition = cube->localTransform.position;
                                if(cubePosition.y < 0) continue;

                                auto currentTime = std::chrono::steady_clock::now();
                                auto it = lastCollisionTimes.find(entity);

                                distance = pow(cubePosition.x - entityPosition.x, 2) + pow(cubePosition.z - entityPosition.z, 2);
                                if(distance <= 3){
                                    if (it == lastCollisionTimes.end() || (currentTime - it->second) >= std::chrono::milliseconds (75)) {
                                        if(distance < leastDistance)
                                        {
                                            cubeCollision = true;
                                            leastDistance = distance;
                                            nearestCube[entity] = cube;
                                        }
                                    }
                                }
                            }
                        }
                        if(cubeCollision)
                        {
                            glm::vec3& cubePosition = nearestCube[entity]->localTransform.position;
                            auto currentTime = std::chrono::steady_clock::now();

                            // Update the last collision time for this entity
                            lastCollisionTimes[entity] = currentTime;
                            // Reverse its linear velocity depending on where it hit the wall
                            if(abs(cubePosition.x - entityPosition.x) > abs(cubePosition.z - entityPosition.z))
                                movement->linearVelocity.x *= -1;
                            else
                                movement->linearVelocity.z *= -1;
                        }
                    }
                }
            }
        }
    };
}