//
// Created by test on 10/12/2023.
//
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <vector>

#include "mnist.h"


// This function is used to reverse the byte order of an integer.
// This is necessary because the MNIST format is in Big Endian format.
auto reverseInt = [](int i) {
    unsigned char c1, c2, c3, c4;
    c1 = i & 255;
    c2 = (i >> 8) & 255;
    c3 = (i >> 16) & 255;
    c4 = (i >> 24) & 255;
    return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
};

// Function to read and process MNIST image data
std::vector<std::vector<unsigned char>> read_mnist_images(const std::string& full_path, int& number_of_images, int& image_size) {

    // std::cout << "Reading MNIST images from `" << full_path << "`..." << std::endl;

    // Open the MNIST image file in binary mode
    std::ifstream file(full_path, std::ios::binary);

    if (file.is_open()) {

        int magic_number = 0, n_rows = 0, n_cols = 0;

        // Read meta-information from the MNIST image file
        file.read(reinterpret_cast<char*>(&magic_number), sizeof(magic_number));
        magic_number = reverseInt(magic_number);

        // 2051 is the magic number for MNIST image files.
        if (magic_number != 2051)
            throw std::runtime_error("Not valid MNIST file!");

        // Reading number of images, number of rows, and number of columns
        file.read(reinterpret_cast<char*>(&number_of_images), sizeof(number_of_images));
        number_of_images = reverseInt(number_of_images);
        file.read(reinterpret_cast<char*>(&n_rows), sizeof(n_rows));
        n_rows = reverseInt(n_rows);
        file.read(reinterpret_cast<char*>(&n_cols), sizeof(n_cols));
        n_cols = reverseInt(n_cols);

        // Calculate image size
        image_size = n_rows * n_cols;

        // Create a 2D vector to hold the image dataset
        std::vector<std::vector<unsigned char>> dataset(number_of_images, std::vector<unsigned char>(image_size));
        // Read image data into the dataset
        for (int i = 0; i < number_of_images; i++) {
            file.read(reinterpret_cast<char*>(&dataset[i][0]), image_size);
        }
        return dataset;
    }
    else {
        throw std::runtime_error("Could not open file `" + full_path + "`!");
    }
}

// Helper function to print an image from the MNIST dataset
void print_image(const std::vector<unsigned char>& image, int width, int height) {
    // Loop through each pixel of the image and print it to the console
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            std::cout << static_cast<int>(image[i * width + j]) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

}
