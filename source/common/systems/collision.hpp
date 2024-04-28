#pragma once

#include "../ecs/world.hpp"
#include "../components/movement.hpp"
#include "../components/enemy.hpp"
#include "../components/keyboard-movement.hpp"
#include "../components/enemy.hpp"
#include "application.hpp"
#include "components/camera.hpp"
#include "components/dot.hpp"
#include "../systems/area-coverage.hpp"


#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <chrono>

namespace our
{

    // The collision system is responsible for the effect of any two entities colliding.
    class CollisionSystem {
    public:

        const glm::vec3 RESET_DOT = glm::vec3(10, -3.05 ,15);

        // Define a map to store the last collision time for each entity
        std::unordered_map<Entity*, std::chrono::steady_clock::time_point> lastCollisionTimes;

        // Define a map to store the last wall collisions for each entity (separate from entity x entity collisions)
        std::unordered_map<Entity*, Entity*> nearestCube;
        std::unordered_map<Entity*, Entity*> latestCube;

        // This should be called every frame to update all entities containing a MovementComponent.
        void update(World* world, AreaCoverageSystem *areaCoverageSystem) {
            // get the player (later used for collision calculations)
            KeyboardMovementComponent *move = nullptr;
            for(auto entity : world->getEntities()){
                move = entity->getComponent<KeyboardMovementComponent>();
                if(move) break;
            }

            // we also get the camera entity since we need to reset it on player death
            CameraComponent* camera = nullptr;
            for(auto cameraEntity : world->getEntities()){
                camera = cameraEntity->getComponent<CameraComponent>();
                if(camera) break;
            }
            Entity* cameraEntity = camera->getOwner();
            glm::vec3& cameraPosition = cameraEntity->localTransform.position;

            // For each entity in the world
            for(auto entity : world->getEntities()){
                // Get the movement component if it exists
                MovementComponent* movement = entity->getComponent<MovementComponent>();
                // If the movement component exists
                if(movement) {
                    glm::vec3& entityPosition = entity->localTransform.position;
                    auto* enemy = entity->getComponent<EnemyComponent>();
                    Entity* player = nullptr;
                    if(move) player = move->getOwner();

                    // here we do enemy collision logic with other enemies
                    if(enemy) {
                        for (auto otherEnemyEntity: world->getEntities()) {
                            if (otherEnemyEntity == entity) continue;
                            auto *otherEnemyComponent = otherEnemyEntity->getComponent<EnemyComponent>();
                            if (otherEnemyComponent) {
                                glm::vec3 otherEnemyPosition = otherEnemyEntity->localTransform.position;
                                if (pow(otherEnemyPosition.x - entityPosition.x, 2) +
                                    pow(otherEnemyPosition.z - entityPosition.z, 2) <= 5) {
                                    // Check if enough time has passed since the last collision
                                    auto currentTime = std::chrono::steady_clock::now();
                                    auto it = lastCollisionTimes.find(entity);
                                    if (it == lastCollisionTimes.end() ||
                                        (currentTime - it->second) >= std::chrono::milliseconds (50)) {
                                        // Update the last collision time for this entity
                                        lastCollisionTimes[entity] = currentTime;
                                        lastCollisionTimes[otherEnemyEntity] = currentTime;

                                        // Swap the linear speed of both colliding enemies (pseudo collision)
                                        glm::vec3 tempVelocity = entity->getComponent<MovementComponent>()->linearVelocity;
                                        entity->getComponent<MovementComponent>()->linearVelocity = otherEnemyEntity->getComponent<MovementComponent>()->linearVelocity;
                                        otherEnemyEntity->getComponent<MovementComponent>()->linearVelocity = tempVelocity;

                                        // reset the last cube they hit
                                        // this is due to the fact that if an enemy collides with another enemy
                                        // they could collide with the same cube or its neighbours twice in a row
                                        latestCube[entity] = nullptr;
                                        latestCube[otherEnemyEntity] = nullptr;
                                    }
                                }
                            }
                        }
                    }

                    // enemy collision with the walls
                    if(enemy){
                        // Collision with the cubes
                        double leastDistance = 3; // hit-box required for the ball and the cube to collide
                        double distance;
                        bool cubeCollision = false;
                        for(auto cube : world->getEntities()){
                            if (cube->getComponent<CoveredCubeComponent>())
                            {
                                glm::vec3& cubePosition = cube->localTransform.position;
                                if(cubePosition.y < 0) continue;

                                distance = pow(cubePosition.x - entityPosition.x, 2) + pow(cubePosition.z - entityPosition.z, 2);
                                if(distance < leastDistance)
                                {
                                    cubeCollision = true;
                                    leastDistance = distance;
                                    nearestCube[entity] = cube;
                                }
                            }
                        }
                        if(cubeCollision)
                        {
                            // If the current and previous cube the entity collided with are neighbours don't allow collision
                            if(latestCube[entity])
                            {
                                if(our::AreaCoverageSystem::isNeighbourCube(nearestCube[entity], latestCube[entity]))
                                {
                                    continue;
                                }
                            }
                            latestCube[entity] = nearestCube[entity];

                            glm::vec3& cubePosition = nearestCube[entity]->localTransform.position;
                            // Reverse its linear velocity depending on where it hit the cube
                            if(abs(cubePosition.x - entityPosition.x) > abs(cubePosition.z - entityPosition.z))
                                movement->linearVelocity.x *= -1;
                            else if(abs(cubePosition.x - entityPosition.x) < abs(cubePosition.z - entityPosition.z))
                                movement->linearVelocity.z *= -1;
                            else
                                movement->linearVelocity *= -1;
                        }
                    }

                    // Enemy collision logic with the player
                    if(enemy && player) {
                        glm::vec3 playerPosition = player->localTransform.position;
                        if (pow(playerPosition.x - entityPosition.x, 2) + pow(playerPosition.z - entityPosition.z, 2) <= 3) {
                            player->localTransform.position = INITIAL_PLAYER_POSITION;
                            cameraPosition = INITIAL_CAMERA_POSITION;
                            areaCoverageSystem->dieReset();
                        }

                        // Enemy collision logic with the line the player is drawing
                        std::vector<Entity*> dots = areaCoverageSystem->dots;
                        for(auto dot : dots)
                        {
                            glm::vec3 dotPosition = dot->localTransform.position;
                            if (dotPosition.y < 0) continue;
                            if(pow(dotPosition.x - entityPosition.x, 2) + pow(dotPosition.z - entityPosition.z, 2) <= 2.5){
                                player->localTransform.position = INITIAL_PLAYER_POSITION;
                                cameraPosition = INITIAL_CAMERA_POSITION;
                                areaCoverageSystem->dieReset();
                            }
                        }
                    }
                }
            }
        }

    };

}
