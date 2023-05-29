//
// Created by jonathan on 5/17/23.
//

#ifndef PACE_QUICK_SOLVER_H
#define PACE_QUICK_SOLVER_H

#include "Distribution.h"

using namespace std;

class QuickSolver {
private:
    vector<boost::dynamic_bitset<>> edge_sets;
    map<boost::dynamic_bitset<>, int> map_to_nodes;
    map<int, boost::dynamic_bitset<>&> map_to_bitsets;
    unique_ptr<Sampler> s;
    //set<int> removed_nodes;
    vector<pair<int, int>> solution;
    std::unique_ptr<graph> G_ptr;
    int r_init;
    int c_init;
public:
    QuickSolver(graph G, int r, int c) {
        edge_sets = vector<boost::dynamic_bitset<>>(G.n, boost::dynamic_bitset<>(G.n));
        map_to_bitsets = {};
        map_to_nodes = {};
        for (int indx = 0; indx < G.n; ++indx) {
            for (auto n = G.G[indx]; n != NULL; n=n->suiv) {
                edge_sets[indx][n->s] = true;
            }

            map_to_nodes.insert({edge_sets[indx], indx});
            map_to_bitsets.insert({indx, edge_sets[indx]});
            std::cout << "\r" << indx;
        }
        std::cout << "\n";
        s = make_unique<Sampler>(Sampler(edge_sets, r, c));
        G_ptr = make_unique<graph>(G);
        r_init = r;
        c_init = c;
        std::cout << "initialized..\n";
    }

    void refresh() {
        std::cout << "refreshing..\n";

        edge_sets = vector<boost::dynamic_bitset<>>(map_to_bitsets.size(), boost::dynamic_bitset<>(map_to_bitsets.size()));
        map<int, boost::dynamic_bitset<>&> map_to_bitsets2 = {};
        map_to_nodes = {};
        for (auto k: map_to_bitsets) {
            int indx = k.first;
            for (auto n = G_ptr->G[indx]; n != NULL; n=n->suiv) {
                edge_sets[indx][n->s] = true;
            }

            map_to_nodes.insert({edge_sets[indx], indx});
            map_to_bitsets2.insert({indx, edge_sets[indx]});
        }
        map_to_bitsets = map_to_bitsets2;
        s = make_unique<Sampler>(Sampler(edge_sets, r_init, c_init));
        std::cout << "refreshed..\n";

    }

    vector<pair<int, int>> solve(int initial_candidates=300, int to_remove=3) {
        std::cout << "solving\n";
        while (map_to_bitsets.size() > 1) {
            inner_solve2(initial_candidates, to_remove);
        }

        std::cout << "\n";
        return solution;
    }

    void inner_solve2(int initial_candidates=300, int to_remove=3) {
        int i = 0;
        for (const auto& nb_pair : map_to_bitsets) {
            inner_solve(nb_pair.first, to_remove);
            std::cout << "\r" << i << " / " << initial_candidates;
            ++i;
            if (i > initial_candidates)
                break;
        }
        refresh();
    }

    void inner_solve(int candidate_node, int to_remove=3) {
        std::cout << "inner solve 1\n";
        if (!map_to_bitsets.contains(candidate_node))
            return;
        auto bits = map_to_bitsets.at(candidate_node);
        auto results = s->query(bits);
        int i = 0;
        std::cout << "Got " << results.size() << " results..\n";
        for (const auto& result : results) {
            if (i > to_remove)
                return;
            if (map_to_bitsets.contains(result.first)) {
                solution.emplace_back(candidate_node, result.first);
                map_to_bitsets.erase(result.first);
            }
            i++;
        }
        std::cout << "leaving inner solve 1\n";
    }

};

#endif //PACE_QUICK_SOLVER_H
