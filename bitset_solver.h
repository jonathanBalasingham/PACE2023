//
// Created by jonathan on 5/29/23.
//

#ifndef PACE_BITSET_SOLVER_H
#define PACE_BITSET_SOLVER_H

using namespace std;

#include <vector>
#include <iostream>
#include <chrono>
#include <numeric>
#include "graph_v3.h"
#include <memory>
#include <set>

using namespace std::chrono;
using namespace std;

class Solver {
private:
    shared_ptr<RoaringGraph> G_r;
    vector<pair<int, int>> solution;
    std::string method;

    int solve_module(const set<int>& node_subset) {
        Roaring node_subset_bitset;
        for (auto n : node_subset)
            node_subset_bitset.add(n);

        auto cs = G_r->bfs_solve(node_subset_bitset);

        solution.insert(solution.end(), cs.begin(), cs.end());
        return cs.back().first;
    }

    void traverse(node *N) {
        fils *F;
        if (N->fils == NULL) {
            //cout << "hit a dead end.. moving up\n";
        } else {
            bool mod = true;
            std::set<int> leaves;
            for (F = N->fils; F != NULL; F = F->suiv) {
                if (F->pointe->type != LEAF) {
                    mod = false;
                    traverse(F->pointe);
                } else
                    leaves.insert(F->pointe->nom);
            }

            if (mod) {
                int remaining_node =  solve_module(leaves);
                N->type = LEAF;
                N->nom = remaining_node;
                N->fils = NULL;
            }
        }

    }
public:
    shared_ptr<RoaringGraph> get_graph(){ return G_r; }
    vector<pair<int, int>> get_solution() { return solution; }

    vector<pair<int, int>> solve(graph g, bool verbose=true, const std::string& m="roaring", bool skip_md=false) {
        if (skip_md) {
	    method = m;
            set<int> node_subset = {};
            for (int i = 0; i < g.n; ++i) {
                node_subset.insert(i);
            }
	    solution = {};
	    G_r = make_shared<RoaringGraph>(RoaringGraph(g));
            solve_module(node_subset);

        } else {
            auto R = decomposition_modulaire(g);

            method = m;

            G_r = make_shared<RoaringGraph>(RoaringGraph(g));
            solution = {};

            int level = 0;
            while (R->type != LEAF) {
                traverse(R);
                level++;
            }
        }
        return solution;
    }
};

#endif //PACE_BITSET_SOLVER_H
