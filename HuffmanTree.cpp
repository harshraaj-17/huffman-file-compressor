#include "HuffmanTree.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>

bool HuffmanTree::buildFrequencyMap(const std::string& inputFilename) {
    // Standard C++ stream in binary mode
    std::ifstream inputFile(inputFilename, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << inputFilename << std::endl;
        return false;
    }

    frequencies.clear();
    char byte;
    // Read byte by byte and populate frequency map
    while (inputFile.get(byte)) {
        frequencies[static_cast<uint8_t>(byte)]++;
    }

    inputFile.close();
    return true;
}

void HuffmanTree::buildTree() {
    if (frequencies.empty()) {
        root = nullptr;
        return;
    }

    std::vector<std::unique_ptr<HuffmanNode>> minHeap;

    // Populate the heap vector
    for (const auto& pair : frequencies) {
        minHeap.push_back(std::make_unique<HuffmanNode>(pair.first, pair.second));
    }

    // Convert vector into a min-heap
    std::make_heap(minHeap.begin(), minHeap.end(), CompareNode());

    // Handle edge case where file contains only one unique character
    if (minHeap.size() == 1) {
        auto left = std::move(minHeap.front());
        minHeap.clear();
        root = std::make_unique<HuffmanNode>(0, left->frequency, std::move(left), nullptr);
        return;
    }

    // Build the tree by combining the two lowest frequency nodes
    while (minHeap.size() > 1) {
        // Extract first minimum
        std::pop_heap(minHeap.begin(), minHeap.end(), CompareNode());
        auto left = std::move(minHeap.back());
        minHeap.pop_back();

        // Extract second minimum
        std::pop_heap(minHeap.begin(), minHeap.end(), CompareNode());
        auto right = std::move(minHeap.back());
        minHeap.pop_back();

        // Create internal node with combined frequency
        uint64_t combinedFreq = left->frequency + right->frequency;
        auto parent = std::make_unique<HuffmanNode>(0, combinedFreq, std::move(left), std::move(right));

        // Push new parent node back into heap
        minHeap.push_back(std::move(parent));
        std::push_heap(minHeap.begin(), minHeap.end(), CompareNode());
    }

    // The remaining node is the root
    if (!minHeap.empty()) {
        root = std::move(minHeap.front());
        minHeap.clear();
    }
}

void HuffmanTree::generateCodes() {
    codes.clear();
    if (root) {
        generateCodesRecursive(root.get(), "");
    }
}

void HuffmanTree::generateCodesRecursive(const HuffmanNode* node, const std::string& currentCode) {
    if (!node) {
        return;
    }

    // If it's a leaf node, assign the accumulated code
    if (!node->left && !node->right) {
        // Handle edge case for root only leaf
        if (currentCode.empty()) {
            codes[node->data] = "0";
        } else {
            codes[node->data] = currentCode;
        }
        return;
    }

    generateCodesRecursive(node->left.get(), currentCode + "0");
    generateCodesRecursive(node->right.get(), currentCode + "1");
}

void HuffmanTree::printCodes() const {
    std::cout << "Huffman Codes generated:\n";
    std::cout << "Byte (Hex)\tFreq\tCode\n";
    std::cout << "------------------------------------\n";
    for (const auto& pair : codes) {
        uint8_t byte = pair.first;
        const std::string& code = pair.second;
        uint64_t freq = frequencies.at(byte);
        
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) 
                  << std::dec << "\t\t" << freq << "\t" << code << "\n";
    }
}

bool HuffmanTree::compress(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream inFile(inputFilename, std::ios::binary);
    std::ofstream outFile(outputFilename, std::ios::binary);

    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open input file for compression.\n";
        return false;
    }
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file for compression.\n";
        return false;
    }

    // 1. Write the header
    // Header format: [number of unique bytes (uint16_t)] followed by [byte (uint8_t)][frequency (uint64_t)]...
    uint16_t uniqueBytes = static_cast<uint16_t>(frequencies.size());
    outFile.write(reinterpret_cast<const char*>(&uniqueBytes), sizeof(uniqueBytes));

    for (const auto& pair : frequencies) {
        uint8_t byte = pair.first;
        uint64_t freq = pair.second;
        outFile.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
        outFile.write(reinterpret_cast<const char*>(&freq), sizeof(freq));
    }

    // 2. Write the compressed bitstream
    uint8_t bitBuffer = 0;
    int bitCount = 0;

    char byte;
    while (inFile.get(byte)) {
        const std::string& code = codes[static_cast<uint8_t>(byte)];
        for (char bitChar : code) {
            if (bitChar == '1') {
                bitBuffer |= (1 << (7 - bitCount));
            }
            bitCount++;
            
            // If the buffer is full (8 bits), write it to the file
            if (bitCount == 8) {
                outFile.write(reinterpret_cast<const char*>(&bitBuffer), sizeof(bitBuffer));
                bitBuffer = 0;
                bitCount = 0;
            }
        }
    }

    // If there are remaining bits in the buffer, write them out
    if (bitCount > 0) {
        outFile.write(reinterpret_cast<const char*>(&bitBuffer), sizeof(bitBuffer));
    }

    inFile.close();
    outFile.close();
    return true;
}

bool HuffmanTree::decompress(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream inFile(inputFilename, std::ios::binary);
    std::ofstream outFile(outputFilename, std::ios::binary);

    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open input file for decompression.\n";
        return false;
    }
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file for decompression.\n";
        return false;
    }

    // 1. Read header and reconstruct frequency map
    uint16_t uniqueBytes;
    inFile.read(reinterpret_cast<char*>(&uniqueBytes), sizeof(uniqueBytes));

    frequencies.clear();
    uint64_t totalCharacters = 0;

    for (uint16_t i = 0; i < uniqueBytes; ++i) {
        uint8_t byte;
        uint64_t freq;
        inFile.read(reinterpret_cast<char*>(&byte), sizeof(byte));
        inFile.read(reinterpret_cast<char*>(&freq), sizeof(freq));
        frequencies[byte] = freq;
        totalCharacters += freq;
    }

    // 2. Rebuild the Huffman Tree based on the read frequencies
    buildTree();

    if (!root) {
        // Edge case: Empty file
        inFile.close();
        outFile.close();
        return true;
    }

    // 3. Decompress the bitstream
    HuffmanNode* currentNode = root.get();
    uint64_t decodedCount = 0;
    char byteBuffer;

    while (decodedCount < totalCharacters && inFile.get(byteBuffer)) {
        for (int i = 7; i >= 0; --i) {
            bool bit = (byteBuffer >> i) & 1;
            
            if (bit) {
                currentNode = currentNode->right.get();
            } else {
                currentNode = currentNode->left.get();
            }

            // If we hit a leaf node
            if (!currentNode->left && !currentNode->right) {
                outFile.put(static_cast<char>(currentNode->data));
                decodedCount++;
                currentNode = root.get();

                // Stop exactly when we've decoded the total original bytes
                // This prevents reading padding zeros as valid characters!
                if (decodedCount == totalCharacters) {
                    break;
                }
            }
        }
    }

    inFile.close();
    outFile.close();
    return true;
}
