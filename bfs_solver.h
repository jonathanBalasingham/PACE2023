//
// Created by jonathan on 5/13/23.
//

#ifndef PACE_BFS_SOLVER_H
#define PACE_BFS_SOLVER_H

#include <memory>
#include <utility>
#include <iostream>
#include "graph.h"

using namespace std;

class BFSSolver {
private:
    shared_ptr<Graph> g;
    vector<pair<int, int>> cs;
    map<pair<int, int>, int> cache;
public:
    vector<pair<int, int>> solve(shared_ptr<Graph> G, const set<int>& subset) {
        cache = {};
        g = G;
        int original_size = g->order();

        int cn = *subset.begin();
        cn = inner_solve(cn, subset);

        if (original_size - G->order() + 1 == subset.size()){
            return cs;
        }

        auto v = g->vertices_as_set();
        int new_starter;
        int to_remove = subset.size();
        while (to_remove > 1) {
            for (auto vertex : v) {
                if (vertex != cn and subset.contains(vertex))
                    new_starter = vertex;
            }
            cs.emplace_back(new_starter, cn);
            g->contract(new_starter, cn);
            cn = inner_solve(new_starter, subset);
            v = g->vertices_as_set();
            to_remove--;
        }

        return cs;
    }

    vector<pair<int, int>> get_cs(){
        return cs;
    }

    int inner_solve(int current_node, const set<int>& subset) {
        const auto cn = g->neighbors_at(current_node);
        int crd = g->get_max_red_degree();
        int i = 0;

        int num_eligible = 0;
        for (auto &n : *cn) {
            if (subset.contains(n.first))
                num_eligible++;
        }

        if (cn->empty() or num_eligible == 0) {
            return current_node;
        }

        int best_score = crd;
        int chosen_node;
        for (const auto& nbr : *cn) {
            if (!subset.contains(nbr.first))
                continue;

            int scores;
            if (cache.contains({std::min(current_node, nbr.first), std::max(current_node, nbr.first)})){
                scores = crd - cache.at({std::min(current_node, nbr.first), std::max(current_node, nbr.first)});
            } else {
                int rd = g->red_degree_after_contraction(current_node, nbr.first);
                scores = crd - rd;
                cache.insert({{std::min(current_node, nbr.first), std::max(current_node, nbr.first)}, rd});
            }

            if (i == 0) {
                best_score = scores;
                chosen_node = nbr.first;
            } else if(scores > best_score) {
                best_score = scores;
                chosen_node = nbr.first;
            }

            i++;
            if (i > 0)
                break;
        }
        //g->contract(current_node, chosen_node);
        g->contract(chosen_node, current_node);

        //cs.emplace_back(current_node, chosen_node);
        cs.emplace_back(chosen_node, current_node);

        std::cout << "\r" << "Graph Size: " << g->order();
        return inner_solve(chosen_node, subset);
    }
};

#endif //PACE_BFS_SOLVER_H
