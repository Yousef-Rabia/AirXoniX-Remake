#include "entity.hpp"
#include "../deserialize-utils.hpp"

#include <glm/gtx/euler_angles.hpp>

namespace our {

    // This function computes and returns a matrix that represents this transform
    // Remember that the order of transformations is: Scaling, Rotation then Translation
    // HINT: to convert euler angles to a rotation matrix, you can use glm::yawPitchRoll
    glm::mat4 Transform::toMat4(glm::mat4 selfRotation) const {
        //TODO: (Req 3) Write this function

        // Compute translation matrix
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

        // Compute rotation matrix from euler angles
        glm::mat4 rotationMatrix = glm::yawPitchRoll(rotation.y, rotation.x, rotation.z);

        // Compute scaling matrix
        glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), scale);

        // Construct the transformation matrix by combining scaling, rotation, and translation
        glm::mat4 transformMatrix;
        if(selfRotation == glm::mat4(1.0))
            transformMatrix = translationMatrix * rotationMatrix * scalingMatrix;
        else
            transformMatrix = translationMatrix * selfRotation * scalingMatrix;

        return transformMatrix;
    }

     // Deserializes the entity data and components from a json object
    void Transform::deserialize(const nlohmann::json& data){
        position = data.value("position", position);
        rotation = glm::radians(data.value("rotation", glm::degrees(rotation)));
        scale    = data.value("scale", scale);
    }

}