#pragma once

#include "iostream"
#include "../ecs/world.hpp"
#include "../components/movement.hpp"
#include "../components/keyboard-movement.hpp"
#include "application.hpp"
#include "components/camera.hpp"
#include "../components/covered-cube.hpp"
#include "../components/enemy.hpp"
#include "../components/dot.hpp"


#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <chrono>
#include <vector>

namespace our {

    // The collision system is responsible for the effect of any two entities colliding.
    class AreaCoverageSystem {
    public:
        /*
         * =================================CONSTANTS=================================
         * GRID_DIMENSION: The dimension of the grid (40x40)
         * RESET_STARTPOS: Dummy Invalid position (indication of not covering area at the moment)
         * EPS: A small value for floating point comparison
         */
        const int GRID_DIMENSION = 40;
        const glm::vec2 RESET_STARTPOS = glm::vec2(-1, -1);
        const glm::vec3 RESET_DOT = glm::vec3(10, -3.05 ,15);
        const float EPS = 1e-4;

        // 2D vector to store the cubes Entities Pointer
        std::vector<std::vector<Entity *>> cubes;

        // 2D vector to store the enemies Entities Pointer
        std::vector<std::vector<Entity *>> enemies;
        std::vector<std::vector<bool>> vis;

        // 2D vector to store the dots Entities Pointer
        std::vector<Entity*> dots;
        int curDot = 0;

        // 2D vector to store the grid (0: Not Drawn, 1: Drawn, 2: Pending)
        std::vector<std::vector<short>> grid;

        // Start Position of the movement into uncovered area
        glm::vec2 startPos;

        // Previous Position of the last drawn cell I stepped on
        glm::vec2 prevPos;

        // Direction of the movement
        int direction = -1;

        // Flag to check if the cubes list is filled
        bool cubesFilled = false;

        // Flag to check if the enemies list is filled
        bool enemiesFilled = false;

        AreaCoverageSystem() {
            // Initialize the grid and cubes list
            startPos = RESET_STARTPOS;
            cubes.resize(GRID_DIMENSION);
            grid.resize(GRID_DIMENSION);
            enemies.resize(GRID_DIMENSION);
            vis.resize(GRID_DIMENSION);

            // Fill the grid with 0s and 1s (1s are the border)
            for (int i = 0; i < GRID_DIMENSION; i++) {
                grid[i].resize(GRID_DIMENSION);
                cubes[i].resize(GRID_DIMENSION);
                enemies[i].resize(GRID_DIMENSION);
                vis[i].resize(GRID_DIMENSION,false);
                for (int j = 0; j < GRID_DIMENSION; j++) {
                    if ((i >= 0 && i <= 1) || (i <= GRID_DIMENSION - 1 && i >= GRID_DIMENSION - 2) ||
                        (j >= 0 && j <= 1) || (j <= GRID_DIMENSION - 1 && j >= GRID_DIMENSION - 2))
                        grid[i][j] = 1;
                    else
                        grid[i][j] = 0;
                }
            }

        }

