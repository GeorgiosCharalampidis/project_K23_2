//
// Created by test on 11/20/2023.
//

///////////////////////////////////////////////////////
//                  Start of ./graph_search          //
///////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include "mnist.h"
#include "lsh_class.h"
#include "Hypercube.h"
#include "global_functions.h"
#include "graph.h"


int main(int argc, char** argv) {

    std::vector<std::string> args(argv, argv + argc);

    std::string inputFile, queryFile, outputFile;
    int number_of_images, image_size;
    int k = 50; // Number of nearest neighbors in graph
    int E = 30; // Number of expansions
    int R = 10; // Number of random restarts
    int N = 1;  // Number of nearest neighbors to search for
    int l = 0;  // Only for Search-on-Graph
    int mode = 0; // 1 for GNNS, 2 for MRNG

    char repeatChoice = 'n'; // to control the loop
    do {

        if (args.size() == 1) {  // Only mode provided, prompt for paths

            std::cout << "Enter the path to the dataset: ";
            std::cin >> inputFile;

        } else {
            for (size_t i = 1; i < args.size(); i++) {
                if (args[i] == "-d") {
                    inputFile = args[++i];
                } else if (args[i] == "-q") {
                    queryFile = args[++i];
                } else if (args[i] == "-k") {
                    k = std::stoi(args[++i]);
                } else if (args[i] == "-E") {
                    E = std::stoi(args[++i]);
                } else if (args[i] == "-o") {
                    outputFile = args[++i];
                } else if (args[i] == "-N") {
                    N = std::stoi(args[++i]);
                } else if (args[i] == "-R") {
                    R = std::stoi(args[++i]);
                } else if (args[i] == "-l") {
                    l = std::stoi(args[++i]);
                } else if (args[i] == "-m") {
                    mode = std::stoi(args[++i]);
                }
            }
        }

        std::vector<std::vector<unsigned char>> dataset = read_mnist_images(inputFile, number_of_images, image_size);


        if (queryFile.empty()) {
            std::cout << "Enter the path to the query file: ";
            std::cin >> queryFile;
        }

        std::vector<std::vector<unsigned char>> query_set = read_mnist_images(queryFile, number_of_images,image_size);


        if (outputFile.empty()) {
            std::cout << "Enter the path for output file: ";
            std::cin >> outputFile;
        }


        LSH lsh(dataset);

        Hypercube cube(dataset);

        std::ofstream outputFileStream("output.dat");
        if (!outputFileStream.is_open() || outputFileStream.fail()) {
            std::cerr << "Failed to open output.dat for writing." << std::endl;
            return 2;
        }

        int datasetSize = 60000;

        std::cout << "Building the k-NNG..." << std::endl;

        Graph kNNG_L = buildKNNG(lsh, k, datasetSize);
        //Graph kNNG_H = buildKNNG_H(cube, k, datasetSize);

        std::cout << "Finished building the k-NNG." << std::endl;

        int T = 10; // Number of greedy steps


        std::cout << "Starting GNNS..." << std::endl;

        // Iterate through the query set
        for (size_t i = 0; i < 10; ++i) {
            outputFileStream << "\nQuery: " << i << std::endl;

            // GNNS results
            auto startGNNS = std::chrono::high_resolution_clock::now();
            auto gnnsResults = GNNS(kNNG_L, query_set[i], N, R, T, E);
            auto endGNNS = std::chrono::high_resolution_clock::now();
            double tGNNS = std::chrono::duration<double, std::milli>(endGNNS - startGNNS).count() / 1000.0;

            // True nearest neighbors for the query point
            auto startTrue = std::chrono::high_resolution_clock::now();
            auto trueResults = trueNNearestNeighbors(dataset, query_set[i], N);
            auto endTrue = std::chrono::high_resolution_clock::now();
            double tTrue = std::chrono::duration<double, std::milli>(endTrue - startTrue).count() / 1000.0;

            // Calculate Maximum Approximation Factor (MAF)
            double maxApproximationFactor = 0.0;
            for (int j = 0; j < N; ++j) {
                double distanceApproximate = gnnsResults[j].second;
                double distanceTrue = trueResults[j].second;

                outputFileStream << "Nearest neighbor-" << j + 1 << ": " << gnnsResults[j].first << std::endl;
                outputFileStream << "distanceApproximate: " << distanceApproximate << std::endl;
                outputFileStream << "distanceTrue: " << distanceTrue << std::endl;

                maxApproximationFactor = std::max(maxApproximationFactor, distanceApproximate / distanceTrue);
            }

            // Output timing and MAF
            outputFileStream << "tAverageApproximate: " << tGNNS << " seconds" << std::endl;
            outputFileStream << "tAverageTrue: " << tTrue << " seconds" << std::endl;
            outputFileStream << "MAF: " << maxApproximationFactor << std::endl;
        }

        std::cout << "Finished GNNS." << std::endl;

        // Ask the user if they want to repeat with new files
        std::cout << "Do you want to repeat with new input.dat and query.dat? (y/n): ";
        std::cin >> repeatChoice;

        if(repeatChoice == 'y' || repeatChoice == 'Y') {
            std::cout << "Enter the path to the new dataset (-d): ";
            std::cin >> inputFile;

            std::cout << "Enter the path to the new query file (-q): ";
            std::cin >> queryFile;

            // Optional: clear the arguments and populate with new paths
            args.clear();
            args.emplace_back(argv[0]);
            args.emplace_back("-d");
            args.push_back(inputFile);
            args.emplace_back("-q");
            args.push_back(queryFile);
        }

    } while (repeatChoice == 'y' || repeatChoice == 'Y'); // Repeat if user chooses 'y' or 'Y'

    return 0;
}


