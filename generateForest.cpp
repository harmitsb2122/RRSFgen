#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random_spanning_tree.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/random.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <fstream>
#include <boost/bimap.hpp>

#include <chrono>
#include <queue>
#include <iomanip>

uint64_t random_address() { char *p = new char; delete p; return uint64_t(p); }

int numberOfComponents(std::vector<int> &par)
{
	std::map<int,int> components;
	for(auto i : par)
	{
		components[i]++;
	}
	return components.size();
}

int markedComponents(std::vector<int> &marked)
{
	int count = 0;
	for(auto i : marked)
	{
		count += (i != -1);
	}
	return count;
}

int rootedComponents(std::vector<int> &par)
{
	int count = 0;
	for (int i = 0; i < (int)par.size(); i++)
	{
		count += (i == par[i]);
	}
	return count;
	
}

int treeDepth(int root,std::vector<std::vector<int>> &adj,int n)
{
	std::vector<int> depth(n,1e9);
	std::queue<int> q;
	q.push(root);
	depth[root] = 0;

	while(!q.empty())
	{
		int cur = q.front();
		q.pop();

		for(auto child : adj[cur])
		{
			if(depth[child] > depth[cur] + 1)
			{
				depth[child] = depth[cur] + 1;
				q.push(child);
			}
		}
	}

	for(int i=0;i<n;i++){
		if(depth[i] == 1e9) depth[i] = -1;
	}

	return *max_element(depth.begin(),depth.end());
}

bool dfs_directed(int src,std::vector<int> &vis,std::vector<std::vector<int>> &adj)
{
	vis[src] = 1;
	for(auto child : adj[src]){
		if(vis[child] == 0){
			if(dfs_directed(child,vis,adj)){
				return true;
			}
		}
		else if(vis[child] == 1){
			return true;
		}
	}
	vis[src] = 2;
	return false;
}

bool validateRST(const std::vector<int> &parent,const int comp_count)
{
	int n = parent.size();
	std::vector<int> roots;

	for (int i = 0; i < n; i++)
	{
		if(parent[i] == i)
			roots.push_back(i);
	}

	int k = roots.size();

	if(k != comp_count)
	{
		std::cout<<"Wrong : found "<< k <<" roots instead of "<<comp_count<<std::endl;
		return false;		
	}

	std::vector<std::vector<int>> adj(n);
	for (int i = 0; i < n; i++)
	{
		if(parent[i] != i)
			adj[parent[i]].push_back(i);
	}

	std::vector<int> vis(n,0);
	
	for(auto root : roots)
	{
		if(dfs_directed(root,vis,adj))
		{
			std::cout<<"Wrong : Tree has cycles - node["<<root<<"]"<<std::endl;
			return false;
		}
	}

	for(int i=0;i<n;i++)
	{
		if(vis[i] != 2)
		{	
			std::cout<<"Wrong : Tree has cycles - node["<<i<<"]"<<std::endl;
			return false;
		}
	}

	std::vector<int> max_tree_depths;
	for(auto root : roots)
	{
		max_tree_depths.push_back(treeDepth(root,adj,n));
	}

	std::cout << "Max tree depth = " << *max_element(max_tree_depths.begin(),max_tree_depths.end()) << std::endl;
	std::cout << std::fixed << std::setprecision(2) << "Avg tree depth = " << accumulate(max_tree_depths.begin(),max_tree_depths.end(),(double)0) / (double)roots.size() << std::endl;
	return true;
}

int main(int argc, char *argv[]) {
    // Define the graph type with edge properties
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
        boost::no_property, boost::property<boost::edge_index_t, int>> Graph;
    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

    if (argc < 2) {
        std::cout << "Usage: <input_graph>" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream graphInputFile(argv[1]);
    std::ofstream graphOutputFile("spanning_forest.txt");

    int vertices, edges;
    graphInputFile >> vertices >> edges;

    // Create a graph
    Graph g(vertices);
    for (int i = 0; i < edges; ++i) {
        int u, v;
        graphInputFile >> u >> v;
        if(u < v)
            boost::add_edge(u, v, g);
    }

    // Random number generator
    boost::mt19937 gen(static_cast<unsigned int>(static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count() * (random_address() | 1))));

    // Find the connected components
    std::vector<int> component(boost::num_vertices(g));
    int num_components = boost::connected_components(g, &component[0]);

    // parent array
    std::vector<Vertex> parent(boost::num_vertices(g));

    std::cout<<"Number of components in the graph : "<<num_components<<"\n";
    
    //random spanning tree for each component
    for (int i = 0; i < num_components; ++i) {
        
        // bimap to map vertices between the original graph and the subgraph
        typedef boost::bimap<Vertex, Vertex> VertexMap;
        VertexMap vertex_map;

        Graph subgraph;

        for (auto v : boost::make_iterator_range(boost::vertices(g))) {
            if (component[v] == i) {
                Vertex subgraph_vertex = boost::add_vertex(subgraph);
                vertex_map.insert({v, subgraph_vertex});
            }
        }
        
        for (auto e : boost::make_iterator_range(boost::edges(g))) {
            Vertex u = boost::source(e, g);
            Vertex v = boost::target(e, g);
            if (component[u] == i && component[v] == i) {
                boost::add_edge(vertex_map.left.at(u), vertex_map.left.at(v), subgraph);
            }
        }
         
        std::vector<Vertex> subgraph_parent(boost::num_vertices(subgraph), boost::graph_traits<Graph>::null_vertex());

        // Random spanning tree for the subgraph
        boost::random_spanning_tree(subgraph, gen,
            boost::predecessor_map(boost::make_iterator_property_map(subgraph_parent.begin(), boost::get(boost::vertex_index, subgraph))));

        for (std::size_t i = 0; i < subgraph_parent.size(); ++i) {
            if (subgraph_parent[i] == std::numeric_limits<std::size_t>::max()) {
                subgraph_parent[i] = i; 
            }
        }

        // Remap the parent array back to the original graph
        for (auto v : boost::make_iterator_range(boost::vertices(subgraph))) {
            Vertex original_vertex = vertex_map.right.at(v);
            if (subgraph_parent[v] == v) 
            {
                parent[original_vertex] = original_vertex; 
            } else 
            {
                parent[original_vertex] = vertex_map.right.at(subgraph_parent[v]);
            }
        }
    }

    for (std::size_t i = 0; i < parent.size(); ++i) {
        if (parent[i] == std::numeric_limits<std::size_t>::max()) {
            parent[i] = i; 
        }
    }

    std::vector<int> std_parent(vertices);
    graphOutputFile << vertices << std::endl;
    for (std::size_t i = 0; i < parent.size(); ++i) {
        graphOutputFile << i << " " << parent[i] << std::endl;
        std_parent[i] = parent[i];
    }

    if(validateRST(std_parent,num_components))
    {
        std::cout<<"Validation Success. Random Forest created!\n";
    }
    else
    {
        std::cout<<"Error!\n";
    }

    return 0;
}
