//
// Created by jonathan on 5/8/23.
//

#ifndef PACE_SOLVER_H
#define PACE_SOLVER_H

using namespace std;

#include <vector>
#include <iostream>
#include <boost/dynamic_bitset.hpp>
//#include "graph.h"
#include <chrono>
#include <numeric>
#include "mcts.h"

using namespace std::chrono;


class Solver {
private:
    shared_ptr<Graph> G;
    vector<pair<int, int>> solution;
    int solve_module(const vector<int>& node_subset) {
        //cout << "solving module of size: " << node_subset.size() << "\n";

        if (node_subset.size() == 2) {
            solution.emplace_back(node_subset[0], node_subset[1]);
            G->contract(node_subset[0], node_subset[1]);
            return solution.back().first;
        }

        auto m = MonteCarloTreeSearch();
        auto sub_g = G->subgraph(node_subset);
        auto cs = m.solve(make_shared<Graph>(sub_g), node_subset);
        solution.insert(solution.end(), cs.begin(), cs.end());

        for (auto contraction : cs)
            G->contract(contraction.first, contraction.second);

        return cs.back().first;
    }

    void traverse(node *N) {
        fils *F;
        if (N->fils == NULL) {
            cout << "hit a dead end.. moving up\n";
        } else {
            bool mod = true;
            std::vector<int> leaves;
            for (F = N->fils; F != NULL; F = F->suiv) {
                if (F->pointe->type != LEAF) {
                    mod = false;
                    traverse(F->pointe);
                } else
                    leaves.push_back(F->pointe->nom);
            }

            if (mod) {
                // we need to process this portion of the graph
                int remaining_node =  solve_module(leaves);
                // generate contraction sequence, last node remaining
                // is the new leaf
                N->type = LEAF;
                N->nom = remaining_node; // this is just an example
                // then remove this branch
                N->fils = NULL;
            }
        }

    }
public:
    vector<pair<int, int>> solve(graph g, bool verbose=true) {
        if (verbose)
            cout << "running modular decomposition..";
        auto start = high_resolution_clock::now();
        auto R = decomposition_modulaire(g);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<seconds>(stop - start);

        if (verbose) {
            cout << "[DONE] - " << duration.count() << " seconds \n";
            cout << "Making c++ graph..";
        }
        start = high_resolution_clock::now();
        G = make_shared<Graph>(Graph(g));
        stop = high_resolution_clock::now();
        if (verbose) {
            cout << "[DONE] - " << duration_cast<seconds>(stop - start).count() << " seconds\n";
        }
        solution = {};

        int level = 0;
        while (R->type != LEAF) {
            cout << "solving level " << level << "..\n";
            traverse(R);
            level++;
        }
        std::cout << "Twinwidth: " << G->get_max_red_degree() << "\n";
        return solution;
    }
};

#endif //PACE_SOLVER_H
