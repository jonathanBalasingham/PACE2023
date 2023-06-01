#include <iostream>
#include <vector>
#include <fstream>
#include "bitset_solver.h"
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

        //printf("%d nodes, %d edges\n", n, nedges);

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

void write_contraction_sequence(std::vector<std::pair<int, int>> cs) {
    for (auto& c : cs) {
        std::cout << c.first + 1 << " " << c.second + 1 << "\n";
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
    auto nodes_left = s.get_graph()->get_nodes();
    auto unfinished = s.get_graph()->get_solution();
    auto cs = s.get_solution();
    cs.insert(cs.end(), unfinished.begin(), unfinished.end());
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
    auto cs = finish_sequence();
    write_contraction_sequence(cs);
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

    s = Solver();
    auto sol = s.solve(G, true, "roaring", G.n > 150000);
    write_contraction_sequence(sol);
    return 0;
}
