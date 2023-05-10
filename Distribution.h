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
//#include "dm_english.h"
#include "graph.h"
#include <boost/dynamic_bitset.hpp>
#include <random>
#include <algorithm>
#include <iostream>

using namespace std;

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
        assert(r < d);
        assert(c*r < d);
        assert(k <= d);
        std::random_device seed;
        std::mt19937 generator{seed()}; // seed the generator
        gen = generator;
        std::uniform_int_distribution<> dist{0, d}; // set min and max
        u_dist = dist;
        bm = get_masks();
        cout << "made it to preprocess\n";
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

class Distribution {
private:
    map<pair<int, int>, vector<int>> symmetric_differences;
    vector<pair<int, int>> keys;
    vector<float> cdf;
    default_random_engine gen;
    uniform_real_distribution<float> distribution = uniform_real_distribution<float>(0.0, 1.0);


    void get_sym_differences(const shared_ptr<Graph>& G, const vector<int>& node_subset) {
        //auto probs = vector<float>(pow((float)node_subset.size(), 2));
        keys = vector<pair<int,int>>(); // ((int) pow((float)node_subset.size(), 2));
        symmetric_differences = map<pair<int, int>, vector<int>>();
        int k = 0;
        for (int i = 0; i < node_subset.size() - 1; ++i) {
            for (int j = i + 1; j < node_subset.size(); ++j) {
                if (i == j)
                    continue;
                int a = node_subset[i];
                int b = node_subset[j];
                symmetric_differences.insert({{a, b}, G->sym_diff(a, b)});
                //probs[k] = 1.0 / pow(max((float) symmetric_differences[{i, j}].size(), (float) 0.001), 1);
                //keys[k] = {i, j};
                keys.emplace_back(a, b);
                k++;
            }
        }
    }

    void make_distribution() {
        auto probs = vector<float>(symmetric_differences.size());
        int k = 0;
        for (const auto& p : symmetric_differences) {
            probs[k] = 1 / pow(max((float) p.second.size(), (float) 0.001), 1);
            k++;
        }

        auto sum_prob = std::reduce(probs.begin(), probs.end());
        for (float & prob : probs) {
            prob /= sum_prob;
        }

        cdf = vector<float>(probs.size());
        float cumsum = 0.;
        for (int i = 0; i < cdf.size(); ++i){
            cumsum += probs[i];
            cdf[i] = cumsum;
        }
    }

public:
    Distribution(const shared_ptr<Graph>& G, const vector<int>& node_subset, int r=25, int c=5) {
        //auto edge_sets = vector<boost::dynamic_bitset<>>(G.order(), boost::dynamic_bitset<>(G.order()));
        //auto map_to_nodes = map<boost::dynamic_bitset<>, int>();
        //for (int indx = 0; indx < G.order(); ++indx) {
        //    for (auto n: G.neighbors(indx)) {
        //        edge_sets[indx][n] = true;
        //    }
            //for (auto n = G.G[indx]; n != NULL; n=n->suiv) {
            //    edge_sets[indx][n->s] = true;
            //}
        //    map_to_nodes.insert({edge_sets[indx], indx});
        //}

        //auto s = Sampler(edge_sets, r, c);
        // We sample the nearest neighbors for each node and build the distribution
        get_sym_differences(G, node_subset);
        //make_distribution(G, node_subset);


    }
    vector<pair<int, int>> sample(int sample_size=1, bool remove_samples=true) {
        make_distribution();

        int nsamples = std::min(sample_size, (int) cdf.size());
        auto sampled_probabilities = vector<float>(nsamples);

        for (float & sampled_probability : sampled_probabilities)
            sampled_probability = distribution(gen);

        auto moves = vector<pair<int, int>>(nsamples);
        // for each sample we find the point in the cdf that is just above it
        int moves_found = 0;
        auto indices_to_remove = vector<int>(nsamples);
        for (auto sample: sampled_probabilities) {
            // TODO: This can be fast, something like binary search?
            int index = 0;
            while (sample > cdf[index]) {
                index++;
            }
            moves[moves_found] = keys[index];
            indices_to_remove[moves_found] = index;
            moves_found++;
        }
        if (remove_samples)
            remove(moves);

        std::cout << "generated moves: ";
        for (auto m: moves)
            std::cout << m.first << "," << m.second << " ; ";
        std::cout << "\n";
        return moves;
    }

    void remove(const vector<pair<int, int>>& samples) {
        // TODO: This can be optimized, maybe at the cost of memory
        auto to_remove = std::set<int>();
        for (auto sample: samples) {
            to_remove.insert(sample.second);
        }

        keys = vector<pair<int,int>>();
        int i = 0;
        bool removed;
        for (auto it = symmetric_differences.cbegin(), next_it = it; it != symmetric_differences.cend(); it = next_it) {
            ++next_it;
            removed = false;

            if (to_remove.contains(it->first.first)){
                //std::cout << "  Erasing due to condition 1 at position " << i;
                symmetric_differences.erase(it);
                //keys.erase(keys.begin() + i);
                i--;
                removed = true;
            }

            if (to_remove.contains(it->first.second)){
                //std::cout << "  Erasing due to condition 2 at position " << i;
                symmetric_differences.erase(it);
                //keys.erase(keys.begin() + i);
                i--;
                removed = true;
            }
            if (!removed)
                keys.push_back(it->first);
            ++i;
            //std::cout << "\n";
        }
    }

};

#endif //PACE_DISTRIBUTION_H
