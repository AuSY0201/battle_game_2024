#include "battle_game/core/bullets/superball.h"

#include "battle_game/core/game_core.h"
#include "battle_game/core/particles/particles.h"

namespace battle_game::bullet {
Superball::Superball(GameCore *core,
                     uint32_t id,
                     uint32_t unit_id,
                     uint32_t player_id,
                     glm::vec2 position,
                     float rotation,
                     float damage_scale,
                     glm::vec2 velocity)
    : Bullet(core, id, unit_id, player_id, position, rotation, damage_scale),
      velocity_(velocity) {
}

Superball::Superball(GameCore *core,
                     uint32_t id,
                     uint32_t unit_id,
                     uint32_t player_id,
                     glm::vec2 position,
                     float rotation,
                     float damage_scale,
                     glm::vec2 velocity,
                     float angular_velocity)
    : Bullet(core, id, unit_id, player_id, position, rotation, damage_scale),
      velocity_(velocity),
      angular_velocity_(angular_velocity) {
}

void Superball::Render() {
  SetTransformation(position_, rotation_, glm::vec2{0.5f});
  SetColor(game_core_->GetPlayerColor(player_id_));
  SetTexture(BATTLE_GAME_ASSETS_DIR "textures/particle_superball.png");
  DrawModel(0);
}

void Superball::Update() {
  life_count_down_--;
  rotation_ += angular_velocity_ * kSecondPerTick;
  bool should_die = false;

  auto target_position = position_ + velocity_ * kSecondPerTick;
  if (game_core_->IsOutOfRange(target_position)) {
    DealCollision(game_core_->GetBoundaryNormalDirection(target_position));
  } else if (game_core_->IsBlockedByObstacles(target_position)) {
    auto obstacle = game_core_->GetBlockedObstacle(target_position);
    auto collision_event =
        obstacle->GetSurfaceNormal(position_, target_position);
    if (collision_event.second.x == 0.0f && collision_event.second.y == 0.0f) {
      /*
       * For those obstacles that does not have the GetSurfaceNormal
       * function implemented, we treat them as a block that can absorb the
       * bullet. Therefore, we call on collaborators who created any obstacle
       * to implement the GetSurfaceNormal function.
       */
      should_die = true;
    } else {
      position_ = collision_event.first;
      glm::vec2 normal_vec = collision_event.second;
      float normal_rotation =
          std::atan2(normal_vec.y, normal_vec.x) - glm::radians(90.0f);
      DealCollision(normal_rotation);
    }
  } else {
    position_ += velocity_ * kSecondPerTick;
  }

  if (life_count_down_ <= 0 or bounce_count_down_ <= 0) {
    should_die = true;
  }

  auto &units = game_core_->GetUnits();
  for (auto &unit : units) {
    /* Avoid friendly fire
    if (unit.first == unit_id_) {
      continue;
    }
    */
    if (unit.second->IsHit(position_)) {
      game_core_->PushEventDealDamage(
          unit.first, id_,
          // We encourage higher angular velocity
          (damage_scale_ + glm::abs(angular_velocity_) / 10.0f) * 15.0f);
      should_die = true;
    }
  }

  if (should_die) {
    game_core_->PushEventRemoveBullet(id_);
  }
}

void Superball::DealCollision(float normal_rotation) {
  velocity_ = glm::vec2{glm::rotate(glm::mat4{1.0f}, normal_rotation * (-1),
                                    glm::vec3{0.0f, 0.0f, 1.0f}) *
                        glm::vec4{velocity_, 0.0f, 0.0f}};
  velocity_.y *= -1;
  float x_velocity =
      velocity_.x * 3.0f / 7.0f - angular_velocity_ * radius_ * 4.0f / 7.0f;
  float angular_velocity =
      angular_velocity_ * (-3.0f) / 7.0f - velocity_.x * 10.0f / 7.0f / radius_;
  velocity_.x = x_velocity;
  angular_velocity_ = angular_velocity;
  velocity_ = glm::vec2{glm::rotate(glm::mat4{1.0f}, normal_rotation,
                                    glm::vec3{0.0f, 0.0f, 1.0f}) *
                        glm::vec4{velocity_, 0.0f, 0.0f}};
  bounce_count_down_--;
}

Superball::~Superball() {
  for (int i = 0; i < 5; i++) {
    game_core_->PushEventGenerateParticle<particle::Smoke>(
        position_, rotation_, game_core_->RandomInCircle() * 2.0f, 0.2f,
        glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}, 3.0f);
  }
}
}  // namespace battle_game::bullet
