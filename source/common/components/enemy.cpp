#include "enemy.hpp"
#include "../ecs/entity.hpp"

namespace our {
    void EnemyComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        enemyType = data.value("enemyType", enemyType);
    }
}