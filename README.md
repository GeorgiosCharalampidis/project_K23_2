Hello delapongoli

Έχω κρατήσει κάποια αρχέια από το προηγούμενο project και πρόσθεσα το graph_search.cpp , που είναι η αντίστοιχη main.

Έφτιαξα τα graph.cpp και graph.h που είναι ο αλγόριθμος αναζήτησης πλησιέστερων γειτόνων GNNS.

Ακολουθεί μία μικρή εξήγηση 

Graph Construction (buildKNNG Function):

The code constructs the k-NN graph using the LSH algorithm, which is consistent with the idea of building a k-NN graph in an offline phase.
For each point in the dataset, it queries the k nearest neighbors and adds edges to these neighbors in the graph.

GNNS Algorithm (GNNS Function):
The GNNS function in your code performs a hill-climbing search starting from randomly chosen nodes.
It iterates for a fixed number of greedy steps (T) and expansions (E), choosing at each step the neighbor closest to the query point, which is consistent with the GNNS algorithm described in the paper.
The function sorts the potential neighbors based on their distance to the query point and then selects the top N.

Graph Data Structure:
Your graph class (Graph) seems to appropriately handle storing nodes, adding edges, and retrieving neighbors, which are essential for implementing the GNNS algorithm.

