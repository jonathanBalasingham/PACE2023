#include <iostream>
#include <vector>
#include <fstream>
#include "bitset_solver.h"
#include <csignal>
#include <string>

long mystoi(const char *s)
{
    long i;
    i = 0;
    while(*s >= '0' && *s <= '9')
    {
        i = i * 10 + (*s - '0');
        s++;
    }
    return i;
}

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

bool is_file(std::string& str) {
    return str.find('.') != std::string::npos;
}

graph from_string(const std::string& graph_as_string) {
    graph G;
    int i,j,n;
    adj *a;
    node *R;

    auto lines = split(graph_as_string, "\n");
    auto firstLine = lines[0];
    auto parameters = split(firstLine, " ");
    n = mystoi(parameters[2].c_str());
    int nedges = mystoi(parameters[3].c_str());

    G.n=n;
    G.e=nedges;
    G.G=(adj **)malloc(n*sizeof(adj *));

    for(i=0;i<n;i++)
        G.G[i]=NULL;

    int line_number = 0;
    for (auto& line : lines) {
        if (line_number == 0)
            line_number++;

        if (!line.empty() or line[0] == 'c') {
            auto vertices_involved = split(line, " ");
            auto n1 = stoi(vertices_involved[0]) - 1;
            auto n2 = stoi(vertices_involved[1]) - 1;
            add_edge(G, n1, n2);
        }
    }
    return G;
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
        n = mystoi(parameters[2].c_str());
        nedges = mystoi(parameters[3].c_str());

        G.n=n;
        G.e=nedges;
        G.G=(adj **)malloc(n*sizeof(adj *));

        for(i=0;i<n;i++)
            G.G[i]=NULL;

        while (std::getline(f, line)) {
            if (!line.empty() or line[0] == 'c') {
                auto vertices_involved = split(line, " ");
                auto n1 = mystoi(vertices_involved[0].c_str()) - 1;
                auto n2 = mystoi(vertices_involved[1].c_str()) - 1;
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
std::string filename = "r.tww";

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

std::string get_line() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

std::string read_token() {
    std::string result;
    std::cin >> std::noskipws;
    char c;
    while (std::cin >> c)
    {
        if(c == '\t' || c == ' ')
            break;
        result.push_back(c);
    }
    return result;
}

graph from_cin2() {
    int i, j, n;
    int nedges;
    char c;
    std::string tww;
    cin >> c >> tww >> n >> nedges;
    int edges_added = 0;
    graph G;
    G.n=n;
    G.e=nedges;
    G.G=(adj **)malloc(n*sizeof(adj *));
    for(i=0;i<n;i++)
        G.G[i]=NULL;

    while(edges_added < nedges) {
        cin >> i >> j;
        add_edge(G, i - 1, j - 1);
        edges_added++;
    }
    return G;
}

graph from_cin() {
    graph G;
    int i,j,n;
    int nedges = -1;
    char current_char;
    int n1, n2;
    int edges_added = 0;

    while (true) {
        cin >> current_char;
        if (current_char == '\n')
            continue;
        if (current_char == 'c') {
            cin >> current_char;
        } else if (current_char == 'p') {
            //cin >> current_char; // space
            cin >> current_char; // t
            cin >> current_char; // w
            cin >> current_char; // w
            //cin >> current_char; // space
            cin >> n;
            //cin >> current_char;
            cin >> nedges;
            G.n=n;
            G.e=nedges;
            G.G=(adj **)malloc(n*sizeof(adj *));
            for(i=0;i<n;i++)
                G.G[i]=NULL;

            while (edges_added != nedges) {
                cin >> n1;
                cin >> n2;
                add_edge(G, n1 - 1, n2 - 1);
                edges_added++;
            }
        }
        if (edges_added == nedges) {
            break;
        }
    }
    return G;
}

graph from_stdin() {
    graph G;
    int i,j,n;
    adj *a;
    node *R;
    std::string line = get_line();
    while (line.empty()) {
        line = get_line();
    }

    if (line.ends_with(".gr")) {
        return from_file(line, 0);
    } else {
        std::vector<std::string> lines;
        int nedges = -1;
        int edges_added = 0;
        while (true) {
            if (line.starts_with("p")) {
                auto pline = split(line, " ");
                nedges = mystoi(pline[3].c_str());
                n = mystoi(pline[2].c_str());
                G.n=n;
                G.e=nedges;
                G.G=(adj **)malloc(n*sizeof(adj *));

                for(i=0;i<n;i++)
                    G.G[i]=NULL;

            } else if(!line.empty() and !line.starts_with("c")) {
                auto edge = split(line , " ");
                int n1 = mystoi(edge[0].c_str()) - 1;
                int n2 = mystoi(edge[1].c_str()) - 1;
                add_edge(G, n1, n2);
                edges_added++;
            }

            if (edges_added == nedges)
                break;
            line = get_line();
        }
    }
    return G;
}

void handle_sigint( int signum ) {
    auto cs = finish_sequence();
    write_contraction_sequence(cs);
    exit(signum);
}

int main(int argc, char** argv) {
    graph G;
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    G = from_stdin();

    //if (argc < 2) {
        //G = from_cin2();
    //    G = from_stdin();
    //} else {
    //    G = from_file(argv[1], 0);
    //}

    signal(SIGINT, handle_sigint);
    auto sol = s.solve(G, true, "roaring", G.n > 150000);
    write_contraction_sequence(sol);
    return 0;
}
