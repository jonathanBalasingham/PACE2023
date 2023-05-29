//
// Created by jonathan on 5/7/23.
//

#ifndef PACE_GRAPH_H
#define PACE_GRAPH_H

#include <vector>
#include <map>
#include <ranges>
#include <set>
#include <algorithm>
#include "dm_english.h"

using AdjacencyDict = std::map<int, std::map<int, int>>;

struct cmp {
    bool operator()(int i, const std::pair<int, int>& p) const
    {
        return i < p.first;
    }

    bool operator()(const std::pair<int, int>& p, int i) const
    {
        return p.first < i;
    }

    bool operator()(int k, int i) const
    {
        return k < i;
    }

    bool operator()(const std::pair<int, int>& p, const std::pair<int, int>& p2) const
    {
        return p.first < p2.first;
    }

};

bool cmp2(const std::pair<int, int>& p, const std::pair<int, int>& p2) {
    return p.first < p2.first;
}

class Graph {
private:
    int nedges;
    int nnodes;
    int max_red_degree;
    AdjacencyDict adjacency_dict;


public:
    Graph(int size=0) {
        nedges = 0;
        nnodes = size;
        max_red_degree = 0;
        adjacency_dict = {};
        for (int i = 0; i < size; ++i) {
            adjacency_dict.insert({i, std::map<int, int>()});
        }
    }

    Graph(graph g) {
        nnodes = g.n;
        max_red_degree = 0;
        adjacency_dict = {};
        nedges = 0;

        for (int i = 0; i < nnodes; ++i) {
            adjacency_dict.insert({i, std::map<int, int>()});
        }

        for (int i = 0; i < g.n; ++i) {
            for (auto n = g.G[i]; n != NULL; n=n->suiv) {
                adjacency_dict[i][n->s] = 1;
                adjacency_dict[n->s][i] = 1;
                //add_edge(i, n->s);
            }
        }
    }

    bool has_node(int n) {
        return adjacency_dict.find(n) != adjacency_dict.end();
    }

    void add_edge(int n1, int n2) {
        if (has_node(n1) && has_node(n2)) {
            auto res = adjacency_dict.at(n1);
            if (res.find(n2) == res.end()) {
                res.insert({n2, 1});
                adjacency_dict.at(n2).insert({n1, 1});
            } else {
                // edge already exists
            }
        } else {
            // node doesnt exist
        }
    }

    void add_edges(const std::vector<std::pair<int,int>>& es) {
        for (auto e : es) {
            add_edge(e.first, e.second);
        }
    }

    bool has_edge(int n1, int n2) {
        if (has_node(n1) and has_node(n2)) {
            return adjacency_dict.at(n1).find(n2) != adjacency_dict.at(n1).end()
                and adjacency_dict.at(n2).find(n1) != adjacency_dict.at(n2).end();
        } else {
            // node doesnt exist
            return false;
        }
    }

    int order(){ return nnodes; }
    int size(){ return nedges; }

    std::vector<int> vertices() {
        auto kv = std::views::keys(adjacency_dict);
        std::vector<int> v{ kv.begin(), kv.end() };
        return v;
    }

    std::set<int> vertices_as_set() {
        auto kv = std::views::keys(adjacency_dict);
        std::set<int> v{ kv.begin(), kv.end() };
        return v;
    }

    int first() {
        return adjacency_dict.begin()->first;
    }

    std::set<int> neighbors(int n) {
        if (has_node(n)) {
            auto d = adjacency_dict.at(n);
            auto kv = std::views::keys(d);
            std::set<int> v{ kv.begin(), kv.end() };
            return v;
        } else {
            // probably throw an exception
            std::cout << n;
            throw std::runtime_error("Node not found in graph" );
        }
    }

    std::shared_ptr<std::map<int, int>> neighbors_at(int n) {
        return std::make_shared<std::map<int, int>>(adjacency_dict.at(n));
    }

    void set_max_red_degree(int i) {
        max_red_degree = std::max(i, max_red_degree);
    }

    void add_neighbors(int n, const std::set<int>& vs) {
        for (auto v : vs) {
            adjacency_dict[n][v] = 1;
            nedges++;
        }
    }

    void add_node(int n) {
        if (!has_node(n)) {
            adjacency_dict[n] = std::map<int, int>();
            nnodes++;
        }
    }

    Graph subgraph(const vector<int>& v) {
        auto g = Graph();
        std::set<int> v_as_set(v.begin(), v.end());

        for (auto vertex: v) {
            g.add_node(vertex);
            auto vertex_neighbors = neighbors(vertex);
            auto to_keep = std::set<int>();
            std::set_intersection(vertex_neighbors.begin(), vertex_neighbors.end(),
                                  v_as_set.begin(), v_as_set.end(), inserter(to_keep, to_keep.begin()));
            g.add_neighbors(vertex, to_keep);
        }

        return g;
    }

