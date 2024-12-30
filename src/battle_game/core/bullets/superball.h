#pragma once
#include "battle_game/core/bullet.h"

namespace battle_game::bullet {
class Superball : public Bullet {
 public:
  Superball(GameCore *core,
             uint32_t id,
             uint32_t unit_id,
             uint32_t player_id,
             glm::vec2 position,
             float rotation,
             float damage_scale,
             glm::vec2 velocity);
  Superball(GameCore *core,
             uint32_t id,
             uint32_t unit_id,
             uint32_t player_id,
             glm::vec2 position,
             float rotation,
             float damage_scale,
             glm::vec2 velocity,
             float angular_velocity);
  ~Superball() override;
  void Render() override;
  void Update() override;

 private:
  void DealCollision(float normal_rotation);

  glm::vec2 velocity_{};
  // The superball here is a uniform solid ball with radius 0.5 units
  const float radius_{0.5f}; 
  float angular_velocity_{0.0f}; // rad per sec
  int life_count_down_{5*60}; // 5 seconds life time
  int bounce_count_down_{5}; // 5 times bounce
};
}  // namespace battle_game::bullet