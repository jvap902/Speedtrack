#include "../include/collisions.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>

struct Endpoint {
    float value;
    int boxId;
    bool isMin;
};

std::set<std::pair<int,int>> SweepAndPrune(std::vector<AABB>& boxes)
{
    std::set<std::pair<int,int>> possible;
    std::vector<Endpoint> endpoints;
    endpoints.reserve(boxes.size() * 2);

    // Build endpoints along X
    for (const auto& box : boxes)
    {
        endpoints.push_back({ box.min.x, box.id, true  });
        endpoints.push_back({ box.max.x, box.id, false });
    }

    // Sort by coordinate
    std::sort(endpoints.begin(), endpoints.end(),
              [](const Endpoint& a, const Endpoint& b){ return a.value < b.value; });

    // Precompute fast lookup from id → box
    std::unordered_map<int, const AABB*> idToBox;
    idToBox.reserve(boxes.size());
    for (const auto& box : boxes)
        idToBox[box.id] = &box;

    // Active list (contains box IDs)
    std::vector<int> active;
    active.reserve(boxes.size());

    // Sweep
    for (const auto& e : endpoints)
    {
        if (e.isMin)
        {
            const AABB* A = idToBox[e.boxId];
            for (int activeId : active)
            {
                const AABB* B = idToBox[activeId];

                // YZ overlap check
                bool overlapY = (A->min.y <= B->max.y && A->max.y >= B->min.y);
                bool overlapZ = (A->min.z <= B->max.z && A->max.z >= B->min.z);

                if (overlapY && overlapZ)
                    if(A->objectId != B->objectId)
                        possible.insert({std::min(A->objectId, B->objectId), std::max(A->objectId, B->objectId)});
            }

            active.push_back(e.boxId);
        }
        else
        {
            // Remove when we reach the max endpoint
            active.erase(std::remove(active.begin(), active.end(), e.boxId), active.end());
        }
    }

    return possible;
}

bool RaySphere(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const Sphere& sphere, float & t_hit){

    glm::vec3 oc = rayOrigin - sphere.center;

    float a = glm::dot(rayDir, rayDir);
    float b = 2.0f * glm::dot(oc, rayDir);
    float c = glm::dot(oc, oc) - sphere.radius * sphere.radius;

    float discriminant = b*b - 4*a*c;
    if (discriminant < 0.0f)
        return false;  // no intersection

    float sqrt_disc = sqrt(discriminant);
    float t1 = (-b - sqrt_disc) / (2.0f * a);
    float t2 = (-b + sqrt_disc) / (2.0f * a);

    // check if intersection occurs in front of ray
    if (t1 >= 0.0f) {
        t_hit = t1;
        return true;
    } else if (t2 >= 0.0f) {
        t_hit = t2;
        return true;
    }

    return false;
}

Sphere BoundingSphere(const ObjModel& model, int id)
{
    const auto& verts = model.attrib.vertices;
    if (verts.empty())
        return { glm::vec3(0.0f), 0.0f, id };

    glm::vec3 min( FLT_MAX);
    glm::vec3 max(-FLT_MAX);

    // Pass 1: find AABB
    for (size_t i = 0; i < verts.size(); i += 3)
    {
        glm::vec3 v(verts[i+0], verts[i+1], verts[i+2]);
        min = glm::min(min, v);
        max = glm::max(max, v);
    }

    // Pass 2: center = midpoint of AABB
    glm::vec3 center = (min + max) * 0.5f;

    // Pass 3: radius = max distance from center
    float radius = 0.0f;
    for (size_t i = 0; i < verts.size(); i += 3)
    {
        glm::vec3 v(verts[i+0], verts[i+1], verts[i+2]);
        float dist = glm::length(v - center);
        if (dist > radius)
            radius = dist;
    }

    return { center, radius, id };
}

bool SSCollision(const Sphere& s1, const Sphere& s2){
    float distance = glm::length(s1.center - s2.center);
    return distance < (s1.radius + s2.radius);
}

// Builds a compound hitbox (multiple OBBs) from an ObjModel
std::vector<OBB> BuildCompoundHitbox(const ObjModel& model, const glm::mat4& transform, int id)
{
    std::vector<OBB> hitboxes;
    const auto& vertices = model.attrib.vertices;

    for (const auto& shape : model.shapes)
    {
        glm::vec3 minBounds( FLT_MAX);
        glm::vec3 maxBounds(-FLT_MAX);

        // Each shape has its own mesh of indices
        for (size_t i = 0; i < shape.mesh.indices.size(); ++i)
        {
            int idx = shape.mesh.indices[i].vertex_index * 3;
            if (idx + 2 >= (int)vertices.size()) continue;

            glm::vec3 v(vertices[idx + 0], vertices[idx + 1], vertices[idx + 2]);
            minBounds = glm::min(minBounds, v);
            maxBounds = glm::max(maxBounds, v);
        }

        // Compute center and half-size in object space
        glm::vec3 center = (minBounds + maxBounds) * 0.5f;
        glm::vec3 halfSize = (maxBounds - minBounds) * 0.5f;

        // Transform center into world space
        glm::vec3 worldCenter = glm::vec3(transform * glm::vec4(center, 1.0f));

        // Extract world axes from the model matrix
        glm::vec3 worldAxisX = glm::normalize(glm::vec3(transform[0]));
        glm::vec3 worldAxisY = glm::normalize(glm::vec3(transform[1]));
        glm::vec3 worldAxisZ = glm::normalize(glm::vec3(transform[2]));

        OBB obb;
        obb.center = worldCenter;
        obb.halfSize = halfSize;
        obb.axis[0] = worldAxisX;
        obb.axis[1] = worldAxisY;
        obb.axis[2] = worldAxisZ;
        obb.id = id;

        hitboxes.push_back(obb);
    }

    return hitboxes;
}

bool CHitboxSphereCollision(const OBB& box, const Sphere& sphere, glm::vec3& mtv){
    glm::vec3 d = sphere.center - box.center;
    glm::vec3 closest = box.center;

    // Find closest point on OBB to sphere center
    for (int i = 0; i < 3; ++i)
    {
        float dist = glm::dot(d, box.axis[i]);
        dist = glm::clamp(dist, -box.halfSize[i], box.halfSize[i]);
        closest += dist * box.axis[i];
    }

    glm::vec3 toSphere = sphere.center - closest;
    float distanceSq = glm::dot(toSphere, toSphere);

    // No collision
    if (distanceSq >= (sphere.radius * sphere.radius))
        return false;

    // Collision detected! Calculate MTV.
    float distance = std::sqrt(distanceSq);
    float penetrationDepth = sphere.radius - distance;
    glm::vec3 mtv_direction;

    if (distance < 0.00001f)
    {
        // Sphere center is right on (or inside) the closest point.
        // We need a fallback direction. Use box-to-sphere center.
        mtv_direction = glm::normalize(sphere.center - box.center);
        
        // Failsafe for if they are at the *exact* same spot
        if (glm::length(mtv_direction) < 0.0001f)
             mtv_direction = glm::vec3(0, 1, 0); // Push up
    }
    else
    {
        mtv_direction = toSphere / distance; // Normalized vector from closest point to sphere center
    }
    
    // We want the vector to push the *box* (the car)
    // It should be in the opposite direction of the penetration.
    mtv = -mtv_direction * penetrationDepth;
    return true;
}