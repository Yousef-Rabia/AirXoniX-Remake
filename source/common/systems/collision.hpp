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
                                if (pow(otherEnemyPosition.x - entity->localTransform.position.x, 2) +
                                    pow(otherEnemyPosition.z - entity->localTransform.position.z, 2) <= 5) {
                                    // Check if enough time has passed since the last collision
                                    auto currentTime = std::chrono::steady_clock::now();
                                    auto it = lastCollisionTimes.find(entity);
                                    if (it == lastCollisionTimes.end() ||
                                        (currentTime - it->second) >= std::chrono::milliseconds (10)) {
                                        // Update the last collision time for this entity
                                        lastCollisionTimes[entity] = currentTime;
                                        lastCollisionTimes[otherEnemyEntity] = currentTime;

                                        glm::vec3 tempVelocity = entity->getComponent<MovementComponent>()->linearVelocity;
                                        entity->getComponent<MovementComponent>()->linearVelocity = otherEnemyEntity->getComponent<MovementComponent>()->linearVelocity;
                                        otherEnemyEntity->getComponent<MovementComponent>()->linearVelocity = tempVelocity;
                                    }
                                }
                            }
                        }
                    }
                    // and here we do enemy collision logic with the player
                    if(enemy && player){
                        glm::vec3 playerPosition = player->localTransform.position;
                        if(pow(playerPosition.x - entity->localTransform.position.x, 2) + pow(playerPosition.z - entity->localTransform.position.z, 2) <= 5){
                            player->localTransform.position = INITIAL_PLAYER_POSITION;
                            cameraPosition = INITIAL_CAMERA_POSITION;
                            areaCoverageSystem->dieReset();
                        }
                    }
                }
            }
        }

    };

}
