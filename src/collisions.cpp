#include "../include/collision.h"
#include <algorithm>

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


/*#include "../include/collision.h"

using namespace std;

struct BoxEndpoint {
    float value;
    int boxIndex;
    bool isMin;
};

vector<pair<int,int>> SweepAndPrune(vector<AABB>& boxes){
    vector<pair<int, int>> possibleCollisions;

    priority_queue<pair<float, int>, vector<pair<float, int>>, greater<pair<float, int>>> x_endpoints, y_endpoints, z_endpoints; //pair<coord, object>
    vector<pair<int, int>> x_possible, y_possible, z_possible, temp;
    set<int> active; //set auxiliar que tem as bbox que foram abertas mas não fechadas ainda

    //ideia: coloca todos inicios e fins das bbox em um mesmo vetor/pilha, enquanto não fechar o inicio com o fim, adiciona como possivel colisão todos que aparecerem
    for (auto box : boxes){
        x_endpoints.push({box.min.x, box.id});
        x_endpoints.push({box.max.x, box.id});

        y_endpoints.push({box.min.y, box.id});
        y_endpoints.push({box.max.y, box.id});

        z_endpoints.push({box.min.z, box.id});
        z_endpoints.push({box.max.z, box.id});
    }

    //construindo lista de possíveis colisões em x
    while(!x_endpoints.empty()){
        auto atual = x_endpoints.top();
        x_endpoints.pop();

        if(active.find(atual.second) != active.end()){ //se já estava nos ativos, quer dizer que se fecha bbox e coloca todos pares possíveis entre atual e outros que estão ativos

            active.erase(atual.second);

            for (auto object : active){
                if (atual.second > object)
                    x_possible.push_back({object, atual.second});
                else
                    x_possible.push_back({atual.second, object});
            }
        }
        else{ //caso contrário, apenas coloca nos ativos
            active.insert(atual.second);
        }
    }

    while(!y_endpoints.empty()){
        auto atual = y_endpoints.top();
        y_endpoints.pop();

        if(active.find(atual.second) != active.end()){ //se já estava nos ativos, quer dizer que se fecha bbox e coloca todos pares possíveis entre atual e outros que estão ativos

            active.erase(atual.second);

            for (auto object : active){

                if (atual.second > object)
                    y_possible.push_back({object, atual.second});
                else
                    y_possible.push_back({atual.second, object});
            }
        }
        else{ //caso contrário, apenas coloca nos ativos
            active.insert(atual.second);
        }
    }

    while(!z_endpoints.empty()){
        auto atual = z_endpoints.top();
        z_endpoints.pop();

        if(active.find(atual.second) != active.end()){ //se já estava nos ativos, quer dizer que se fecha bbox e coloca todos pares possíveis entre atual e outros que estão ativos

            active.erase(atual.second);

            for (auto object : active){
                if (atual.second > object)
                    z_possible.push_back({object, atual.second});
                else
                    z_possible.push_back({atual.second, object});
            }
        }
        else{ //caso contrário, apenas coloca nos ativos
            active.insert(atual.second);
        }
    }

    //intersecção entre os 3 vetores de possíveis colisões
    sort(x_possible.begin(), x_possible.end());
    sort(y_possible.begin(), y_possible.end());
    sort(z_possible.begin(), z_possible.end());

    set_intersection(x_possible.begin(), x_possible.end(),
                    y_possible.begin(), y_possible.end(),
                    back_inserter(temp));

    set_intersection(temp.begin(), temp.end(),
                    z_possible.begin(), z_possible.end(),
                    back_inserter(possibleCollisions));

    return possibleCollisions;
}
    */