        // This should be called every frame to update all entities containing a MovementComponent.
        void update(World *world) {
            // get the player (later used for collision calculations)
            KeyboardMovementComponent *move = nullptr;

            // For each entity in the world
            for (auto entity: world->getEntities()) {
                move = entity->getComponent<KeyboardMovementComponent>();
                if (move) break;
            }

            //if No Movement, there is Nothing to do here!
            if (!move) return;

            //get the player entity
            Entity *player = nullptr;
            player = move->getOwner();
            glm::vec3 playerPosition = player->localTransform.position;

            // if the cubes list is not filled, Fill it Once And for All!
            if (!cubesFilled) {
                cubesFilled = true;
                fillDotsList(world);
                fillCubesList(world);
            }

            // if the enemies list is not filled, Fill it Once And for All!
            if (!enemiesFilled) {
                enemiesFilled = true;
                fillEnemiesList(world);
            }

            if (player) {
                // getting their place in the 2d vector
//                std::cout << "player: " << " X: " << playerPosition.x << " ,Z: " << playerPosition.z << "\n";
                int x = glm::round(playerPosition.x + 19.5);
                int z = glm::round(playerPosition.z + 19.5);

                // Index Limit Check (player's position may exceed the grid's limits)
                if (x < 0) x = 0;
                if (z < 0) z = 0;
                if (x >= GRID_DIMENSION) x = GRID_DIMENSION - 1;
                if (z >= GRID_DIMENSION) z = GRID_DIMENSION - 1;

//                std::cout << "player: " << " X: " << x << " ,Z: " << z << "\n";

                //if it is not drawn, Draw it, else do NOTHING
                if (grid[x][z] != 1 ) {
//                    std::cout << "Drawing: " << " X: " << cubes[x][z]->localTransform.position.x << " ,Z: "
//                              << cubes[x][z]->localTransform.position.z << "\n";
                    // NOT DRAWN
                    if (startPos == RESET_STARTPOS) {
                        // Started moving into uncovered area
                        startPos = glm::vec2(x, z);

                        // calculates direction of first movement
                        if (startPos.x == prevPos.x) {
                            if (startPos.y > prevPos.y) {
                                direction = 0; //DOWN
                            } else {
                                direction = 1; //UP
                            }
                        } else {
                            if (startPos.x > prevPos.x) {
                                direction = 2; //Right
                            } else {
                                direction = 3; //Left
                            }
                        }
                        std::cout << "start moving Direction: " << direction << " X: " << x << " ,Z: " << z << "\n";
                    }
                    // Draws the cube
                    if(grid[x][z] != 2)
                    {
                        dots[curDot++]->localTransform.position = glm::vec3(cubes[x][z]->localTransform.position.x, 0,
                                                                            cubes[x][z]->localTransform.position.z);
                        std::cout << "Drawing dot curDot: " << curDot<< "\n";
                    }
                    //Mark this cell as pending(Dotted)
                    grid[x][z] = 2;
                } else {
                    // Already DRAWN

                    //updates the previous position of the last drawn cell I stepped on .
                    // (useful for calculating the direction later)
                    prevPos = glm::vec2(x, z);

                    //checks if this was the END of moving into uncovered area, if so, starts the DFS
                    if (startPos != RESET_STARTPOS && startPos != prevPos) {
                        fillBorderAfterCovering();
                        printEnemy();
                        switch (direction) {
                            case 0:
//                                std::cout << "Sent: " << " X: " << startPos.x + 1 << " ,Z: " << startPos.y << "\n";
//                                std::cout << "Sent: " << " X: " << startPos.x - 1 << " ,Z: " << startPos.y << "\n";
//                                printGrid();

                                // checks if the enemy exists in the covered area, DO NOT fill it
                                if(!enemyExists(startPos.x + 1, startPos.y)) {
                                    dfsAndDraw(startPos.x + 1, startPos.y);
                                }

                                // checks if the enemy exists in the covered area, DO NOT fill it
                                if(!enemyExists(startPos.x - 1, startPos.y))
                                    dfsAndDraw(startPos.x - 1, startPos.y);
                                break;
                            case 1:
//                                std::cout << "Sent: " << " X: " << startPos.x + 1 << " ,Z: " << startPos.y << "\n";
                                // checks if the enemy exists in the covered area, DO NOT fill it
                                if(!enemyExists(startPos.x + 1, startPos.y))
                                    dfsAndDraw(startPos.x + 1, startPos.y);

                                // checks if the enemy exists in the covered area, DO NOT fill it
                                if(!enemyExists(startPos.x - 1, startPos.y))
                                    dfsAndDraw(startPos.x - 1, startPos.y);
                                break;
                            case 2:
//                                std::cout << "Sent: " << " X: " << startPos.x + 1 << " ,Z: " << startPos.y << "\n";
                                // checks if the enemy exists in the covered area, DO NOT fill it
                                if(!enemyExists(startPos.x, startPos.y - 1))
                                    dfsAndDraw(startPos.x, startPos.y - 1);

                                // checks if the enemy exists in the covered area, DO NOT fill it
                                if(!enemyExists(startPos.x, startPos.y + 1))
                                    dfsAndDraw(startPos.x, startPos.y + 1);                                break;
                            case 3:
//                                std::cout << "Sent: " << " X: " << startPos.x + 1 << " ,Z: " << startPos.y << "\n";
                                // checks if the enemy exists in the covered area, DO NOT fill it
                                if(!enemyExists(startPos.x, startPos.y - 1))
                                    dfsAndDraw(startPos.x, startPos.y - 1);

                                // checks if the enemy exists in the covered area, DO NOT fill it
                                if(!enemyExists(startPos.x, startPos.y + 1))
                                    dfsAndDraw(startPos.x, startPos.y + 1);
                                break;
                        }
                        vis.clear();
                        vis.resize(GRID_DIMENSION, std::vector<bool>(GRID_DIMENSION, false));
                        startPos = RESET_STARTPOS;
                    }

                }
            }

        }

