//
// Created by jonathan on 5/7/23.
//

#ifndef PACE_MCTS_H
#define PACE_MCTS_H


#include <memory>
#include <vector>
#include "tree.h"
#include "graph.h"
#include "Distribution.h"

using ContractionPair = std::pair<int, int>;
using ContractionSequence = std::vector<ContractionPair>;

class MonteCarloTreeSearch {
private:
    int num_candidates;
    int sim_depth;
    int sims;
    std::unique_ptr<Tree<ContractionPair>> tree;
    std::vector<int> solution_path;
    ContractionSequence contraction_sequence;
    std::unique_ptr<Distribution> dist;
    std::shared_ptr<Graph> G;

    ContractionSequence generate_random_moves(Graph g, int n) {
        auto v = g.vertices();

        std::vector<int> out;
        std::sample(
                v.begin(),
                v.end(),
                std::back_inserter(out),
                n * 2,
                std::mt19937{std::random_device{}()}
        );

        auto moves = std::vector<std::pair<int, int>>(n);
        for (int i = 0; i < n; i+=2) {
            moves[i / 2] = {out[i], out[i+1]};
        }
        return moves;
    }

    float simulate(Graph current_graph, const shared_ptr<Node<ContractionPair>>& n) {
        //std::cout << "Starting simulations for move: " << n->content.first << ", " << n->content.second << "\n";
        vector<float> simulations;
        current_graph.contract(n->content.first, n->content.second);
        auto simulation_results = vector<vector<float>>(sims, vector<float>(sim_depth));
        for (int i = 0; i < sims; ++i) {
            auto dist_copy = *dist;
            dist_copy.remove({n->content});

            for (int j = 0; j < sim_depth; ++j) {
                auto move = dist_copy.sample();
                //std::cout << "contracting: " << move[0].first << ", " << move[0].second << "\n";
                current_graph.contract(move[0].first, move[0].second);
                simulation_results[i][j] = (float) current_graph.get_max_red_degree();
                if (current_graph.order() < 2)
                    break;
            }
        }
        
        auto scores = vector<float>(sims);
        for (int i = 0; i < sims; ++i) {
            scores[i] = std::reduce(simulation_results[i].begin(), simulation_results[i].end()) / ((float) simulation_results[i].size());
        }
        return std::reduce(scores.begin(), scores.end()) / ((float) sims);
    }


public:
    MonteCarloTreeSearch(int candidate_nodes=5, int simulation_depth=3, int num_simulations=1) {
        num_candidates = candidate_nodes;
        sim_depth = simulation_depth;
        sims = num_simulations;
        solution_path = {};
        contraction_sequence = {};
        tree = make_unique<Tree<ContractionPair>>(Tree<ContractionPair>());
    }

    std::shared_ptr<Node<ContractionPair>> traverse_path(const std::vector<int>& path) {
        auto current_node = tree->get_root();
        for (auto p: path) {
            current_node = current_node->children[p];
        }
        return current_node;
    }

    Graph get_graph(const Graph& g) {
        Graph g_copy = Graph(g);
        for (auto contraction : contraction_sequence) {
            g_copy.contract(contraction.first, contraction.second);
        }
        return g_copy;
    }

    void expand(std::shared_ptr<Node<ContractionPair>> n, const vector<int>& subset,
                const std::string& method="setdiff") {
        dist->remove(contraction_sequence);
        auto moves = dist->sample(num_candidates, false);

        for (auto move: moves) {
            auto new_node = Node<ContractionPair>(move);
            tree->insert(n, make_shared<Node<ContractionPair>>(new_node));
        }

        vector<float> scores;
        int selected_child = -1;
        float best_score;
        auto current_graph = get_graph(*G);

        if (current_graph.order() == 2) {
            contraction_sequence.emplace_back(current_graph.vertices()[0], current_graph.vertices()[1]);
            return;
        }

        auto rd = (float) current_graph.get_max_red_degree();

        for (int i = 0; i < n->children.size(); ++i) {
            scores.push_back(rd - simulate(current_graph, n->children[i]));
            if (selected_child == -1) {
                selected_child = i;
                best_score = scores.back();
            } else {
                if (scores.back() > best_score)
                    selected_child = i;
            }
        }
        dist->remove({moves[selected_child]});
        solution_path.push_back(selected_child);
        contraction_sequence.push_back(moves[selected_child]);
    }

    std::vector<std::pair<int,int>> solve(const shared_ptr<Graph>& g, const vector<int>& subset,
                                          const std::string& method="setdiff") {
        G = g;
        int depth = 0;
        dist = make_unique<Distribution>(Distribution(g, subset));
        auto n = tree->get_root();
        // collapsing twins is not necessary when using the distribution
        while (depth + 1 < subset.size()) {
            expand(n, subset, method);
            n = traverse_path(solution_path);
            depth++;
        }
        return contraction_sequence;
    }

};

#endif //PACE_MCTS_H
