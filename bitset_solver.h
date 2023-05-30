//
// Created by jonathan on 5/29/23.
//

#ifndef PACE_BITSET_SOLVER_H
#define PACE_BITSET_SOLVER_H

using namespace std;

#include <vector>
#include <iostream>
#include <boost/dynamic_bitset.hpp>
//#include "graph.h"
#include <chrono>
#include <numeric>
#include "graph_v2.h"
#include "graph_v3.h"

using namespace std::chrono;


class Solver {
private:
    shared_ptr<BitsetGraph> G_b;
    shared_ptr<RoaringGraph> G_r;
    vector<pair<int, int>> solution;
    std::string method;

    int solve_module(const set<int>& node_subset) {
        if (method == "db") {
            boost::dynamic_bitset<> node_subset_bitset(G_b->original_size());
            for (auto n : node_subset)
                node_subset_bitset[n] = true;

            auto cs = G_b->bfs_solve(node_subset_bitset);

            solution.insert(solution.end(), cs.begin(), cs.end());
            return cs.back().first;
        } else {
            Roaring node_subset_bitset;
            for (auto n : node_subset)
                node_subset_bitset.add(n);

            auto cs = G_r->bfs_solve(node_subset_bitset);

            solution.insert(solution.end(), cs.begin(), cs.end());
            return cs.back().first;

        }
    }

    void traverse(node *N) {
        fils *F;
        if (N->fils == NULL) {
            cout << "hit a dead end.. moving up\n";
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
    shared_ptr<RoaringGraph> get_graph(){ return G_r; }
    vector<pair<int, int>> get_solution() { return solution; }

    vector<pair<int, int>> solve(graph g, bool verbose=true, const std::string& m="roaring") {
        if (verbose)
            cout << "running modular decomposition..";
        auto start = high_resolution_clock::now();
        auto R = decomposition_modulaire(g);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<seconds>(stop - start);
        method = m;
        if (verbose) {
            cout << "[DONE] - " << duration.count() << " seconds \n";
            cout << "Making c++ graph..";
        }
        start = high_resolution_clock::now();
        if (method == "roaring") {
            G_r = make_shared<RoaringGraph>(RoaringGraph(g));
        } else {
            G_b = make_shared<BitsetGraph>(BitsetGraph(g));
        }

        stop = high_resolution_clock::now();
        if (verbose) {
            cout << "[DONE] - " << duration_cast<seconds>(stop - start).count() << " seconds\n";
        }
        solution = {};

        int level = 0;
        while (R->type != LEAF) {
            start = high_resolution_clock::now();
            cout << "solving level " << level << "..";
            traverse(R);
            level++;
            stop = high_resolution_clock::now();
            if (verbose) {
                cout << "[DONE] - " << duration_cast<seconds>(stop - start).count() << " seconds\n";
            }
        }

        std::cout << "Size of solution: " << solution.size() << "\n";
        if (method == "roaring")
            std::cout << "Twinwidth: " << G_r->get_max_red_degree() << "\n";
        else
            std::cout << "Twinwidth: " << G_b->get_max_red_degree() << "\n";
        return solution;
    }
};

#endif //PACE_BITSET_SOLVER_H
