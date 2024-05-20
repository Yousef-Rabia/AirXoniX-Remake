#pragma once

#include "../ecs/world.hpp"
#include "../components/movement.hpp"
#include "../components/enemy.hpp"
#include "../components/keyboard-movement.hpp"
#include "../components/enemy.hpp"
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
        Application* app; // The application in which the state runs

    public:
        const glm::vec3 RESET_DOT = glm::vec3(10, -3.05 ,15);

        // Define a map to store the last collision time for each entity
        std::unordered_map<Entity*, std::chrono::steady_clock::time_point> lastCollisionTimes;

        // Define a map to store the last wall collisions for each entity (separate from entity x entity collisions)
        std::unordered_map<Entity*, Entity*> nearestCube;
        std::unordered_map<Entity*, Entity*> latestCube;

        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application* app){
            this->app = app;
        }

        // This should be called every frame to update all entities containing a MovementComponent.
        void update(World* world, AreaCoverageSystem *areaCoverageSystem) {

            // get the player (later used for collision calculations)
            KeyboardMovementComponent *move = nullptr;
            for(auto entity : world->getEntities()){
                move = entity->getComponent<KeyboardMovementComponent>();
                if(move) break;
            }
            if(!move) return;
            Entity* player = move->getOwner();
            glm::vec3& playerPosition = player->localTransform.position;

            // For each entity in the world
            for(auto entity : world->getEntities()){
                // Get the movement component if it exists
                MovementComponent* movement = entity->getComponent<MovementComponent>();
                // If the movement component exists
                if(movement) {
                    glm::vec3& entityPosition = entity->localTransform.position;
                    auto* enemy = entity->getComponent<EnemyComponent>();


                    // here we do enemy collision logic
                    if(enemy) {
                        // Enemy collision with other enemies
                        for (auto otherEnemyEntity: world->getEntities()) {
                            if (otherEnemyEntity == entity) continue;
                            auto otherEnemyComponent = otherEnemyEntity->getComponent<EnemyComponent>();

                            if (otherEnemyComponent) {
                                if (otherEnemyComponent->enemyType != enemy->enemyType) continue;

                                glm::vec3 otherEnemyPosition = otherEnemyEntity->localTransform.position;
                                if (pow(otherEnemyPosition.x - entityPosition.x, 2) +
                                    pow(otherEnemyPosition.z - entityPosition.z, 2) <= ENEMY_ENEMY_HITBOX) {
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

                                        // play the sound
                                        app->soundPlayer.playSound("ball_selfCollide");
                                    }
                                }
                            }
                        }

                        // Ball enemy collision with the walls
                        if(enemy->enemyType == "Ball"){
                            // Collision with the cubes
                            double leastDistance = BALL_CUBE_HITBOX; // hit-box required for the ball and the cube to collide
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
                                {
                                    movement->linearVelocity.x *= -1;
                                    movement->linearVelocity.z *= -1;
                                }

                                // play the sound
                                app->soundPlayer.playSound("ball_reflect");
                            }
                        }

                        // Mine enemy collision with the walls
                        if(enemy->enemyType == "Mine"){
                            // Collision with the outer borders
                            if(entityPosition.x > (ARENA_LENGTH+0.5)){
                                movement->linearVelocity.x = -abs(movement->linearVelocity.x);
                                latestCube[entity] = nullptr;
                                app->soundPlayer.playSound("mine_reflect");
                            }
                            if(entityPosition.x < -(ARENA_LENGTH+0.5)){
                                movement->linearVelocity.x = abs(movement->linearVelocity.x);
                                latestCube[entity] = nullptr;
                                app->soundPlayer.playSound("mine_reflect");
                            }
                            if(entityPosition.z > (ARENA_LENGTH+0.5)){
                                movement->linearVelocity.z = -abs(movement->linearVelocity.x);
                                latestCube[entity] = nullptr;
                                app->soundPlayer.playSound("mine_reflect");
                            }
                            if(entityPosition.z < -(ARENA_LENGTH+0.5)){
                                movement->linearVelocity.z = abs(movement->linearVelocity.x);
                                latestCube[entity] = nullptr;
                                app->soundPlayer.playSound("mine_reflect");
                            }

                            // Collision with the cubes (the hidden ones)
                            double leastDistance = MINE_CUBE_HITBOX; // hit-box required for the ball and the cube to collide
                            double distance;
                            bool cubeCollision = false;
                            for(auto cube : world->getEntities()){
                                if (cube->getComponent<CoveredCubeComponent>())
                                {
                                    glm::vec3& cubePosition = cube->localTransform.position;
                                    if(cubePosition.y > -1) continue;

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

                                // play the sound
                                app->soundPlayer.playSound("mine_reflect");
                            }
                        }

                        // Enemy collision logic with the player
                        if(player) {
                            if (pow(playerPosition.x - entityPosition.x, 2) + pow(playerPosition.z - entityPosition.z, 2) <= ENEMY_PLAYER_HITBOX) {
                                areaCoverageSystem->dieReset(world);
                                if(enemy->enemyType == "Mine")
                                    entityPosition = INITIAL_MINE_POSITION;
                            }

                            // Enemy collision logic with the line the player is drawing
                            std::vector<Entity*> dots = areaCoverageSystem->dots;
                            for(auto dot : dots)
                            {
                                glm::vec3 dotPosition = dot->localTransform.position;
                                if (dotPosition.y < 0) continue;
                                if(pow(dotPosition.x - entityPosition.x, 2) + pow(dotPosition.z - entityPosition.z, 2) <= ENEMY_LINE_HITBOX){
                                    areaCoverageSystem->dieReset(world);
                                }
                            }
                        }
                    }
                }
            }
        }

    };

}
