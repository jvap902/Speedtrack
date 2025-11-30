#pragma once

#include <utility>
#include <vector>
#include <glm/glm.hpp>
#include "types.h"

// Controls the car physics.
// deltaTime: time elapsed since last frame (in seconds)
void CarControl(CarState& car, const InputState& input, float deltaTime);

// Handles collision response.
// Now requires the collision hulls (localSphereHull, localCarHulls) to be passed in.
void TreatCarSphereCollision(const glm::mat4& sphere_model_matrix, float sphereUniformScale, const glm::mat4& car_model_matrix, const std::pair<int,int>& collision, CarState& car, const glm::vec3& last_pos, const Sphere& localSphereHull, const std::vector<OBB>& localCarHulls, MovingSphereState& sphere);
void TreatCarBarrierCollision(const glm::mat4& car_model_matrix, const std::pair<int,int>& collision, CarState& car, const glm::vec3& last_pos, const std::vector<OBB>& localCarHulls, const AABB& localBarrierHull);