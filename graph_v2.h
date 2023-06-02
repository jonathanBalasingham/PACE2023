//
// Created by jonathan on 5/27/23.
//

#ifndef PACE_GRAPH_V2_H
#define PACE_GRAPH_V2_H

#include <boost/dynamic_bitset.hpp>
#include "roaring.hh"
#include "dm_english.h"
#include <set>
//#include "roaring.c"

using namespace std;


class BitsetGraph {
private:
    map<int, boost::dynamic_bitset<>> edges = {};
    map<int, boost::dynamic_bitset<>> red_edges = {};
    map<int, set<int>> neighbor_ids = {};
    boost::dynamic_bitset<> nodes;
    int mrd = 0;

    int find_best_node(int cn, const boost::dynamic_bitset<>& ns) {
        int best_node = -1;
        unsigned int sd;
        unsigned int sd2;
        for (int i = 0; i < ns.size(); ++i) {
            if (not ns[i])
                continue;

            if (i == cn)
                continue;

            if (best_node == -1) {
                sd = ((edges[cn] ^ edges[i]) & ns).count();
                best_node = i;
            } else {
                sd2 = ((edges[cn] ^ edges[i]) & ns).count();
                if (sd2 < sd) {
                    best_node = i;
                    sd = sd2;
                }
            }
        }
        return best_node;
    }

public:
    BitsetGraph(int size) {
        for (int i = 0; i < size; ++i) {
            edges[i] = boost::dynamic_bitset<>(size);
            red_edges[i] = boost::dynamic_bitset<>(size);
        }
        nodes = boost::dynamic_bitset<>(size).set();
    }

    BitsetGraph(graph G) {
        int size = G.n;
        for (int i = 0; i < size; ++i) {
            edges[i] = boost::dynamic_bitset<>(size);
            red_edges[i] = boost::dynamic_bitset<>(size);
            neighbor_ids[i] = {};
        }

        for (int i = 0; i < size; ++i) {
            for (auto n = G.G[i]; n != NULL; n=n->suiv) {
                edges[i][n->s] = true;
                edges[n->s][i] = true;
                neighbor_ids[i].insert(n->s);
            }
        }
        nodes = boost::dynamic_bitset<>(size).set();
    }

    void add_edge(int a, int b, bool red_edge=false) {
        edges[a][b] = true;
        edges[b][a] = true;
        if (red_edge) {
            red_edges[a][b] = true;
            red_edges[b][a] = true;
        }
    }

    int sym_diff_size(int a, int b) {
        return ((edges[a] ^ edges[b]) & nodes).count();
    }

    boost::dynamic_bitset<> sym_diff(int a, int b) {
        return (edges[a] ^ edges[b]) & nodes;
    }

    bool has_node(int a) { return nodes[a]; }

    int compute_max_red_degree() {
        int mrd = 0;
        for (unsigned int i = 0; i < nodes.size(); ++i) {
            if (nodes[i])
                mrd = max((int) red_edges[i].count(), mrd);
        }
        return mrd;
    }

    void contract(int a, int b) {
        red_edges[a] = (edges[a] ^ edges[b]) & nodes;
        edges[a] = (edges[a] | edges[b]) & nodes;
        red_edges[a][a] = false;
        red_edges[a][b] = false;
        edges[a][a] = false;
        edges[a][b] = false;
        //neighbor_ids[a];
        nodes[b] = false;
        mrd = max(mrd, (int) red_edges[a].count());
    }

    int get_max_red_degree() { return mrd; }

    int order() {
        return (int) nodes.count();
    }

    int red_degree_after_contraction(int a, int b) {
        auto sd = (edges[a] ^ edges[b]) & nodes;
        sd[a] = false;
        sd[b] = false;
        return (int) sd.count();
    }


    set<int> neighbors(int a) {
        set<int> n{};
        for (int i = 0; i < (int) edges[a].size(); ++i) {
            if (edges[a][i])
                n.insert(i);
        }
        return n;
    }


