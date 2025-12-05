#include "car.h"
#include "collisions.h" // Needed for TransformOBB, CHitboxSphereCollision
#include <cmath>
#include <algorithm>
#include "moving_objects.h"

// Internal helper to keep code clean (optional, but good practice)
// Note: Since we pass state, we don't strictly need this global,
// but we can keep acceleration local to the function if we want,
// or recalculate it every frame.
// For simplicity, I'll calculate it locally.


void CarControl(CarState& car, const InputState& input, float deltaTime) {
    // Parameters
    const float max_accel = 10.0f;
    const float max_speed = 40.0f;
    const float turn_speed = 50.0f;
    const float friction = 8.0f;

    float acceleration = 0.0f;

    if (input.w)
        acceleration = max_accel;
    else if (input.s)
        acceleration = -max_accel;

    // Update speed
    car.speed += acceleration * deltaTime;

    // Apply friction
    if (!input.w && !input.s)
    {
        if (car.speed > 0)
        {
            car.speed -= friction * deltaTime;
            if (car.speed < 0) car.speed = 0;
        }
        else if (car.speed < 0)
        {
            car.speed += friction * deltaTime;
            if (car.speed > 0) car.speed = 0;
        }
    }

    // Limit max speed
    if (car.speed >  max_speed) car.speed =  max_speed;
    if (car.speed < -max_speed) car.speed = -max_speed;

    // Steering logic
    float abs_speed = std::abs(car.speed);
    if(abs_speed > 0.5f){
        // Car turns tighter when slower? Or turns normally.
        // Your original logic:
        if (input.a){
            // operation at the end corrects direction when reversing
            car.angle += (turn_speed * deltaTime * max_speed / (abs_speed + 10.0f)) * (car.speed / abs_speed);
        }
        else if (input.d){
            car.angle -= (turn_speed * deltaTime * max_speed / (abs_speed + 10.0f)) * (car.speed / abs_speed);
        }
    }

    // Wrap angle
    if(car.angle >= 360.0f)
        car.angle -= 360.0f;
    else if(car.angle <= -360.0f)
        car.angle += 360.0f;

    // Local Movement (relative to car angle)
    float rad_angle = glm::radians(car.angle);
    glm::vec3 forward = glm::vec3(sin(rad_angle), 0.0f, cos(rad_angle));

    car.position.x += forward.x * car.speed * deltaTime;
    car.position.z += forward.z * car.speed * deltaTime;
}


void TreatCarSphereCollision(const glm::mat4& sphere_model_matrix, float sphereUniformScale, const glm::mat4& car_model_matrix, const std::pair<int,int>& collision, CarState& car, const glm::vec3& last_pos, const Sphere& localSphereHull, const std::vector<OBB>& localCarHulls, MovingSphereState& sphere)
{
    // 1. Get World-Space Sphere (from passed local hull)
    glm::vec3 worldCenter = glm::vec3(sphere_model_matrix * glm::vec4(localSphereHull.center, 1.0f));
    float worldRadius = localSphereHull.radius * (sphereUniformScale * 0.9f); //0.9f para ficar mais perto
    Sphere worldSphere = { worldCenter, worldRadius, localSphereHull.id };

    glm::vec3 largestMtv(0.0f);
    bool hasCollided = false;

    // 2. Get World-Space Car Hitboxes (from passed local hulls)
    for (const auto& localBox : localCarHulls)
    {
        // Transform local OBB to world OBB
        OBB worldBox = TransformOBB(localBox, car_model_matrix);

        glm::vec3 mtv;

        if (CHitboxSphereCollision(worldBox, worldSphere, mtv))
        {
            hasCollided = true;
            // Find largest pushback
            if (glm::length(mtv) > glm::length(largestMtv))
            {
                largestMtv = mtv;
            }
        }
    }

    // 3. Apply response
    if (hasCollided)
    {
        if (collision.second == SPHERE2)
            CarMovingSphere(sphere_model_matrix, car_model_matrix, car, largestMtv, sphere);

        else {
            // Pushback
            largestMtv.y = 0.0f;


            car.position.x += largestMtv.x;
            car.position.y += largestMtv.y;
            car.position.z += largestMtv.z;

            // Stop velocity
            car.speed = -car.speed * 0.4f;
        }
    }
}

void TreatCarBarrierCollision(const glm::mat4& car_model_matrix, CarState& car, const glm::vec3& last_pos, float& last_angle, const std::vector<OBB>& localCarHulls, const std::vector<AABB>& localBarrierHulls)
{
    for (auto localBarrierHull : localBarrierHulls){
        bool hasCollided = false;

        // Get Barrier AABB
        AABB worldBarrierAABB = localBarrierHull;

        // Car forward vector (Option A)
        glm::vec3 carForward = glm::normalize(glm::vec3(car_model_matrix * glm::vec4(0,0,-1,0)));

        // Test all OBB hitboxes of car
        for (const auto& localBox : localCarHulls)
        {
            OBB worldBox = TransformOBB(localBox, car_model_matrix);

            glm::vec3 mtv;
            if (AabbObbCollision(worldBarrierAABB, worldBox))
            {
                hasCollided = true;
            }
        }

        if (hasCollided)
        {

            car.position = last_pos;
            car.angle = last_angle;

            car.speed = -car.speed * 0.4f;
        }
    }
}