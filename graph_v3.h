//
// Created by jonathan on 5/29/23.
//

#ifndef PACE_GRAPH_V3_H
#define PACE_GRAPH_V3_H


using namespace std;

#include <iostream>
#include "roaring.hh"
#include "graph.h"

class RoaringGraph {
private:
    map<int, Roaring> edges = {};
    map<int, Roaring> red_edges = {};
    Roaring nodes;
    int mrd = 0;
    vector<pair<int, int>> solution;

    int find_best_node(int cn, Roaring& node_subset, int limit=1000) {
        int sd = -1;
        int sd2;
        int best_node = 0;
        int checked_nodes = 0;
        Roaring nodes_to_check = nodes & node_subset;
        for (auto ind : nodes_to_check) {
            Roaring& i = edges[ind];
            if (ind == cn)
                continue;

            if (sd == -1) {
                sd = (edges[cn] ^ i).cardinality();
                best_node = ind;
            } else {
                sd2 = (edges[cn] ^ i).cardinality();
                if (sd2 < sd) {
                    sd = (edges[cn] ^ i).cardinality();
                    best_node = ind;
                }
            }
            checked_nodes++;
            if (checked_nodes >= limit)
                return best_node;
        }

        return best_node;
    }

public:
    RoaringGraph(graph G) {
        int size = G.n;
        nodes = Roaring();
        for (int i = 0; i < size; ++i) {
            edges[i] = Roaring();
            red_edges[i] = Roaring();
            nodes.add(i);
        }

        for (int i = 0; i < size; ++i) {
            for (auto n = G.G[i]; n != NULL; n=n->suiv) {
                edges[i].add(n->s);
            }
        }
    }

    Roaring get_nodes() { return nodes; }
    vector<pair<int, int>> get_solution() { return solution; }

    void contract(int a, int b) {
        red_edges[a] = (edges[a] ^ edges[b]) & nodes;
        edges[a] |= edges[b];

        red_edges[a].remove(a);
        red_edges[a].remove(b);
        edges[a].remove(a);
        edges[a].remove(b);
        nodes.remove(b);
        mrd = max(mrd, (int) red_edges[a].cardinality());
    }

    int get_max_red_degree() { return mrd; }

    int order() {
        return (int) nodes.cardinality();
    }


    vector<pair<int, int>> sym_diff_of_neighbors(int a, Roaring& node_subset, int limit=INT32_MAX) {
        auto result = vector<pair<int, int>>();
        int sd;
        pair<int, int> p;
        const Roaring& n = edges[a] & node_subset;

        for (const auto nid: n) {
            sd = ((n ^ edges[nid])).cardinality() - 2;
            p = {nid, sd};
            if (result.empty()) {
                result.push_back(p);
                continue;
            }
            auto lb = lower_bound(result.begin(), result.end(), p,
                                  [&](pair<int, int> a, pair<int, int> b) { return a.second < b.second; });
            result.insert(lb, p);
            if (result.size() >= limit) {
                return result;
            }
        }
        return result;
    }

    int original_size() { return nodes.cardinality(); }

    vector<pair<int, int>> bfs_solve(Roaring& node_subset) {
        int starting_node;
        solution = {};
        // Find a node in the subset to start at

        for (auto i : node_subset) {
            starting_node = (int) i;
            break;
        }
        // number of neighbors to consider
        int limit = 100;
        // number of neighbors to collapse
        int candidates = (int) .5 * limit;
        if (order() < 10000) {
            limit = INT32_MAX;
            candidates = (int) floor(.001 * order());
        }
        candidates = (int) floor(.001 * node_subset.cardinality()) + 1;
        limit  = candidates * 10 + 10;

        //std::cout << "Candidates: " << candidates << "\n";
        //std::cout << "Limit: " << limit << "\n";
        //inner_bfs_solve(starting_node, node_subset, candidates, limit);
        inner_bfs_solve2(starting_node, node_subset);
        return solution;
    }

    void inner_bfs_solve(int node, Roaring& node_subset, int candidates=100, int limit=INT32_MAX) {
        do {
            auto results = sym_diff_of_neighbors(node, node_subset, limit);

            int nodes_contracted = 0;
            // For each neighbor,
            // compute the sym diff and place it in the result in order
            for (auto p : results) {
                contract(node, p.first);
                node_subset.remove(p.first);
                nodes_contracted++;
                solution.emplace_back(node, p.first);
                if (nodes_contracted > candidates)
                    break;
            }

            if (node_subset.cardinality() > 1) {
                if ((edges[node] & node_subset).cardinality() == 0) {
                    int i = find_best_node(node, node_subset, limit);
                    contract(i, node);
                    node_subset.remove(node);
                    solution.emplace_back(i, node);
                    node = i;
                }
                //std::cout << "\r" << "Graph size: " << order();
            } else return;
        } while (node_subset.cardinality() > 1);
    }


    void inner_bfs_solve2(int node, Roaring& node_subset, int limit=INT32_MAX) {
        Roaring cache = {};
        auto results = vector<pair<int, int>>();
        pair<int, int> p;
        int sd;
        const Roaring& n = edges[node] & node_subset;

        for (const auto nid: n) {
            sd = (n ^ edges[nid]).cardinality() - 2;
            p = {nid, sd};
            if (results.empty()) {
                results.push_back(p);
                continue;
            }
            auto lb = lower_bound(results.begin(), results.end(), p,
                                  [&](pair<int, int> a, pair<int, int> b) { return a.second < b.second; });
            results.insert(lb, p);
            cache.add(nid);
        }

        do {
            if (results.empty()) {
                int i = find_best_node(node, node_subset, limit);
                contract(i, node);
                node_subset.remove(node);
                solution.emplace_back(i, node);
                node = i;
                cache = {};
                auto neighbors = edges[node] & node_subset;
                for (auto nid : neighbors) {
                    sd = ((n ^ edges[nid])).cardinality() - 2;
                    p = {nid, sd};
                    if (results.empty()) {
                        results.push_back(p);
                        continue;
                    }
                    auto lb = lower_bound(results.begin(), results.end(), p,
                                          [&](pair<int, int> a, pair<int, int> b) { return a.second < b.second; });
                    results.insert(lb, p);
                    cache.add(nid);
                }

            } else {
                p = results[0];
                results.erase(results.begin());
                if (not node_subset.contains(p.first)){
                    continue;
                }
                contract(node, p.first);
                node_subset.remove(p.first);
                solution.emplace_back(node, p.first);
                auto neighbors = (edges[node] - cache) & node_subset;

                for (auto nid : neighbors) {
                    sd = (n ^ edges[nid]).cardinality() - 2;
                    p = {nid, sd};
                    auto lb = lower_bound(results.begin(), results.end(), p,
                                          [&](pair<int, int> a, pair<int, int> b) { return a.second < b.second; });
                    results.insert(lb, p);
                    cache.add(nid);
                }
            }
	    if (order() % 10 == 0)
            	std::cout << "\r" << "Graph size: " << order();
        } while (node_subset.cardinality() > 1);
    }

};

#endif //PACE_GRAPH_V3_H
