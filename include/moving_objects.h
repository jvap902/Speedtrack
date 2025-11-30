#pragma once

#include <utility>
#include <vector>
#include <glm/glm.hpp>
#include "types.h"

void CarMovingSphere(const glm::mat4 sphere_model_matrix, const glm::mat4& car_model_matrix, CarState& car, glm::vec3 largestMtv, MovingSphereState& sphere);
void SphereControl(MovingSphereState& sphere, float deltaTime);
void SphereSphereBounce( const glm::mat4& modelA, float scaleA, int idA, const glm::mat4& modelB, float scaleB, int idB, MovingSphereState& movingSphere, const Sphere& localSphereHull);