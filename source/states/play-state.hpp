#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <systems/keyboard-movement.hpp>
#include <systems/collision.hpp>
#include <systems/area-coverage.hpp>
#include <asset-loader.hpp>

// This state shows how to use the ECS framework and deserialization.
class Playstate: public our::State {

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    our::KeyboardMovementSystem keyboardMovementSystem;
    our::CollisionSystem collisionSystem;
    our::AreaCoverageSystem areaCoverageSystem;

    void onInitialize() override {
        // First of all, we get the scene configuration from the app config
        auto& config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if(config.contains("assets")){
            our::deserializeAllAssets(config["assets"]);
        }
        // If we have a world in the scene config, we use it to populate our world
        if(config.contains("world")){
            std::cout<<("a7a1/3")<<std::endl;
            world.deserialize(config["world"]);
            std::cout<<("a7a2")<<std::endl;

        }
        // We initialize the camera controller system since it needs a pointer to the app
        cameraController.enter(getApp());
        keyboardMovementSystem.enter(getApp());
        // areaCoverageSystem.dieReset();
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
        std::cout<<("a7a 3")<<std::endl;

    }

    void onDraw(double deltaTime) override {
        // Here, we just run a bunch of systems to control the world logic
        movementSystem.update(&world, (float)deltaTime);
        cameraController.update(&world, (float)deltaTime);
        keyboardMovementSystem.update(&world, (float)deltaTime, &areaCoverageSystem);
        collisionSystem.update(&world, &areaCoverageSystem);
        areaCoverageSystem.update(&world);
        world.deleteMarkedEntities();

        // And finally we use the renderer system to draw the scene
        renderer.render(&world);

        // Get a reference to the keyboard object
        auto& keyboard = getApp()->getKeyboard();

        if(keyboard.justPressed(GLFW_KEY_ESCAPE)){
            // If the escape  key is pressed in this frame, go to the play state
            getApp()->changeState("menu");
        }
    }

    void onDestroy() override {
        // Don't forget to destroy the renderer
        renderer.destroy();
        // On exit, we call exit for the camera controller system to make sure that the mouse is unlocked
        cameraController.exit();
        // Clear the world
        world.clear();
        // and we delete all the loaded assets to free memory on the RAM and the VRAM
        areaCoverageSystem.exit_reset();
        our::clearAllAssets();
    }
};