        void dfsAndDraw(int x, int y) {
            if (x < 0 || x >= GRID_DIMENSION || y < 0 || y >= GRID_DIMENSION || grid[x][y] == 1 ||
                grid[x][y] == 2)
                return;
            grid[x][y] = 1;
            if (cubes[x][y]) {
                cubes[x][y]->localTransform.position = glm::vec3(cubes[x][y]->localTransform.position.x, 0,
                                                                 cubes[x][y]->localTransform.position.z);
            }
            dfsAndDraw(x + 1, y);
            dfsAndDraw(x - 1, y);
            dfsAndDraw(x, y + 1);
            dfsAndDraw(x, y - 1);
        }

        void fillBorderAfterCovering() {
            for(int i = curDot-1; i>=0;  i--){
                int x = glm::round(dots[i]->localTransform.position.x + 19.5);
                int z = glm::round(dots[i]->localTransform.position.z + 19.5);
                if(x < 0) x = 0;
                if(z < 0) z = 0;
                if(x >= GRID_DIMENSION) x = GRID_DIMENSION - 1;
                if(z >= GRID_DIMENSION) z = GRID_DIMENSION - 1;
                cubes[x][z]->localTransform.position = glm::vec3(cubes[x][z]->localTransform.position.x, 0,
                                                                 cubes[x][z]->localTransform.position.z);
                grid[x][z] = 1;
                dots[i]->localTransform.position = RESET_DOT;
                curDot=0;
            }
        }

        void fillCubesList(World *world) {
            for (auto entity: world->getEntities()) {
                auto *coveredCube = entity->getComponent<CoveredCubeComponent>();
                if (coveredCube) {
                    glm::vec3 coveredCubePosition = entity->localTransform.position;
                    int x = glm::round(coveredCubePosition.x + 19.5);
                    int z = glm::round(coveredCubePosition.z + 19.5);
                    cubes[x][z] = entity;
                }
            }
        }

        void fillDotsList(World *world) {
            for (auto entity: world->getEntities()) {
                auto *dot = entity->getComponent<DotComponent>();
                if (dot) {
                    dots.push_back(entity);
                }
            }
        }


        void fillEnemiesList(World *world) {
            for (auto entity: world->getEntities()) {
                auto *enemy = entity->getComponent<EnemyComponent>();
                if (enemy) {
                    glm::vec3 enemyPosition = entity->localTransform.position;
                    int x = glm::round(enemyPosition.x + 19.5);
                    int z = glm::round(enemyPosition.z + 19.5);
                    if(x < 0) x = 0;
                    if(z < 0) z = 0;
                    if(x >= GRID_DIMENSION) x = GRID_DIMENSION - 1;
                    if(z >= GRID_DIMENSION) z = GRID_DIMENSION - 1;
                    enemies[x][z] = entity;
                }
            }
        }

        bool enemyExists(int x, int y) {
            if (vis[x][y]|| x < 0 || x >= GRID_DIMENSION || y < 0 || y >= GRID_DIMENSION || grid[x][y] == 1 ||
                grid[x][y] == 2)
                return false;
            vis[x][y] = true;
            if (enemies[x][y]) {
//                std::cout << "Enemy AT: " << " X: " << x << " ,Z: " << y << "\n";
                return true;
            }
            if(enemyExists(x + 1, y) ||
            enemyExists(x - 1, y) ||
            enemyExists(x, y + 1) ||
            enemyExists(x, y - 1)){
                return true;
            }
            return false;
        }

        void printGrid(){
            for (int i = 0; i < GRID_DIMENSION; i++) {
                for (int j = 0; j < GRID_DIMENSION; j++) {
                    std::cout << grid[i][j];
                }
                std::cout << "\n";
            }
        }

        void printEnemy(){
            for (int i = 0; i < GRID_DIMENSION; i++) {
                for (int j = 0; j < GRID_DIMENSION; j++) {
                    if(enemies[i][j]){
                        std::cout << "Enemy AT: " << " X: " << i << " ,Z: " << j << "\n";
                    }
                }
            }
        }

    };

}
