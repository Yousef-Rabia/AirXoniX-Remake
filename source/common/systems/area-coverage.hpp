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
        Application* app; // The application in which the state runs

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
        std::vector<Entity*> enemies;
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

        glm::vec2 endPos;

        glm::vec2 point1,point2;

        // Direction of the movement initially
        int startDirection = -1;


        // Flag to check if the cubes list is filled
        bool cubesFilled = false;

        // Flag to check if the enemies list is filled
        bool enemiesFilled = false;

        // Flag to check if there exits at least 2 in the grid (for the DFS)
        bool found2 = false;

        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application* app){
            this->app = app;
        }

        AreaCoverageSystem() {
            // Initialize the grid and cubes list
            startPos = RESET_STARTPOS;
            endPos = RESET_STARTPOS;
            point1 = RESET_STARTPOS;
            point2 = RESET_STARTPOS;
            cubes.resize(GRID_DIMENSION);
            grid.resize(GRID_DIMENSION);
            vis.resize(GRID_DIMENSION);

            // Fill the grid with 0s and 1s (1s are the border)
            for (int i = 0; i < GRID_DIMENSION; i++) {
                grid[i].resize(GRID_DIMENSION);
                cubes[i].resize(GRID_DIMENSION);
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
                int x = glm::round(playerPosition.x + 19.5);
                int z = glm::round(playerPosition.z + 19.5);

                // Index Limit Check (player's position may exceed the grid's limits)
                if (x < 0) x = 0;
                if (z < 0) z = 0;
                if (x >= GRID_DIMENSION) x = GRID_DIMENSION - 1;
                if (z >= GRID_DIMENSION) z = GRID_DIMENSION - 1;

                //if it is not drawn, Draw it, else do NOTHING
                if (grid[x][z] != 1 ) {
                    // NOT DRAWN
                    if (startPos == RESET_STARTPOS) {
                        // Started moving into uncovered area
                        startPos = glm::vec2(x, z);

                        // calculates direction of first movement
                        startDirection = calcDirection(prevPos, startPos);
                        std::cout << "start moving in Direction: " << startDirection << " ,X: " << x << " ,Z: " << z << "\n";
                    }
                    // Draws the cube

                    if(grid[x][z] != 2)
                    {
                        dots[curDot++]->localTransform.position = glm::vec3(cubes[x][z]->localTransform.position.x, 0,
                                                                            cubes[x][z]->localTransform.position.z);
                    }

                    if(point1 == glm::vec2 (x, z) || point2 == glm::vec2 (x, z)){
                        point1 = RESET_STARTPOS;
                        point2 = RESET_STARTPOS;
                    }

                    if(endPos ==RESET_STARTPOS)
                        endPos = prevPos;
                    if((point1== RESET_STARTPOS || point2== RESET_STARTPOS)){
                        setPoint1_2(calcDirection(endPos, glm::vec2(x, z)), glm::vec2(x, z));
                    }
                    endPos = glm::vec2(x, z);
                    grid[x][z] = 2;
                } else {
                    // Already DRAWN

                    //updates the previous position of the last drawn cell I stepped on .
                    // (useful for calculating the direction later)
                    prevPos = glm::vec2(x, z);

                    //checks if this was the END of moving into uncovered area, if so, starts the DFS
                    if (startPos != RESET_STARTPOS && startPos != prevPos) {
                        fillBorderAfterCovering();
                        if(point1== RESET_STARTPOS || point2== RESET_STARTPOS) {
                            setPoint1_2(calcDirection(endPos, prevPos), prevPos);
                        }

                        // checks if the enemy exists in the covered area, DO NOT fill it
                        if(point1 != RESET_STARTPOS && !enemyExists(point1.x, point1.y)) {
                            dfsAndDraw(point1.x, point1.y);
                        }

                        // checks if the enemy exists in the covered area, DO NOT fill it
                        if(point2 != RESET_STARTPOS && !enemyExists(point2.x, point2.y))
                            dfsAndDraw(point2.x, point2.y);

                        calcCoveredPercentage();
                        vis.clear();
                        vis.resize(GRID_DIMENSION, std::vector<bool>(GRID_DIMENSION, false));
                        startPos = RESET_STARTPOS;
                        point1 = RESET_STARTPOS;
                        point2 = RESET_STARTPOS;
                        endPos = RESET_STARTPOS;
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

        void dieReset(World* world){
            for(int i = curDot-1; i>=0;  i--){
                int x = glm::round(dots[i]->localTransform.position.x + 19.5);
                int z = glm::round(dots[i]->localTransform.position.z + 19.5);
                if(x < 0) x = 0;
                if(z < 0) z = 0;
                if(x >= GRID_DIMENSION) x = GRID_DIMENSION - 1;
                if(z >= GRID_DIMENSION) z = GRID_DIMENSION - 1;

                if(grid[x][z] == 2)
                    grid[x][z] = 0;
                dots[i]->localTransform.position = RESET_DOT;
            }
            curDot = 0;
            startPos = RESET_STARTPOS;
            prevPos = RESET_STARTPOS;
            point1 = RESET_STARTPOS;
            point2 = RESET_STARTPOS;
            app->lives -= 1;

            // when the player dies end their and the camera's speed (for when they die while building)
            for(auto entity : world->getEntities()){
                if(entity->getComponent<CameraComponent>() || entity->getComponent<KeyboardMovementComponent>())
                    entity->getComponent<MovementComponent>()->linearVelocity = {0, 0, 0};
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
                    enemies.push_back(entity);
                }
            }
        }

        bool enemyExists(int x, int y) {
            if (vis[x][y]|| x < 0 || x >= GRID_DIMENSION || y < 0 || y >= GRID_DIMENSION || grid[x][y] == 1 ||
                grid[x][y] == 2)
                return false;
            vis[x][y] = true;

            for(auto entity: enemies){
                glm::vec3 enemyPosition = entity->localTransform.position;
                int enemyX = glm::round(enemyPosition.x + 19.5);
                int enemyY = glm::round(enemyPosition.z + 19.5);
                if(enemyX == x && enemyY == y){
                    return true;
                }
            }

            if(enemyExists(x + 1, y) ||
            enemyExists(x - 1, y) ||
            enemyExists(x, y + 1) ||
            enemyExists(x, y - 1)){
                return true;
            }
            return false;
        }

        int calcDirection(glm::vec2 start, glm::vec2 end){
            if(start.x == end.x){
                if(start.y > end.y){
                    return 1; //Up
                }else{
                    return 0; //Down
                }
            }else{
                if(start.x > end.x){
                    return 2; //Right
                }else{
                    return 3; //Left
                }
            }
        }

        void setPoint1_2 (int direction, glm::vec2 curPos){
            // DO NOT try to find ONLY if both are already set!
            if((point1 != RESET_STARTPOS && point2 != RESET_STARTPOS)) return;

            switch (direction) {
                //==============> DOWN <===============
                case 0:
                    if(grid[curPos.x + 1][curPos.y] == 0)
                        point1 = glm::vec2(curPos.x + 1, curPos.y);
                    if(grid[curPos.x - 1][curPos.y] == 0)
                        point2 = glm::vec2(curPos.x - 1, curPos.y);
                    break;
                //==============> UP <===============
                case 1:
                    if(grid[curPos.x + 1][curPos.y] == 0)
                        point1 = glm::vec2(curPos.x + 1, curPos.y);
                    if(grid[curPos.x - 1][curPos.y] == 0)
                        point2 = glm::vec2(curPos.x - 1, curPos.y);
                    break;
                //==============> RIGHT <===============
                case 2:
                    if(grid[curPos.x][curPos.y + 1] == 0)
                        point1 = glm::vec2(curPos.x, curPos.y + 1);
                    if(grid[curPos.x][curPos.y - 1] == 0)
                        point2 = glm::vec2(curPos.x, curPos.y - 1);
                    break;
                //==============> LEFT <===============
                case 3:
                    if(grid[curPos.x][curPos.y + 1] == 0)
                        point1 = glm::vec2(curPos.x, curPos.y + 1);
                    if(grid[curPos.x][curPos.y - 1] == 0)
                        point2 = glm::vec2(curPos.x, curPos.y - 1);
                    break;
            }
        }

        void printGrid(){
            for (int i = 0; i < GRID_DIMENSION; i++) {
                for (int j = 0; j < GRID_DIMENSION; j++) {
                    std::cout << grid[i][j];
                }
                std::cout << "\n";
            }
        }

        float calcCoveredPercentage(){
            int tot= 0 ;
            for (int i = 0; i < GRID_DIMENSION; i++) {
                for (int j = 0; j < GRID_DIMENSION; j++) {
                    if(grid[i][j] == 1)
                        tot++;
                }
            }
            tot -=  (38 * 4 + 38 * 4);
            float percentage = tot/(1288.0) * 100.0;
            return percentage;
        }

        void printEnemy(){
            for(auto enemyEntity: enemies){
                glm::vec3 enemyPosition = enemyEntity->localTransform.position;
                int enemyX = glm::round(enemyPosition.x + 19.5);
                int enemyY = glm::round(enemyPosition.z + 19.5);
                std::cout << "Enemy AT: " << " X: " << enemyX << " ,Z: " << enemyY << "\n";
            }
        }

        // Returns true if the player character is currently building blocks (to move automatically)
        bool isBuilding(){
           return (startPos != RESET_STARTPOS);
        }

        static bool isNeighbourCube(Entity* cube1, Entity* cube2){
            float xDistance = abs(cube1->localTransform.position.x - cube2->localTransform.position.x);
            float zDistance = abs(cube1->localTransform.position.z - cube2->localTransform.position.z);
            return (xDistance + zDistance <= 1);
        }

        void exit_reset(){
            cubesFilled = false;
            enemiesFilled = false;
            dots.clear();
            grid.clear();
            enemies.clear();
            dots.clear();
            vis.clear();
            startPos = RESET_STARTPOS;
            endPos = RESET_STARTPOS;
            point1 = RESET_STARTPOS;
            point2 = RESET_STARTPOS;
            cubes.resize(GRID_DIMENSION);
            grid.resize(GRID_DIMENSION);
            vis.resize(GRID_DIMENSION);

            // Fill the grid with 0s and 1s (1s are the border)
            for (int i = 0; i < GRID_DIMENSION; i++) {
                grid[i].resize(GRID_DIMENSION);
                cubes[i].resize(GRID_DIMENSION);
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
    };

}
