#include "battle_game/core/bullets/bullets.h"
#include "battle_game/core/game_core.h"
#include "battle_game/graphics/graphics.h"
#include "tank_ausy0201.h"

namespace battle_game::unit {

namespace {
uint32_t tank_body_model_index = 0xffffffffu;
uint32_t tank_turret_model_index = 0xffffffffu;
}  // namespace

Ausy0201Tank::Ausy0201Tank(GameCore *game_core, uint32_t id, uint32_t player_id)
    : Unit(game_core, id, player_id) {
  if (!~tank_body_model_index) {
    auto mgr = AssetsManager::GetInstance();
    {
      /* Tank Body */
      tank_body_model_index = mgr->RegisterModel(
          {
              {{-0.8f, 0.8f}, {0.0f, 0.0f}, {0.9f, 0.0f, 0.1f, 1.0f}},
              {{-0.8f, -1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
              {{0.8f, 0.8f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
              {{0.8f, -1.0f}, {0.0f, 0.0f}, {0.5f, 0.5f, 0.0f, 1.0f}},
              // distinguish front and back
              {{0.6f, 1.0f}, {0.0f, 0.0f}, {0.2f, 0.8f, 0.0f, 1.0f}},
              {{-0.6f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
          },
          {0, 1, 2, 1, 2, 3, 0, 2, 5, 2, 4, 5});
    }

    {
      /* Tank Turret */
      std::vector<ObjectVertex> turret_vertices;
      std::vector<uint32_t> turret_indices;
      const int precision = 60;
      const float inv_precision = 1.0f / float(precision);
      for (int i = 0; i < precision; i++) {
        auto theta = (float(i) + 0.5f) * inv_precision;
        theta *= glm::pi<float>() * 2.0f;
        auto sin_theta = std::sin(theta);
        auto cos_theta = std::cos(theta);
        turret_vertices.push_back({{sin_theta * 0.5f, cos_theta * 0.5f},
                                   {0.0f, 0.0f},
                                   {0.7f, 0.7f, 0.7f, 1.0f}});
        turret_indices.push_back(i);
        turret_indices.push_back((i + 1) % precision);
        turret_indices.push_back(precision);
      }
      turret_vertices.push_back(
          {{0.0f, 0.0f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{-0.1f, 0.0f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{0.1f, 0.0f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{-0.1f, 1.2f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_vertices.push_back(
          {{0.1f, 1.2f}, {0.0f, 0.0f}, {0.7f, 0.7f, 0.7f, 1.0f}});
      turret_indices.push_back(precision + 1 + 0);
      turret_indices.push_back(precision + 1 + 1);
      turret_indices.push_back(precision + 1 + 2);
      turret_indices.push_back(precision + 1 + 1);
      turret_indices.push_back(precision + 1 + 2);
      turret_indices.push_back(precision + 1 + 3);
      tank_turret_model_index =
          mgr->RegisterModel(turret_vertices, turret_indices);
    }
  }
}

void Ausy0201Tank::Render() {
  battle_game::SetTransformation(position_, rotation_);
  battle_game::SetTexture(0);
  battle_game::SetColor(game_core_->GetPlayerColor(player_id_));
  battle_game::DrawModel(tank_body_model_index);
  battle_game::SetRotation(turret_rotation_);
  battle_game::DrawModel(tank_turret_model_index);
}

void Ausy0201Tank::Update() {
  TankMove(Max_speed_, Max_angular_speed_);
  TurretRotate();
  Fire();
}

void Ausy0201Tank::TankMove(float move_speed, float rotate_angular_speed) {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    glm::vec2 offset{0.0f, 1.0f};
    int acceleration = 0;
    if (input_data.key_down[GLFW_KEY_W]) {
      acceleration += 1;
    }
    if (input_data.key_down[GLFW_KEY_S]) {
      acceleration -= 1;
    }
    UpdateSpeedScale(acceleration);
    float speed = move_speed * GetSpeedScale();
    offset *= kSecondPerTick * speed;
    auto new_position =
        position_ + glm::vec2{glm::rotate(glm::mat4{1.0f}, rotation_,
                                          glm::vec3{0.0f, 0.0f, 1.0f}) *
                              glm::vec4{offset, 0.0f, 0.0f}};
    if (!game_core_->IsBlockedByObstacles(new_position)) {
      game_core_->PushEventMoveUnit(id_, new_position);
    } else {
      SetSpeedScale(0.0f);
    }
    // printf("speed_scale: %.3f\n", GetSpeedScale());

    float rotation_offset = 1.0f;
    int angular_acceleration = 0;
    if (input_data.key_down[GLFW_KEY_A]) {
      angular_acceleration += 1;
    }
    if (input_data.key_down[GLFW_KEY_D]) {
      angular_acceleration -= 1;
    }
    UpdateAngularSpeedScale(angular_acceleration);
    rotation_offset *=
        kSecondPerTick * rotate_angular_speed * GetAngularSpeedScale();
    game_core_->PushEventRotateUnit(id_, rotation_ + rotation_offset);
    // printf("angular_speed_scale: %.3f\n", GetAngularSpeedScale());
  }
}

void Ausy0201Tank::TurretRotate() {
  auto player = game_core_->GetPlayer(player_id_);
  if (player) {
    auto &input_data = player->GetInputData();
    auto diff = input_data.mouse_cursor_position - position_;
    float target_turret_rotation = rotation_;
    if (glm::length(diff) > 1e-4) {
      target_turret_rotation = std::atan2(diff.y, diff.x) - glm::radians(90.0f);
    }
    UpdateTurretRotation(target_turret_rotation);
    /*
    printf("turret_rotation: %.3f; target_rotation: %.3f\n",
           glm::degrees(turret_rotation_),
           glm::degrees(target_turret_rotation));
    */
  }
}

void Ausy0201Tank::Fire() {
  if (fire_count_down_ == 0) {
    auto player = game_core_->GetPlayer(player_id_);
    if (player) {
      auto &input_data = player->GetInputData();
      if (input_data.mouse_button_down[GLFW_MOUSE_BUTTON_LEFT]) {
        auto velocity = Rotate(glm::vec2{0.0f, 20.0f}, turret_rotation_);
        GenerateBullet<bullet::CannonBall>(
            position_ + Rotate({0.0f, 1.2f}, turret_rotation_),
            turret_rotation_, GetDamageScale(), velocity);
        fire_count_down_ = kTickPerSecond;  // Fire interval 1 second.
      } else if (input_data.mouse_button_down[GLFW_MOUSE_BUTTON_RIGHT]) {
        auto velocity = Rotate(glm::vec2{0.0f, 20.0f}, turret_rotation_);
        GenerateBullet<bullet::Superball>(
            position_ + Rotate({0.0f, 1.2f}, turret_rotation_),
            turret_rotation_, GetDamageScale(), velocity);
        fire_count_down_ = 2*kTickPerSecond;  // Fire interval 22 second.
      }
    }
  }
  if (fire_count_down_) {
    fire_count_down_--;
  }
}

bool Ausy0201Tank::IsHit(glm::vec2 position) const {
  position = WorldToLocal(position);
  return position.x > -0.8f && position.x < 0.8f && position.y > -1.0f &&
         position.y < 1.0f && position.x + position.y < 1.6f &&
         position.y - position.x < 1.6f;
}

void Ausy0201Tank::UpdateSpeedScale(int acceleration) {
  if (!acceleration) {
    speed_scale_ -= speed_scale_ * kSecondPerTick / Decelerate_time_constant_;
  } else if (acceleration == 1) {
    speed_scale_ +=
        (1.0f - speed_scale_) * kSecondPerTick / Accelerate_time_constant_;
  } else {
    speed_scale_ +=
        (-0.4f - speed_scale_) * kSecondPerTick / Accelerate_time_constant_;
  }
}

void Ausy0201Tank::UpdateAngularSpeedScale(int angular_acceleration) {
  if (!angular_acceleration) {
    angular_speed_scale_ -= angular_speed_scale_ * kSecondPerTick /
                            Angular_decelerate_time_constant_;
  } else {
    angular_speed_scale_ += (angular_acceleration - angular_speed_scale_) *
                            kSecondPerTick / Angular_accelerate_time_constant_;
  }
}

void Ausy0201Tank::UpdateTurretRotation(float target_turret_rotation) {
  if(target_turret_rotation - turret_rotation_ > glm::radians(180.0f)) {
    target_turret_rotation -= glm::radians(360.0f);
  } else if(turret_rotation_ - target_turret_rotation > glm::radians(180.0f)) {
    target_turret_rotation += glm::radians(360.0f);
  }
  turret_rotation_ += (target_turret_rotation - turret_rotation_) *
                      kSecondPerTick / turret_rotation_time_constant_;
  if(turret_rotation_ < glm::radians(-270.0f)) {
    turret_rotation_ += glm::radians(360.0f);
  } else if(turret_rotation_ > glm::radians(90.0f)) {
    turret_rotation_ -= glm::radians(360.0f);
  }
}

const char *Ausy0201Tank::UnitName() const {
  return "tank_ausy0201";
}

const char *Ausy0201Tank::Author() const {
  return "AuSY0201";
}
}  // namespace battle_game::unit
