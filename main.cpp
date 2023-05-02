#include <iostream>
#include <vector>
#include <fstream>
#include "Distribution.h"


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
        case SERIE: C[4*level]++; break;
        case PARALLELE: C[4*level+1]++; break;
        case PREMIER: C[4*level+2]++; break;
        case FEUILLE: C[4*level+3]++; break;
    }
    if(N->type!=FEUILLE)
        for(F=N->fils;F!=NULL;F=F->suiv)
            compte(F->pointe, level+1, C);
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

    int verbose = 1;
    // appel de la fonction de decomposition
    auto R = decomposition_modulaire(G);

    // affichage de l'arbre
    if(verbose)
        printarbre(R);

    auto edge_sets = vector<boost::dynamic_bitset<>>(G.n, boost::dynamic_bitset<>(G.n));
    for (int i = 0; i < G.n; ++i) {
        for (auto n = G.G[i]; n != NULL; n=n->suiv) {
            edge_sets[i][n->s] = true;
        }
    }

    auto s = Sampler(edge_sets, 25, 5);
    std::cout << "Inv Map " << s.inv_map->size() << " results.\n";
    auto res = s.query(edge_sets[0]);
    std::cout << "Returned " << res.size() << " results.\n";

    int j = 0;
    for (auto p : res) {
        cout << "Distance: " << p.first << "\n";
        j++;
        if (j > 10)
            break;
    }

    //compte(R,0,C);
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
