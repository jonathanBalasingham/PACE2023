#include <iostream>
#include <vector>
#include <fstream>
#include "solver.h"


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
    // et reciproquement
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


void compte(node *N, int level, int *C) {
    fils *F;
    switch(N->type)
    {
        case SERIES: C[4 * level]++; break;
        case PARALLEL: C[4 * level + 1]++; break;
        case PRIME: C[4 * level + 2]++; break;
        case LEAF: C[4 * level + 3]++; break;
    }
    if(N->type != LEAF)
        for(F=N->fils;F!=NULL;F=F->suiv)
            compte(F->pointe, level+1, C);
}


/*
 * The idea is to run this function on the MDTree repeatedly
 * and on each iteration, remove modules (portion of tree with leaves only).
 * On each application the tree will be pruned.
 * */
void traverse(node *N) {
    fils *F;
    if (N->fils == NULL) {
        std::cout << "hit a dead end.. moving up\n";
    } else {
        bool mod = true;
        std::vector<int> leaves;
        for (F = N->fils; F != NULL; F = F->suiv) {
            if (F->pointe->type != LEAF) {
                mod = false;
                traverse(F->pointe);
            } else
                leaves.push_back(F->pointe->nom);
        }

        if (mod) {
            cout << "found a module: {";
            for (auto leaf: leaves)
                cout << leaf << " ";
            cout << "}\n";

            // we need to process this portion of the graph

            // generate contraction sequence, last node remaining
            // is the new leaf
            N->type = LEAF;
            N->nom = leaves[0]; // this is just an example
            // then remove this branch
            N->fils = NULL;
        }
    }

}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "No input file provided. Exiting...\n";
        exit(1);
    }

    auto NIV = 20;
    auto G = from_file(argv[1], 0);
    int C[4*NIV];
    int i;

    int verbose = 0;
    // appel de la fonction de decomposition
    auto R = decomposition_modulaire(G);

    // affichage de l'arbre
    if(verbose)
        printarbre(R);

    for(i=0;i<4*NIV;i++) C[i]=0;

    compte(R,0, C);

    auto s = Solver();
    auto sol = s.solve(G);

    printf("Decomposition Tree Statistics:\n");
    if(C[0])
        printf("The root is Series\n");
    else if(C[1])
        printf("The root is Parallel\n");
    else
        printf("The root is Prime \n");
    for(i=1 ; i<NIV ; i++) {
        printf("Level %i: %i modules (S-P-Pr= %i - %i - %i) and %i leaves\n",i,
               C[4*i]+C[4*i+1]+C[4*i+2], C[4*i], C[4*i+1], C[4*i+2],C[4*i+3]);
        if(i<NIV-1 && C[4*i+4]+C[4*i+5]+C[4*i+6]+C[4*i+7]==0)
            break;
    }
    printf("\n");

    return 0;
}
