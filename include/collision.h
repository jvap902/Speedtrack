#include <glm/glm.hpp>
#include <bits/stdc++.h>

// Simple bounding box representation
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
    int id; // optional: useful for debugging (e.g., "car", "tree")
};

// Returns pairs of indices (or object names) that may collide
std::vector<std::pair<int, int>> SweepAndPrune(std::vector<AABB>& boxes);

