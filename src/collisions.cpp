#include "collisions.h"
#include <unordered_map> // Needed for the hash map optimization
#include <algorithm>
#include <glm/gtx/norm.hpp>

struct Endpoint {
    float value;
    int boxId;
    bool isMin;
};

std::set<std::pair<int,int>> SweepAndPrune(const std::vector<AABB>& boxes)
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

    // Precompute fast lookup from id -> box
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

        // Build local bounding box
        for (size_t i = 0; i < shape.mesh.indices.size(); ++i)
        {
            int idx = shape.mesh.indices[i].vertex_index * 3;
            if (idx + 2 >= (int)vertices.size()) continue;

            glm::vec3 v(vertices[idx + 0], vertices[idx + 1], vertices[idx + 2]);
            minBounds = glm::min(minBounds, v);
            maxBounds = glm::max(maxBounds, v);
        }

        glm::vec3 center   = (minBounds + maxBounds) * 0.5f;
        glm::vec3 halfSize = (maxBounds - minBounds) * 0.5f;

        // Transform center to world space
        glm::vec3 worldCenter = glm::vec3(transform * glm::vec4(center, 1.0f));

        // Extract world axes from rotation part
        glm::vec3 worldAxisX = glm::normalize(glm::vec3(transform[0]));
        glm::vec3 worldAxisY = glm::normalize(glm::vec3(transform[1]));
        glm::vec3 worldAxisZ = glm::normalize(glm::vec3(transform[2]));

        // Apply scaling from model matrix to halfSize
        glm::vec3 scaledHalfSize = glm::vec3(
            halfSize.x * glm::length(glm::vec3(transform[0])),
            halfSize.y * glm::length(glm::vec3(transform[1])),
            halfSize.z * glm::length(glm::vec3(transform[2]))
        );

        // Build final OBB
        OBB obb;
        obb.center = worldCenter;
        obb.halfSize = scaledHalfSize;
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

