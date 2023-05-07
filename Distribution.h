//
// Created by jonathan on 4/19/23.
//

#ifndef PACE_DISTRIBUTION_H
#define PACE_DISTRIBUTION_H


#include <utility>
#include <map>
#include <vector>
#include <valarray>
#include <set>
#include "dm_english.h"
#include <boost/dynamic_bitset.hpp>
#include <random>

using namespace std;

class Distribution {
private:
    graph G;
    map<pair<int, int>, int> symmetric_differences;

public:
    Distribution(graph G) {
        G = G;
        for (int i = 0; i < G.n; ++i) {
            for (auto n = G.G[i]; n != NULL; n=n->suiv) {

            }
        }
    }
    graph getGraph(){ return G; }
    pair<int, int> sample(int sample_size=1) {

        return {};
    }

};

vector<vector<int>> sym_diff(graph G) {
    auto edge_sets = vector<valarray<bool>>();
    for (int i = 0; i < G.n; ++i) {
        valarray<bool> sym_diff_for_i(G.n, false);
        for (auto n = G.G[i]; n != NULL; n=n->suiv) {
            sym_diff_for_i[n->s] = true;
        }
        edge_sets.push_back(sym_diff_for_i);
    }
    vector<vector<int>> sd(G.n, vector<int>(G.n, 0));
    for (int i; i < G.n; ++i) {
        for (int j = 0; j < G.n; ++j) {

        }
    }

    return sd;
}

vector<vector<int>> bf_sym_diff(graph G) {
    auto edge_sets = vector<set<int>>();
    for (int i = 0; i < G.n; ++i) {
        set<int> s = set<int>();
        for (auto n = G.G[i]; n != NULL; n=n->suiv) {
            s.insert(n->s);
        }
        edge_sets.push_back(s);
    }
    vector<vector<int>> sd(G.n, vector<int>(G.n, 0));
    for (int i = 0; i < G.n - 1; ++i) {
        for (int j = i + 1; j < G.n; ++j) {
            std::vector<int> v_symDifference;
            std::set_symmetric_difference(edge_sets[i].begin(), edge_sets[i].end(), edge_sets[j].begin(), edge_sets[j].end(),
                                          std::back_inserter(v_symDifference));
            sd[i][j] = v_symDifference.size();
        }
    }

    return sd;
}

int hamming(const boost::dynamic_bitset<>& a, const boost::dynamic_bitset<>& b){
    return (a ^ b).count();
}

vector<vector<int>> bf_sym_diff_bs(graph G) {
    auto edge_sets = vector<boost::dynamic_bitset<>>(G.n, boost::dynamic_bitset<>(G.n));
    for (int i = 0; i < G.n; ++i) {
        for (auto n = G.G[i]; n != NULL; n=n->suiv) {
            edge_sets[i][n->s] = true;
        }
    }
    vector<vector<int>> sd(G.n, vector<int>(G.n, 0));
    for (int i = 0; i < G.n - 1; ++i) {
        for (int j = i + 1; j < G.n; ++j) {
            sd[i][j] = hamming(edge_sets[i], edge_sets[j]);
        }
    }

    return sd;
}


class Sampler {
private:
    int n;
    int d;
    int r;
    int c;
    float p1;
    float p2;
    int k;
    int L;
    vector<boost::dynamic_bitset<>> bm;
    vector<boost::dynamic_bitset<>> S;
    mt19937 gen;
    uniform_int_distribution<> u_dist;

    boost::dynamic_bitset<> kOnes() {
        auto mask = boost::dynamic_bitset<>(d);
        int sample;
        int bits_set = 0;
        while (bits_set < k) {
            sample = u_dist(gen);
            if (!mask[sample]) {
                mask[sample] = true;
                bits_set++;
            }
        }
        return mask;
    }
    vector<boost::dynamic_bitset<>> get_masks() {
        vector<boost::dynamic_bitset<>> bitmasks(L, kOnes());
        return bitmasks;
    }

    string get_bucket(boost::dynamic_bitset<> bitmask, boost::dynamic_bitset<> x) {
        boost::dynamic_bitset<> bucket {bitmask.count(), 0};
        int j = 0;
        for (int i = 0; i < bitmask.size(); ++i) {
            if (bitmask[i]) {
                bucket[j] = x[i];
                j++;
            }
        }
        string buffer;
        to_string(bucket, buffer);
        return buffer;
    }

     void preprocess() {
        auto i_map = std::map<string, vector<boost::dynamic_bitset<>>>();
        inv_map = make_unique<map<string, vector<boost::dynamic_bitset<>>>>(i_map);
        for (const auto& x: S) {
            auto used_bucket = std::set<string>();
            for (int i = 0; i < L; ++i) {
                auto g_ix = get_bucket(bm[i], x);
                if (used_bucket.find(g_ix) != used_bucket.end())
                    continue;
                used_bucket.insert(g_ix);
                if (inv_map->find(g_ix) == inv_map->end()) {
                    inv_map->insert({g_ix, vector<boost::dynamic_bitset<>>()});
                }
                inv_map->at(g_ix).push_back(x);
            }
        }
    }


public:
    unique_ptr<map<string, vector<boost::dynamic_bitset<>>>> inv_map;


    Sampler(const vector<boost::dynamic_bitset<>>& data, int r, int c) : n(data.size()), d(data[0].size()), r(r), c(c), S(data) {
        cout << "d is " << d << "\n";
        cout << "r is " << r << "\n";
        p1 = 1 - float(r)/float(d);
        p2 = 1 - float(c)*float(r)/float(d);
        k = (int) floor(log(n) / log(1/p2));
        L = (int) floor(5/p1);
        cout << "p1 is " << p1 << "\n";
        cout << "p2 is " << p2 << "\n";
        cout << "k is " << k << "\n";
        cout << "L is " << L << "\n";
        std::random_device seed;
        std::mt19937 generator{seed()}; // seed the generator
        gen = generator;
        std::uniform_int_distribution<> dist{0, d}; // set min and max
        u_dist = dist;
        bm = get_masks();
        preprocess();
    }

    set<pair<int, boost::dynamic_bitset<>>> query(const boost::dynamic_bitset<>& q) {
        std::set<std::pair<int, boost::dynamic_bitset<>>> nns {};
        string bucket;
        for (int i = 0; i < L; ++i) {
            bucket = get_bucket(bm[i], q);
            for (const auto& x: inv_map->at(bucket)) {
                auto dist = hamming(x, q);
                if (dist > c * r)
                    continue;
                auto p = pair<int, boost::dynamic_bitset<>> {dist, x};
                if (nns.find(p) == nns.end())
                    nns.insert(p);
            }
        }
        return nns;
    }
};

#endif //PACE_DISTRIBUTION_H
