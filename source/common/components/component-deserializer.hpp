#pragma once
#include "iostream"
#include "../ecs/entity.hpp"
#include "camera.hpp"
#include "mesh-renderer.hpp"
#include "free-camera-controller.hpp"
#include "movement.hpp"
#include "keyboard-movement.hpp"
#include "enemy.hpp"
#include "dot.hpp"
#include "covered-cube.hpp"

namespace our {

    // Given a json object, this function picks and creates a component in the given entity
    // based on the "type" specified in the json object which is later deserialized from the rest of the json object
    inline void deserializeComponent(const nlohmann::json& data, Entity* entity){
        std::string type = data.value("type", "");
        Component* component = nullptr;
        //TODO: (Req 8) Add an option to deserialize a "MeshRendererComponent" to the following if-else statement
        if(type == CameraComponent::getID()){
            component = entity->addComponent<CameraComponent>();
        } else if (type == FreeCameraControllerComponent::getID()) {
            component = entity->addComponent<FreeCameraControllerComponent>();
        }else if(type==MeshRendererComponent::getID()){
            component=entity->addComponent<MeshRendererComponent>();
        }else if (type == MovementComponent::getID()) {
            component = entity->addComponent<MovementComponent>();
        }else if (type == KeyboardMovementComponent::getID()) {
            component = entity->addComponent<KeyboardMovementComponent>();
        }else if (type == EnemyComponent::getID()) {
            component = entity->addComponent<EnemyComponent>();
        }else if (type == CoveredCubeComponent::getID()) {
            component = entity->addComponent<CoveredCubeComponent>();
        }else if (type == DotComponent::getID()) {
            component = entity->addComponent<DotComponent>();
        }
        if(component) component->deserialize(data);
    }

}