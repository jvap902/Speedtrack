#include "moving_objects.h"
#include <algorithm>
#include "collisions.h"

void SphereControl(MovingSphereState& sphere, float deltaTime)
{
    const float friction = 2.0f;      // deceleration per second
    const float EPS_SPEED = 1e-4f;
    const float sphereRadius = 1.0f;  // use the same logical radius you render with

    // Apply friction
    if (sphere.speed > EPS_SPEED)
    {
        sphere.speed -= friction * deltaTime;
        if (sphere.speed < 0.0f) sphere.speed = 0.0f;
    }
    else if (sphere.speed < -EPS_SPEED)
    {
        sphere.speed += friction * deltaTime;
        if (sphere.speed > 0.0f) sphere.speed = 0.0f;
    }
    else
    {
        sphere.speed = 0.0f;
    }

    // Nothing to do if not moving
    if (sphere.speed <= 0.0f) return;

    // Move sphere along its direction
    sphere.position += sphere.direction * sphere.speed * deltaTime;

    // Update rolling angle (distance/radius -> radians, convert to degrees because you use degrees)
    float distance = sphere.speed * deltaTime;
    float deltaAngleRad = distance / sphereRadius;
    sphere.angle += glm::degrees(deltaAngleRad);

    // stabilize angle
    if (sphere.angle > 360.0f) sphere.angle = fmod(sphere.angle, 360.0f);
    if (sphere.angle < 0.0f)   sphere.angle = fmod(sphere.angle, 360.0f) + 360.0f;
}


void CarMovingSphere(const glm::mat4 sphere_model_matrix, const glm::mat4& car_model_matrix, CarState& car, glm::vec3 largestMtv, MovingSphereState& sphere)
{
    // --- 1) Separate objects immediately so they aren't overlapping anymore ---
    // We will push the car a bit (as before) and also nudge the sphere the opposite way.
    // Split the correction so both move out of penetration (tweak factors if needed).
    const float carPushFactor = 0.65f;
    const float spherePushFactor = 0.35f;

    glm::vec3 carPush = largestMtv * carPushFactor;
    glm::vec3 spherePush = -largestMtv * spherePushFactor; // push sphere opposite to car push

    car.position += carPush;
    sphere.position += spherePush;

    // --- 2) Compute horizontal push direction for the sphere.
    // The collision MTV (largestMtv) is the vector that pushes the car out.
    // The sphere should be pushed in the opposite horizontal direction.
    glm::vec3 horizontal = glm::vec3(-largestMtv.x, 0.0f, -largestMtv.z); // NOTE the negation

    // Fallback: if horizontal is tiny (vertical hit), use car forward vector
    if (glm::length(horizontal) < 1e-4f)
    {
        glm::vec3 carForward = glm::normalize(glm::vec3(car_model_matrix * glm::vec4(0,0,-1,0)));
        horizontal = glm::vec3(carForward.x, 0.0f, carForward.z);
    }

    if (glm::length(horizontal) > 1e-5f)
        sphere.direction = glm::normalize(horizontal);
    else
        sphere.direction = glm::vec3(0.0f, 0.0f, 1.0f); // safe default

    // --- 3) Transfer speed from car to sphere (with a minimum)
    float transferred = glm::abs(car.speed) * 0.4f;
    const float MIN_PUSH_SPEED = 1.0f; // tune to taste
    sphere.speed = glm::max(transferred, MIN_PUSH_SPEED);

    // --- 4) Damp car speed (impulse)
    car.speed *= -0.25f;
}

void SphereSphereBounce(const glm::mat4& moving_model, float moving_scale, int moving_id, const glm::mat4& static_model, float static_scale, int static_id, MovingSphereState& movingSphere, const Sphere& localHull)
{
    // --- 1. Get world centers ---
    glm::vec3 Cmoving = glm::vec3(moving_model[3]);
    glm::vec3 Cstatic = glm::vec3(static_model[3]);

    // --- 2. Compute radii ---
    float Rmoving = moving_scale * 0.5f;
    float Rstatic = static_scale * 0.5f;

    // --- 3. Horizontal only ---
    Cmoving.y = 0;
    Cstatic.y = 0;

    // --- 4. Compute vector between centers ---
    glm::vec3 diff = Cmoving - Cstatic;
    float dist = glm::length(diff);

    if (dist < 0.0001f)
        return;  // avoid freak cases

    glm::vec3 normal = diff / dist;  // from static toward moving

    // --- 5. Push moving sphere out (MTV) ---
    float penetration = (Rmoving + Rstatic) - dist;
    if (penetration > 0)
        movingSphere.position += normal * penetration;

    // --- 6. Reflect velocity ---
    glm::vec3 vel = movingSphere.direction * movingSphere.speed;

    float dotN = glm::dot(vel, normal);
    if (dotN < 0) // sphere is moving into static sphere
    {
        glm::vec3 reflected = vel - 2.0f * dotN * normal;

        // damping
        reflected *= 0.8f;

        movingSphere.speed = glm::length(reflected);
        if (movingSphere.speed > 0.0001f)
            movingSphere.direction = glm::normalize(reflected);
        else
            movingSphere.direction = glm::vec3(0);
    }
}

bool SphereOBBCollision_ForSphere(const OBB& box, const Sphere& sphere, glm::vec3& mtvOut)
{
    glm::vec3 d = sphere.center - box.center;
    glm::vec3 closest = box.center;

    // Find closest point on OBB
    for (int i = 0; i < 3; ++i)
    {
        float dist = glm::dot(d, box.axis[i]);
        dist = glm::clamp(dist, -box.halfSize[i], box.halfSize[i]);
        closest += dist * box.axis[i];
    }

    glm::vec3 toSphere = sphere.center - closest;
    float dist2 = glm::dot(toSphere, toSphere);

    if (dist2 >= sphere.radius * sphere.radius)
        return false;  // no collision

    float dist = sqrt(dist2);

    glm::vec3 normal;
    if (dist < 1e-6f)
        normal = glm::normalize(sphere.center - box.center);
    else
        normal = toSphere / dist;

    // MTV must push sphere OUTWARDS
    float penetration = sphere.radius - dist;
    mtvOut = normal * penetration;

    return true;
}


bool SphereBarrierCollision(const glm::mat4& sphere_model_matrix, float sphereScale, const AABB& worldBarrierAABB, MovingSphereState& sphere)
{
    float radius = sphereScale * 0.9f;

    Sphere worldSphere;
    worldSphere.center = sphere.position;
    worldSphere.radius = radius;

    OBB box;
    box.center   = (worldBarrierAABB.min + worldBarrierAABB.max) * 0.5f;
    box.halfSize = (worldBarrierAABB.max - worldBarrierAABB.min) * 0.5f;
    box.axis[0] = glm::vec3(1,0,0);
    box.axis[1] = glm::vec3(0,1,0);
    box.axis[2] = glm::vec3(0,0,1);

    glm::vec3 mtv;
    if (!SphereOBBCollision_ForSphere(box, worldSphere, mtv))
        return false;

    // Fix position
    sphere.position += mtv;

    // Horizontal only
    glm::vec3 normal = glm::normalize(glm::vec3(mtv.x, 0, mtv.z));

    glm::vec3 velocity = sphere.direction * sphere.speed;

    float dotN = glm::dot(velocity, normal);

    if (dotN < 0)
    {
        glm::vec3 reflected = velocity - 2.0f * dotN * normal;
        reflected *= 0.7f;

        sphere.speed = glm::length(reflected);
        sphere.direction = (sphere.speed > 0.001f ? glm::normalize(reflected)
                                                  : glm::vec3(0));
    }

    return true;
}