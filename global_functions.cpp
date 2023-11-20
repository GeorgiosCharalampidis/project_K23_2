#include "global_functions.h"
#include <vector>
#include <cmath>
#include <stdexcept>
#include <random>
#include <algorithm>
#include <queue>


//
// Functions that will be used throughout the project
//




// Υπολογισμός της ευκλείδιας απόστασης μεταξύ δύο διανυσμάτων
double euclideanDistance(const std::vector<unsigned char>& dataset, const std::vector<unsigned char>& query_set) {
    if (dataset.size() != query_set.size()) {
        throw std::runtime_error("Vectors must have the same dimension for L2 distance calculation.");
    }
    double distance = 0.0;
    for (size_t i = 0; i < dataset.size(); ++i) {
        double diff = static_cast<double>(dataset[i]) - static_cast<double>(query_set[i]);
        distance += diff * diff;
    }
    return std::sqrt(distance);
}

int computeDPrime(int n) {
    int logValue = static_cast<int>(std::log2(n));
    int d_prime_lower_bound = logValue - 3;
    int d_prime_upper_bound = logValue - 1;

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(d_prime_lower_bound, d_prime_upper_bound);

    return dist(mt);
}

std::vector<std::pair<int, double>> trueNNearestNeighbors(const std::vector<std::vector<unsigned char>>& dataset,
                                                          const std::vector<unsigned char>& query_point, int N) {
    // Check for dataset's emptiness
    if (dataset.empty()) {
        throw std::runtime_error("Dataset is empty.");
    }

    if (N <= 0) {
        throw std::invalid_argument("N must be positive.");
    }

    // Pair: distance, index. We use distance as the key for the priority queue.
    std::priority_queue<std::pair<double, int>> max_heap;

    for (int i = 0; i < dataset.size(); ++i) {
        double distance = euclideanDistance(dataset[i], query_point);

        // If we haven't yet found N neighbors, or the current point is closer than the farthest neighbor found so far.
        if (max_heap.size() < N || distance < max_heap.top().first) {
            if (max_heap.size() == N) {
                max_heap.pop(); // Remove the farthest neighbor
            }
            max_heap.emplace(distance, i); // Add the current point
        }
    }

    std::vector<std::pair<int, double>> nearest_neighbors;
    while (!max_heap.empty()) {
        nearest_neighbors.emplace_back(max_heap.top().second, max_heap.top().first);
        max_heap.pop();
    }

    // The priority queue will order from largest to smallest distance. So, reverse for correct ordering.
    std::reverse(nearest_neighbors.begin(), nearest_neighbors.end());

    return nearest_neighbors;
}

std::vector<unsigned char> convertToUnsignedChar(const std::vector<double>& vec) {
    std::vector<unsigned char> result;
    result.reserve(vec.size());

    for(const auto& val : vec) {
        result.push_back(static_cast<unsigned char>(std::round(val)));
    }

    return result;
}
