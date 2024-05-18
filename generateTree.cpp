// c++ -I /home/cs20b012/Harmit/Downloads/boost/boost_1_85_0  generate.cpp -o compiled

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random_spanning_tree.hpp>
#include <boost/random.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <fstream>
#include <chrono>

uint64_t random_address() { char *p = new char; delete p; return uint64_t(p); }

int main(int argc, char *argv[]) {
    
    // Define the graph type with edge properties
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
        boost::no_property, boost::property<boost::edge_index_t, int>> Graph;
    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
    
    if(argc < 2) 
    {
        std::cout<<"Usage : <input_graph>";
        return EXIT_FAILURE; 
    }
    std::ifstream graphInputFile(argv[1]);
    std::ofstream graphOutputFile("spanning_tree.txt");

    int vertices , edges;
    graphInputFile>>vertices>>edges;

    // Create a graph
    Graph g(vertices);
    
    for(int i=0;i<edges;i++)
    {
        int u , v;
        graphInputFile>>u>>v;
        if(u < v)
            boost::add_edge(u, v, g);
    }

    // random number generator
    boost::mt19937 gen(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count() * (random_address() | 1)));

    std::vector<Vertex> parent(boost::num_vertices(g));

    // Add edge indices to the graph
    int index = 0;
    boost::graph_traits<Graph>::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei) {
        boost::put(boost::edge_index, g, *ei, index++);
    }

    // Taken from https://www.boost.org/doc/libs/1_85_0/libs/graph/doc/random_spanning_tree.html
    boost::random_spanning_tree(g, gen,
        boost::predecessor_map(boost::make_iterator_property_map(parent.begin(), boost::get(boost::vertex_index, g))));

    for (std::size_t i = 0; i < parent.size(); ++i) {
        if (parent[i] == std::numeric_limits<std::size_t>::max()) {
            parent[i] = i; 
        }
    }

    graphOutputFile << vertices <<std::endl;
    for (std::size_t i = 0; i < parent.size(); ++i) {
        graphOutputFile << i << " " << parent[i] << std::endl;
    }

    return 0;
}
