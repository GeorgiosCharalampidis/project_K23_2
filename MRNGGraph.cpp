#include "MRNGGraph.h"
#include <algorithm>


MRNGNode::MRNGNode(const std::vector<unsigned char>& data) : data(data) {}

MRNGGraph::MRNGGraph(const std::vector<std::vector<unsigned char>>& dataset, int l, int N) {
    // Reserve memory for nodes
    nodes.reserve(dataset.size());

    // Construct the graph from the dataset
    for (const auto& dataPoint : dataset) {
        nodes.emplace_back(dataPoint);
    }

    // MRNG construction
    for (auto& p : nodes) {
        std::vector<MRNGNode*> Rp;
        Rp.reserve(nodes.size() - 1);

        for (auto& r : nodes) {
            if (&p != &r) {
                Rp.push_back(&r);
            }
        }

        // Store distances and sort Rp according to distance to p
        std::vector<std::pair<MRNGNode*, double>> distRp;
        for (auto* node : Rp) {
            double dist = euclideanDistance(p.data, node->data);
            distRp.emplace_back(node, dist);
        }

        std::sort(distRp.begin(), distRp.end(), [](const auto& a, const auto& b) {
            return a.second < b.second;
        });

        std::vector<MRNGNode*> Lp;
        Lp.reserve(l); // Reserve memory for Lp

        // Initialize Lp with the closest N nodes to p in Rp
        for (int i = 0; i < std::min(N, static_cast<int>(distRp.size())); ++i) {
            Lp.push_back(distRp[i].first);
        }

        // Evaluate remaining nodes in Rp
        for (const auto& [r, dist_pr] : distRp) {
            if (std::find(Lp.begin(), Lp.end(), r) != Lp.end() || Lp.size() >= l) {
                continue;
            }
            bool isLongestEdge = true;
            for (auto& t : Lp) {
                double dist_pt = euclideanDistance(p.data, t->data);
                double dist_rt = euclideanDistance(r->data, t->data);

                if (dist_pr <= dist_pt || dist_pr <= dist_rt) {
                    isLongestEdge = false;
                    break;
                }
            }
            if (isLongestEdge) {
                Lp.push_back(r);
            }
        }

        p.neighbors = Lp;
    }
}


std::vector<std::pair<int, double>> MRNGGraph::searchOnGraph(const std::vector<unsigned char>& query, int startNodeIndex, int k, int l) {
    std::vector<std::pair<int, double>> potentialNeighbors;
    std::vector<MRNGNode*> candidatePool;
    const auto& nodes_ = this->getNodes();

    candidatePool.push_back(const_cast<MRNGNode*>(&nodes_[startNodeIndex]));

    while (candidatePool.size() < l) {
        auto currentNode = candidatePool.front();
        candidatePool.erase(candidatePool.begin());

        for (auto& neighbor : currentNode->neighbors) {
            double distance = euclideanDistance(query, neighbor->data);
            int nodeIndex = std::distance(nodes_.begin(), std::find_if(nodes_.begin(), nodes_.end(),
                                                                      [neighbor](const MRNGNode& node) {
                                                                          return &node == neighbor;
                                                                      }));
            potentialNeighbors.emplace_back(nodeIndex, distance);

            candidatePool.push_back(neighbor);
        }

        // Sort and limit candidatePool size to l
        std::sort(candidatePool.begin(), candidatePool.end(), [&](MRNGNode* a, MRNGNode* b) {
            return euclideanDistance(query, a->data) < euclideanDistance(query, b->data);
        });
        if (candidatePool.size() > l) {
            candidatePool.resize(l);
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

    // Keep only the top k elements
    if (potentialNeighbors.size() > k) {
        potentialNeighbors.resize(k);
    }

    return potentialNeighbors;
}


