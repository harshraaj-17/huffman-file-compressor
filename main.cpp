#include "HuffmanTree.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    HuffmanTree huffman;

    std::cout << "Step 1: Reading file and calculating frequencies...\n";
    if (!huffman.buildFrequencyMap(filename)) {
        return 1;
    }

    std::cout << "Step 2: Building Huffman Tree...\n";
    huffman.buildTree();

    std::cout << "Step 3: Generating prefix codes...\n";
    huffman.generateCodes();

    std::cout << "Done. Displaying generated codes:\n";
    huffman.printCodes();

    std::string compressedFile = filename + ".huff";
    std::cout << "\nStep 4: Compressing file to " << compressedFile << "...\n";
    if (huffman.compress(filename, compressedFile)) {
        std::cout << "Compression successful!\n";
    } else {
        std::cerr << "Compression failed.\n";
        return 1;
    }

    std::string decompressedFile = filename + ".restored";
    std::cout << "\nStep 5: Decompressing file to " << decompressedFile << "...\n";
    
    // Create a totally separate HuffmanTree object to prove it reconstructs correctly from the file!
    HuffmanTree decoder;
    if (decoder.decompress(compressedFile, decompressedFile)) {
        std::cout << "Decompression successful!\n";
    } else {
        std::cerr << "Decompression failed.\n";
        return 1;
    }

    return 0;
}
