#pragma once
#include "battle_game/core/unit.h"

namespace battle_game::unit {
class Ausy0201Tank : public Unit {
 public:
  Ausy0201Tank(GameCore *game_core, uint32_t id, uint32_t player_id);
  void Render() override;
  void Update() override;
  [[nodiscard]] bool IsHit(glm::vec2 position) const override;
  [[nodiscard]] float GetSpeedScale() const override {
    return speed_scale_;
  };
  [[nodiscard]] float GetAngularSpeedScale() const {
    return angular_speed_scale_;
  }
  void SetSpeedScale(float speed_scale) {
    speed_scale_ = speed_scale;
  }

 protected:
  void TankMove(float move_speed, float rotate_angular_speed);
  void TurretRotate();
  void Fire();
  [[nodiscard]] const char *UnitName() const override;
  [[nodiscard]] const char *Author() const override;
  void UpdateSpeedScale(int acceleration);
  void UpdateAngularSpeedScale(int angular_acceleration);
  void UpdateTurretRotation(float target_turret_rotation);

  float turret_rotation_{0.0f};
  uint32_t fire_count_down_{0};
  uint32_t mine_count_down_{0};

  const float Accelerate_time_constant_{0.5f};
  const float Decelerate_time_constant_{1.0f};
  const float Angular_accelerate_time_constant_{0.5f};
  const float Angular_decelerate_time_constant_{0.5f};
  const float turret_rotation_time_constant_{0.3f};
  float speed_scale_{0.0f};
  const float Max_speed_{5.0f};
  float angular_speed_scale_{0.0f};
  const float Max_angular_speed_{glm::radians(180.0f)};
};
}  // namespace battle_game::unit
