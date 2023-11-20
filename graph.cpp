//
// Created by test on 11/20/2023.
//

#include <iostream>
#include <algorithm> // For std::sort, std::min_element
#include <random>
#include <limits>
#include "graph.h"
#include "global_functions.h"

Graph::Graph(int size) : nodes(size) {}

void Graph::addEdge(int src, int dest) {
    nodes[src].neighbors.insert(dest);
    // Uncomment the next line for an undirected graph
    // nodes[dest].neighbors.insert(src);
}

const std::set<int>& Graph::getNeighbors(int nodeIndex) const {
    return nodes[nodeIndex].neighbors;
}

Graph buildKNNG(LSH &lsh, int k, int datasetSize) {
    Graph kNNG(datasetSize);

    for (int i = 0; i < datasetSize; ++i) {
        const std::vector<unsigned char>& queryPoint = lsh.getDataset()[i];
        kNNG.storePoint(queryPoint);

        auto neighbors = lsh.queryNNearestNeighbors(queryPoint, k);
        for (const auto& neighbor : neighbors) {
            int neighborIndex = neighbor.first;
            kNNG.addEdge(i, neighborIndex);
        }
    }

    return kNNG;
}

void Graph::storePoint(const std::vector<unsigned char>& point) {
    dataPoints.push_back(point);
}

const std::vector<unsigned char>& Graph::getPoint(int nodeIndex) const {
    return dataPoints[nodeIndex];
}

int Graph::size() const {
    return nodes.size();
}


std::vector<std::pair<int, double>> GNNS(const Graph& graph, const std::vector<unsigned char>& queryPoint, int N, int R, int T, int E) {
    std::vector<std::pair<int, double>> potentialNeighbors;

    std::random_device rd;
    std::default_random_engine engine(rd());

    for (int r = 0; r < R; ++r) {
        std::uniform_int_distribution<int> distribution(0, graph.size() - 1);
        int currentNode = distribution(engine);

        for (int t = 0; t < T; ++t) {
            const auto& neighbors = graph.getNeighbors(currentNode);
            int bestNeighbor = currentNode;
            double bestDistance = std::numeric_limits<double>::max();

            int count = 0;
            for (int neighbor : neighbors) {
                if (count >= E) break;
                double distance = euclideanDistance(graph.getPoint(neighbor), queryPoint);
                potentialNeighbors.emplace_back(neighbor, distance); // Store each neighbor along with the distance

                if (distance < bestDistance) {
                    bestDistance = distance;
                    bestNeighbor = neighbor;
                }
                count++;
            }

            currentNode = bestNeighbor;
        }
    }

    // Sort potential neighbors based on their distance to the query point
    std::sort(potentialNeighbors.begin(), potentialNeighbors.end(),
              [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                  return a.second < b.second;
              });

    // Remove duplicates
    auto last = std::unique(potentialNeighbors.begin(), potentialNeighbors.end(),
                            [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
                                return a.first == b.first;
                            });
    potentialNeighbors.erase(last, potentialNeighbors.end());

    // Resize to keep only the first N elements
    if (potentialNeighbors.size() > N) {
        potentialNeighbors.resize(N);
    }

    return potentialNeighbors;
}

