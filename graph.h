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

using AdjacencyDict = std::map<int, std::map<int, int>>;

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

    std::set<int> neighbors(int n) {
        if (has_node(n)) {
            auto d = adjacency_dict.at(n);
            auto kv = std::views::keys(d);
            std::set<int> v{ kv.begin(), kv.end() };
            return v;
        } else {
            // probably throw an exception
            throw std::runtime_error("Node not found in graph");
        }
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

    int get_max_red_degree() {
        int mrd = 0;
        for (auto v : vertices()) {
            auto rd = get_red_degree(v);
            if (rd > mrd)
                mrd = rd;
        }
        return mrd;
    }

    void contract(int a, int b) {
        // a is kept, b is thrown out
        std::vector<int> set_diff;
        auto na = neighbors(a);
        auto nb = neighbors(b);
        std::set_symmetric_difference(na.begin(), na.end(),
                                      nb.begin(), nb.end(), std::back_inserter(set_diff));
        //auto removed_vertex = adjacency_dict.at(b);
        adjacency_dict.erase(b);
        for (auto vertex : set_diff) {
            adjacency_dict[a][vertex] = 2;
        }

        for (auto vertex : vertices()) {
            if (adjacency_dict.at(vertex).find(b) != adjacency_dict.at(vertex).end()) {
                adjacency_dict.at(vertex).erase(b);
            }
        }

        adjacency_dict.at(a).erase(a);
        nnodes--;
        max_red_degree = std::max(max_red_degree, get_max_red_degree());
    }

    int red_degree_after_contraction(int a, int b) {
        std::vector<int> set_diff;
        auto na = neighbors(a);
        auto nb = neighbors(b);
        std::set_symmetric_difference(na.begin(), na.end(),
                                      nb.begin(), nb.end(), std::back_inserter(set_diff));
        int rd = set_diff.size();
        return std::max(max_red_degree, rd);
    }

    int simulate_contractions() {

    }

};

#endif //PACE_GRAPH_H
