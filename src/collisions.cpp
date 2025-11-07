#include "../include/collision.h"

struct Endpoint {
    float value;
    int box;
    bool isMin;
};

std::vector<std::pair<int,int>> SweepAndPrune(std::vector<AABB>& boxes)
{
    std::vector<std::pair<int,int>> possible;
    std::vector<Endpoint> endpoints;
    endpoints.reserve(boxes.size() * 2);

    // 1. Build endpoints along X
    for (size_t i = 0; i < boxes.size(); ++i)
    {
        endpoints.push_back({ boxes[i].min.x, (int)i, true  });
        endpoints.push_back({ boxes[i].max.x, (int)i, false });
    }

    // 2. Sort by X coordinate
    std::sort(endpoints.begin(), endpoints.end(),
              [](const Endpoint& a, const Endpoint& b){ return a.value < b.value; });

    // 3. Sweep along X
    std::vector<int> active;
    for (auto& e : endpoints)
    {
        if (e.isMin)
        {
            for (int j : active)
            {
                const AABB& A = boxes[e.box];
                const AABB& B = boxes[j];

                // Check Y and Z overlap directly
                bool overlapY = (A.min.y <= B.max.y && A.max.y >= B.min.y);
                bool overlapZ = (A.min.z <= B.max.z && A.max.z >= B.min.z);
                if (overlapY && overlapZ)
                    possible.emplace_back(e.box, j);
            }
            active.push_back(e.box);
        }
        else
        {
            // Remove from active list
            active.erase(std::remove(active.begin(), active.end(), e.box), active.end());
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

bool SphereSphereCollision(const Sphere& s1, const Sphere& s2){
    float distance = glm::length(s1.center - s2.center);
    return distance < (s1.radius + s2.radius);
}