    vector<pair<int, int>> sym_diff_of_neighbors(int a, const boost::dynamic_bitset<>& node_mask, int limit=INT32_MAX) {
        auto result = vector<pair<int, int>>();
        int sd;

        pair<int, int> p;

        set<int> n{}; //; = neighbor_ids[a];
        for (int i = 0; i < edges[a].size(); ++i) {
            if (edges[a][i] and node_mask[i]) // Collect neighbors for node (a) that are also in the subset
                n.insert(i);
        }
        if (limit == INT32_MAX) {
            for (const auto nid: n) {
                sd = ((edges[a] ^ edges[nid]) & node_mask).count() - 2;
                p = {nid, sd};
                if (result.empty()) {
                    result.push_back(p);
                    continue;
                }
                auto lb = lower_bound(result.begin(), result.end(), p,
                                      [&](pair<int, int> a, pair<int, int> b) { return a.second < b.second; });
                result.insert(lb, p);
            }
        } else {
            int nodes_examined = 0;
            for (const auto nid: n) {
                sd = ((edges[a] ^ edges[nid]) & node_mask).count() - 2;
                p = {nid, sd};
                if (result.empty()) {
                    result.push_back(p);
                    continue;
                }
                auto lb = lower_bound(result.begin(), result.end(), p,
                                      [&](pair<int, int> a, pair<int, int> b) { return a.second < b.second; });
                result.insert(lb, p);
                nodes_examined++;
                if (nodes_examined > limit) {
                    break;
                }
            }
        }
        return result;
    }

    int original_size() { return nodes.size(); }

    vector<pair<int, int>> bfs_solve(boost::dynamic_bitset<>& node_subset) {
        int starting_node;
        vector<pair<int, int>> solution{};
        // Find a node in the subset to start at
        for(int i = 0; i < (int) node_subset.size(); ++i) {
            if (node_subset[i])
                starting_node = i;
        }
        // number of neighbors to collapse
        int candidates = (order() / 1000 + 1);
        candidates = 50;
        if (node_subset.count() < 1000) {
            candidates = node_subset.count();
        }
        // number of neighbors to consider
        int limit = 100;

        inner_bfs_solve(solution, starting_node, node_subset, candidates, limit);
        return solution;
    }

    void inner_bfs_solve(vector<pair<int, int>>& sol, int node, boost::dynamic_bitset<>& node_subset, int candidates=100, int limit=INT32_MAX) {
        auto results = sym_diff_of_neighbors(node, node_subset, limit);
        int nodes_contracted = 0;
        // For each neighbor,
        // compute the sym diff and place it in the result in order
        for (auto p : results) {
            if (nodes[p.first] and nodes[node] and node_subset[p.first] and node_subset[node]) { // probably unecessary
                contract(node, p.first);
                node_subset[p.first] = false;
                nodes_contracted++;
                sol.emplace_back(node, p.first);
                if (nodes_contracted > candidates)
                    break;
            }
        }

        if (node_subset.count() > 1) {
            if (not (edges[node] & nodes & node_subset).any()) {
                /*
                // iterate through the subset and find the next node
                for (int i = 0; i < node_subset.size(); ++i) {
                    // we just find the next node in the subset, but this is not selected in any strategic manner
                    // we could find the node in the subset with the lowest sym diff
                    if (node_subset[i] and i != node) { // the node is in the subset
                        contract(i, node);
                        node_subset[node] = false;
                        sol.emplace_back(i, node);
                        node = i;
                        break;
                    }
                }
                 */
                int i = find_best_node(node, node_subset);
                contract(i, node);
                node_subset[node] = false;
                sol.emplace_back(i, node);
                node = i;
            }

            std::cout << "\r" << "Graph size: " << order();
            inner_bfs_solve(sol, node, node_subset, candidates, limit);
        } else return;
    }

};


#endif //PACE_GRAPH_V2_H