bool AabbObbCollision(const AABB& aabb, const OBB& obb, glm::vec3& mtv)
{
    // AABB center + half size
    glm::vec3 aCenter = (aabb.min + aabb.max) * 0.5f;
    glm::vec3 aHalf   = (aabb.max - aabb.min) * 0.5f;

    glm::vec3 aAxes[3] = {
        glm::vec3(1,0,0),
        glm::vec3(0,1,0),
        glm::vec3(0,0,1)
    };

    float bestOverlap = FLT_MAX;
    glm::vec3 bestAxis(0.0f);

    glm::vec3 delta = obb.center - aCenter;

    auto testAxis = [&](const glm::vec3& rawAxis)
    {
        glm::vec3 axis = glm::normalize(rawAxis);

        float aProj =
            fabs(glm::dot(aAxes[0], axis)) * aHalf.x +
            fabs(glm::dot(aAxes[1], axis)) * aHalf.y +
            fabs(glm::dot(aAxes[2], axis)) * aHalf.z;

        float bProj =
            fabs(glm::dot(obb.axis[0], axis)) * obb.halfSize.x +
            fabs(glm::dot(obb.axis[1], axis)) * obb.halfSize.y +
            fabs(glm::dot(obb.axis[2], axis)) * obb.halfSize.z;

        float dist = fabs(glm::dot(delta, axis));

        float overlap = aProj + bProj - dist;

        if (overlap < 0.0f)
            return false;  // separating axis found

        if (overlap < bestOverlap)
        {
            bestOverlap = overlap;

            glm::vec3 fixedAxis = axis;
            if (glm::dot(delta, fixedAxis) < 0)
                fixedAxis = -fixedAxis;

            bestAxis = fixedAxis;
        }
        return true;
    };

    // Test AABB axes
    for (int i = 0; i < 3; i++)
        if (!testAxis(aAxes[i])) return false;

    // Test OBB axes
    for (int i = 0; i < 3; i++)
        if (!testAxis(obb.axis[i])) return false;

    // Test cross axes (AABB vs OBB)
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            glm::vec3 axis = glm::cross(aAxes[i], obb.axis[j]);
            if (glm::length2(axis) > 1e-6f)  // avoid degenerate axis
            {
                if (!testAxis(axis)) return false;
            }
        }
    }

    mtv = bestAxis * bestOverlap;
    return true;
}
// It now asks for the data it needs instead of guessing global variables
void BuildBBoxArray(const std::map<std::string, SceneObject>& virtualScene, std::vector<AABB>& boxes, int& bboxId, const std::string& name, const glm::mat4& modelMatrix, int objectId) {

    // Use the passed map, not g_VirtualScene
    const auto& obj = virtualScene.at(name);

    // Local-space AABB corners
    glm::vec3 minLocal = obj.bbox_min;
    glm::vec3 maxLocal = obj.bbox_max;

    glm::vec3 corners[8] = {
        {minLocal.x, minLocal.y, minLocal.z},
        {maxLocal.x, minLocal.y, minLocal.z},
        {minLocal.x, maxLocal.y, minLocal.z},
        {minLocal.x, minLocal.y, maxLocal.z},
        {maxLocal.x, maxLocal.y, minLocal.z},
        {minLocal.x, maxLocal.y, maxLocal.z},
        {maxLocal.x, minLocal.y, maxLocal.z},
        {maxLocal.x, maxLocal.y, maxLocal.z}
    };

    glm::vec3 worldMin(FLT_MAX);
    glm::vec3 worldMax(-FLT_MAX);

    // Transform all corners by model matrix and recompute world AABB
    for (auto& c : corners)
    {
        glm::vec4 transformed = modelMatrix * glm::vec4(c, 1.0f);
        worldMin = glm::min(worldMin, glm::vec3(transformed));
        worldMax = glm::max(worldMax, glm::vec3(transformed));
    }

    AABB box;
    box.min = worldMin;
    box.max = worldMax;
    box.id = bboxId++;
    box.objectId = objectId;

    boxes.push_back(box);
}

OBB TransformOBB(const OBB& localBox, const glm::mat4& transform) {
    OBB worldBox;

    // Transform center
    worldBox.center = glm::vec3(transform * glm::vec4(localBox.center, 1.0f));
    worldBox.id = localBox.id;

    // Transform axes (assumes local axes are 1,0,0 etc.)
    worldBox.axis[0] = glm::normalize(glm::vec3(transform[0])); // X-axis
    worldBox.axis[1] = glm::normalize(glm::vec3(transform[1])); // Y-axis
    worldBox.axis[2] = glm::normalize(glm::vec3(transform[2])); // Z-axis

    // Apply scale from the transform to the halfSize
    float scale = glm::length(glm::vec3(transform[0]));
    worldBox.halfSize = localBox.halfSize * scale;

    return worldBox;
}

bool SphereSphereCollision(const ObjModel& obj1, const ObjModel& obj2, glm::mat4 object1_matrix, int object1_id, glm::mat4 object2_matrix, int object2_id, float object1_uniformScale, float object2_UniformScale){

    Sphere boundingSphere1 = BoundingSphere(obj1, object1_id);
    Sphere boundingSphere2 = BoundingSphere(obj2, object2_id);

    glm::vec3 worldCenter = glm::vec3(object1_matrix * glm::vec4(boundingSphere1.center, 1.0f));
    float worldRadius = boundingSphere1.radius * object1_uniformScale;

    Sphere worldSphereObj2 = { worldCenter, worldRadius, boundingSphere1.id };

    worldCenter = glm::vec3(object2_matrix * glm::vec4(boundingSphere2.center, 1.0f));
    worldRadius = boundingSphere2.radius * object2_UniformScale;

    Sphere worldSphereObj1 = { worldCenter, worldRadius, boundingSphere2.id };

    if (SSCollision(worldSphereObj1, worldSphereObj2))
    {
        return true;
    }

    return false;
}