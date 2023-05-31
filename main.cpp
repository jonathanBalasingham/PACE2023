#include <iostream>
#include <vector>
#include <fstream>
#include "bitset_solver.h"
#include "quick_solver.h"
#include "bfs_solver.h"
//#include "graph_v2.h"
#include <csignal>

std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

void add_edge(graph G, int i, int j, int verbose=0) {
    adj* a=(adj *)malloc(sizeof(adj));
    a->s=j;
    a->suiv=G.G[i];
    G.G[i]=a;
    a=(adj *)malloc(sizeof(adj));
    a->s=i;
    a->suiv=G.G[j];
    G.G[j]=a;
    if(verbose)
        printf("%i ",j);
}


graph from_file(const std::string& file, int verbose) {
    std::ifstream f(file);
    std::cout << "building graph\n";
    std::string line;
    graph G;
    int i,j,n;
    adj *a;
    node *R;
    int nedges;


    if (f.is_open()) {

        std::string firstLine;
        std::getline(f, firstLine);
        auto parameters = split(firstLine, " ");
        n = std::stoi(parameters[2]);
        nedges = std::stoi(parameters[3]);

        printf("%d nodes, %d edges\n", n, nedges);

        G.n=n;
        G.e=nedges;
        G.G=(adj **)malloc(n*sizeof(adj *));

        for(i=0;i<n;i++)
            G.G[i]=NULL;

        while (std::getline(f, line)) {
            if (!line.empty()) {
                auto vertices_involved = split(line, " ");
                auto n1 = stoi(vertices_involved[0]) - 1;
                auto n2 = stoi(vertices_involved[1]) - 1;
                add_edge(G, n1, n2);
            }
        }
        f.close();

        if (verbose) {
            for (int k = 0; k < n; ++k) {
                printf("node %d: ", k);
                for(a=G.G[k]; a!=NULL; a=a->suiv)
                    printf("%i ",a->s);
                printf("\n");
            }
        }
    } else {
        std::cout << "File not found. Exiting..\n";
        exit(1);
    }

    return G;
}





using namespace roaring;

void write_default_solution(std::string filename, graph G) {
    ofstream f(filename);
    if(f.is_open()) {
        for (int i = 1; i < G.n; ++i) {
            f << i + 1 << " " << i << "\n";
        }
        f.close();
    }
}

void write_contraction_sequence(std::string filename, std::vector<std::pair<int, int>> cs) {
    ofstream f(filename);
    if(f.is_open()) {
        for (auto& c : cs) {
            f << c.first + 1 << " " << c.second + 1 << "\n";
        }
        f.close();
    }
}

Solver s;
std::string filename = "results.tww";

vector<pair<int, int>> finish_sequence() {
    std::cout << "getting remaining nodes..";
    auto nodes_left = s.get_graph()->get_nodes();
    std::cout << "Retrieving current solution...\n";
    auto unfinished = s.get_graph()->get_solution();
    std::cout << "Getting unfinished solution...\n";
    std::cout << "Unfinished size: " << unfinished.size() << "\n";
    auto cs = s.get_solution();
    std::cout << "Combining..\n";
    std::cout << "Current solution size: " << cs.size() << "\n";
    cs.insert(cs.end(), unfinished.begin(), unfinished.end());
    std::cout << "Total completed contractions: " << cs.size() << "\n";
    int i = 0;
    int start_node;
    for (auto n : nodes_left) {
        if (i == 0) {
            start_node = n;
            i++;
            continue;
        }
        cs.emplace_back(start_node, n);
        i++;
    }
    return cs;
}

void handle_sigint( int signum ) {
    std::cout << "wrapping up...\n";
    auto cs = finish_sequence();
    write_contraction_sequence(filename, cs);
    exit(signum);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "No input file provided. Exiting...\n";
        exit(1);
    }

    signal(SIGINT, handle_sigint);

    auto G = from_file(argv[1], 0);
    std::string fn = std::string(argv[1]).substr(std::string(argv[1]).find_last_of("/\\") + 1);
    filename = split(fn, ".")[0] + ".tww";
    write_default_solution(filename, G);

    float con = ((float) G.e) / (float) pow(G.n, 2);
    float epn = ((float) G.e) / ((float) G.n);

    s = Solver();
    auto sol = s.solve(G, true, "roaring");
    write_contraction_sequence(filename, sol);
    return 0;
}
