#include "covered-cube.hpp"
#include "../ecs/entity.hpp"
#include <iostream>
namespace our {
    void CoveredCubeComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        cubeType = data.value("cubeType", cubeType);
        auto it = data.find("position");
        if(it == data.end()){
            std::cout<<"Not Found Position\n";
        }else{
            short i =0;
            for(auto temp : *it){
               position[i++] = temp;
            }
        }


    }
}