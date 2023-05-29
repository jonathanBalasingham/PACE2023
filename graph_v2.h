//
// Created by jonathan on 5/27/23.
//

#ifndef PACE_GRAPH_V2_H
#define PACE_GRAPH_V2_H

#include <boost/dynamic_bitset.hpp>
#include "graph.h"

using namespace std;


class BitsetGraph {
private:
    map<int, boost::dynamic_bitset<>> edges = {};
    map<int, boost::dynamic_bitset<>> red_edges = {};
    map<int, set<int>> neighbor_ids = {};
    boost::dynamic_bitset<> nodes;

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
        edges[a] = (edges[a] | edges[b]) & nodes;
        red_edges[a] = (edges[a] ^ edges[b]) & nodes;
        red_edges[a][a] = false;
        red_edges[a][b] = false;
        edges[a][a] = false;
        edges[a][b] = false;
        //neighbor_ids[a];
        nodes[b] = false;
    }

    int order() {
        return (int) nodes.count();
    }

    int red_degree_after_contraction(int a, int b) {
        auto sd = (edges[a] ^ edges[b]) & nodes;
        sd[a] = false;
        sd[b] = false;
        return (int) sd.count();
    }

    vector<pair<int, int>> contraction_sequence(const set<int>& subset) {
        vector<pair<int, int>> solution{};
        boost::dynamic_bitset<> node_subset(nodes.size());
        for (auto node: subset)
            node_subset[node] = true;



        return solution;
    }

    set<int> neighbors(int a) {
        set<int> n{};
        for (int i = 0; i < (int) edges[a].size(); ++i) {
            if (edges[a][i])
                n.insert(i);
        }
        return n;
    }


    vector<pair<int, int>> sym_diff_of_neighbors(int a, int limit=INT32_MAX) {
        auto result = vector<pair<int, int>>();
        int sd;

        pair<int, int> p;

        set<int> n{}; //; = neighbor_ids[a];
        for (int i = 0; i < edges[a].size(); ++i) {
            if (edges[a][i])
                n.insert(i);
        }
        if (limit == INT32_MAX) {
            for (const auto nid: n) {
                sd = (edges[a] ^ edges[nid]).count() - 2;
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
                sd = (edges[a] ^ edges[nid]).count() - 2;
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

    vector<pair<int, int>> bfs_solve() {
        int starting_node = edges.begin()->first;
        vector<pair<int, int>> solution{};
        int candidates = order() / 1000 + 1;
	    candidates *= 10;
        int limit = 10;
        std::cout << "Using " << candidates << "\n";
        inner_bfs_solve(solution, starting_node, candidates, limit);
        return solution;
    }

    void inner_bfs_solve(vector<pair<int, int>>& sol, int node, int candidates=100, int limit=INT32_MAX) {
        auto results = sym_diff_of_neighbors(node, limit);
        int nodes_contracted = 0;
        for (auto p : results) {
            if (nodes[p.first] and nodes[node]) {
                contract(node, p.first);
                nodes_contracted++;
                sol.emplace_back(node, p.first);
                if (nodes_contracted > candidates)
                    break;
            }
        }

        if (order() > 1) {
            if (not (edges[node] & nodes).any()) {
                //std::cout << "Isolated node " << node << ", moving to ";
                for (const auto& r : edges) {
                    if (nodes[r.first] and (r.second & nodes).any()) {
                        contract(r.first, node);
                        sol.emplace_back(r.first, node);
                        node = r.first;
                        break;
                    }
                }
            }

            std::cout << "\r" << "Graph size: " << order();
            inner_bfs_solve(sol, node, candidates);
        } else return;
    }

};


#endif //PACE_GRAPH_V2_H
