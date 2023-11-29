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
#include "MRNGGraph.h"

int main(int argc, char** argv) {
    std::vector<std::string> args(argv, argv + argc);

    std::string inputFile, queryFile, outputFile;
    int number_of_images, image_size;
    int k = 50; // Number of nearest neighbors in graph
    int E = 30; // Number of expansions
    int R = 1; // Number of random restarts
    int N = 1;  // Number of nearest neighbors to search for
    int l = 20;  // Only for Search-on-Graph
    int mode = 0; // 1 for GNNS, 2 for MRNG

    char repeatChoice = 'n'; // to control the loop
    do {
        if (args.size() == 1) {  // Only mode provided, prompt for paths
            std::cout << "Please enter the mode (1 for GNNS, 2 for MRNG): ";
            std::cin >> mode;
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

        std::vector<std::vector<unsigned char>> query_set = read_mnist_images(queryFile, number_of_images, image_size);

        if (outputFile.empty()) {
            std::cout << "Enter the path for output file: ";
            std::cin >> outputFile;
        }

        std::ofstream outputFileStream(outputFile);
        if (!outputFileStream.is_open() || outputFileStream.fail()) {
            std::cerr << "Failed to open output file for writing." << std::endl;
            return 2;
        }

        double totalTAlgorithm = 0.0;
        double totalTTrue = 0.0;
        double totalMAF = 0.0;
        double maxApproximationFactor = 0.0;

        if (mode == 1) {

            std::vector<std::vector<unsigned char>> testset;

            int T = 10; // Number of greedy steps

            LSH lsh(dataset);
            //Hypercube cube(testset);
            std::cout << "Started building the k-NNG" << std::endl;
            Graph kNNG_L = buildKNNG(lsh, k, dataset.size());
            //Graph kNNG_L = buildKNNG_H(cube, k, dataset.size());

            std::cout << "Finished building the k-NNG." << std::endl;
            outputFileStream << "GNNS Results" << std::endl;

            for (int i = 0; i < 10; ++i) {
                outputFileStream << "\nQuery: " << i << std::endl;

                auto startTimeAlgorithm = std::chrono::high_resolution_clock::now();
                auto results = kNNG_L.GNNS(query_set[i], N, R, T, E);
                auto endTimeAlgorithm = std::chrono::high_resolution_clock::now();

                double tAlgorithm = std::chrono::duration<double, std::milli>(endTimeAlgorithm - startTimeAlgorithm).count() / 1000.0;

                auto startTimeTrue = std::chrono::high_resolution_clock::now();
                auto trueResults = trueNNearestNeighbors(dataset, query_set[i], N);
                auto endTimeTrue = std::chrono::high_resolution_clock::now();

                double tTrue = std::chrono::duration<double, std::milli>(endTimeTrue - startTimeTrue).count() / 1000.0;

                for (int j = 0; j < N; ++j) {
                    double distanceApproximate = results[j].second;
                    double distanceTrue = trueResults[j].second;
                    maxApproximationFactor = std::max(maxApproximationFactor, distanceApproximate / distanceTrue);
                }

                for (int j = 0; j < N; ++j) {
                    outputFileStream << "Nearest neighbor-" << j + 1 << ": " << results[j].first << std::endl;
                    outputFileStream << "distanceApproximate: " << results[j].second << std::endl;
                    outputFileStream << "distanceTrue: " << trueResults[j].second << std::endl;
                }

                totalTAlgorithm += tAlgorithm;
                totalTTrue += tTrue;
                totalMAF += maxApproximationFactor;

            }


        }   else if (mode == 2) {
            std::vector<std::vector<unsigned char>> testset;
            for (int i = 0; i < 3000; ++i) {
                testset.push_back(dataset[i]);
            }
            std::cout << "Started building the MRNG" << std::endl;
            MRNGGraph mrngGraph(testset, l, N);
            std::cout << "Finished building the MRNG." << std::endl;
            outputFileStream << "MRNG Results" << std::endl;


            for (int i = 0; i < 10; ++i) {
                outputFileStream << "\nQuery: " << i << std::endl;

                // Start time for MRNG algorithm
                auto startTimeAlgorithm = std::chrono::high_resolution_clock::now();
                auto results = mrngGraph.searchOnGraph(query_set[i], 0, N, l);
                auto endTimeAlgorithm = std::chrono::high_resolution_clock::now();

                // Calculate the time taken by the MRNG algorithm
                double tAlgorithm = std::chrono::duration<double, std::milli>(endTimeAlgorithm - startTimeAlgorithm).count() / 1000.0;

                // Start time for true nearest neighbors search
                auto startTimeTrue = std::chrono::high_resolution_clock::now();
                auto trueResults = trueNNearestNeighbors(dataset, query_set[i], N);
                auto endTimeTrue = std::chrono::high_resolution_clock::now();

                // Calculate the time taken by the true nearest neighbors search
                double tTrue = std::chrono::duration<double, std::milli>(endTimeTrue - startTimeTrue).count() / 1000.0;

                for (int j = 0; j < N; ++j) {
                    double distanceApproximate = results[j].second;
                    double distanceTrue = trueResults[j].second;
                    if (j==0) maxApproximationFactor = std::max(maxApproximationFactor, distanceApproximate / distanceTrue);
                }

                for (int j = 0; j < N; ++j) {
                    outputFileStream << "Nearest neighbor-" << j + 1 << ": " << results[j].first << std::endl;
                    outputFileStream << "distanceApproximate: " << results[j].second << std::endl;
                    outputFileStream << "distanceTrue: " << trueResults[j].second << std::endl;
                }

                totalTAlgorithm += tAlgorithm;
                totalTTrue += tTrue;

            }


        }

        // Calculate the average values for the 10 queries

        totalTAlgorithm /= 10;
        totalTTrue /= 10;

        outputFileStream << std::endl;
        outputFileStream << "tAverageApproximate: " << totalTAlgorithm << std::endl;
        outputFileStream << "tAverageTrue: " << totalTTrue << std::endl;
        outputFileStream << "MAF: " << maxApproximationFactor << std::endl;


        // Ask the user if they want to repeat with new files
        std::cout << "Do you want to repeat with new input.dat and query.dat? (y/n): ";
        std::cin >> repeatChoice;

        if(repeatChoice == 'y' || repeatChoice == 'Y') {
            std::cout << "Enter the path to the new dataset (-d): ";
            std::cin >> inputFile;

            std::cout << "Enter the path to the new query file (-q): ";
            std::cin >> queryFile;

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