    int get_red_degree(int n) {
        int degree = 0;
        auto ns = neighbors(n);
        for (auto node : ns) {
            if (adjacency_dict.at(n).at(node) == 2)
                degree++;
        }
        return degree;
    }

    int get_max_red_degree(){ return max_red_degree; }

    void contract(int a, int b) {
        // a is kept, b is thrown out
        const map<int, int>* na = &adjacency_dict.at(a);
        const map<int, int>* nb = &adjacency_dict.at(b);
        auto set_diff = sym_diff(*na, *nb);

        set_diff.erase(a);
        set_diff.erase(b);
        int new_red_degree = set_diff.size();
        adjacency_dict.erase(b);
        for (auto vertex : set_diff) {
            adjacency_dict[a][vertex] = 2;
        }

        for (auto vertex : vertices()) {
            if (adjacency_dict.at(vertex).find(b) != adjacency_dict.at(vertex).end()) {
                adjacency_dict.at(vertex).erase(b);
            }
        }

        nnodes--;
        max_red_degree = std::max(max_red_degree, new_red_degree);
    }

    int red_degree_after_contraction(int a, int b) {
        const map<int, int>* na = &adjacency_dict.at(a);
        const map<int, int>* nb = &adjacency_dict.at(b);
        auto set_diff = sym_diff(*na, *nb);

        set_diff.erase(a);
        set_diff.erase(b);
        return std::max(max_red_degree, (int) set_diff.size());
    }

    set<int> sym_diff(int a, int b) {
        std::set<int> set_diff;
        auto na = neighbors(a);
        auto nb = neighbors(b);
        std::set_symmetric_difference(na.begin(), na.end(),
                                      nb.begin(), nb.end(), std::inserter(set_diff, set_diff.begin()));
        set_diff.erase(a);
        set_diff.erase(b);
        return set_diff;
    }

    set<int> sym_diff(const set<int>& na, const set<int>& nb) {
        std::set<int> set_diff;
        std::set_symmetric_difference(na.begin(), na.end(),
                                      nb.begin(), nb.end(), std::inserter(set_diff, set_diff.begin()));
        return set_diff;
    }

    set<int> sym_diff(const set<int>& na, const map<int, int>& nb) {
        std::set<int> set_diff;
        for (auto n: na) {
            if (!nb.contains(n))
                set_diff.insert(n);
        }

        for (auto n: nb) {
            if (!na.contains(n.first))
                set_diff.insert(n.first);
        }

        return set_diff;
    }

    set<int> sym_diff(const map<int, int>& na, const map<int, int>& nb) {
        std::set<int> set_diff;
        for (auto n: na) {
            if (!nb.contains(n.first))
                set_diff.insert(n.first);
        }

        for (auto n: nb) {
            if (!na.contains(n.first))
                set_diff.insert(n.first);
        }

        return set_diff;
    }

    float simulate_contractions(const vector<pair<int, int>>& cs) {
        auto modified_vertices = map<int, map<int, int>>(); // this will hold the new neighbors
        map<int, int>* n1;
        map<int, int>* n2;
        vector<int> red_degrees{};
        float total_degree = 0;

        for (auto c: cs) {
            if (!modified_vertices.contains(c.first)) {
                modified_vertices.insert({c.first, adjacency_dict.at(c.first)});
            }
        }

        set<int> sd;
        for (auto c: cs) {
            //std::cout << "Working on " << c.first << "," << c.second << "\n";
            n1 = &modified_vertices.at(c.first);
            if (modified_vertices.contains(c.second))
                n2 = &modified_vertices.at(c.second);
            else
                n2 = &adjacency_dict.at(c.second);

            // by modifying n1, we've modified all n2's neighbors
            // they need to be added to modified_vertices
            sd = sym_diff(*n1, *n2);
            sd.erase(c.first);
            sd.erase(c.second);
            red_degrees.push_back((int) sd.size());
            total_degree += (float) sd.size();
            //std::cout << "Adding the neighbors of " << c.second << " to " << c.first << ": ";
            for (auto edge : sd) {
                std::cout << edge << " ";
                n1->insert({edge, 2}); // transfer the red edges to the kept node
            }
            //std::cout << "\n";
            auto n2_copy = *n2;
            for (auto n : n2_copy) {
                if (modified_vertices.contains(n.first)) {
                    modified_vertices.at(n.first).erase(c.second);
                    modified_vertices.at(n.first).insert({c.first, 2});
                    continue;
                }
                auto n_neighbors = adjacency_dict.at(n.first);
                n_neighbors.erase(c.second);
                n_neighbors.insert({c.first, 2}); // can i just swap keys
                modified_vertices.insert({n.first, n_neighbors});
            }
        }
        return total_degree / (float) cs.size();
    }


};

#endif //PACE_GRAPH_H
