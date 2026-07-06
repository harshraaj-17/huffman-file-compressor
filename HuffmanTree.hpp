#ifndef HUFFMAN_TREE_HPP
#define HUFFMAN_TREE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>

// Node structure for the Huffman Tree
struct HuffmanNode {
    uint8_t data;
    uint64_t frequency;
    uint8_t min_byte; // Tie-breaker for deterministic tree building
    std::unique_ptr<HuffmanNode> left;
    std::unique_ptr<HuffmanNode> right;

    // Constructor for leaf nodes
    HuffmanNode(uint8_t data, uint64_t frequency)
        : data(data), frequency(frequency), min_byte(data), left(nullptr), right(nullptr) {}

    // Constructor for internal nodes
    HuffmanNode(uint8_t data, uint64_t frequency, std::unique_ptr<HuffmanNode> left, std::unique_ptr<HuffmanNode> right)
        : data(data), frequency(frequency), min_byte(std::min(left->min_byte, right->min_byte)), left(std::move(left)), right(std::move(right)) {}
};

// Comparator for the Min-Heap
struct CompareNode {
    bool operator()(const std::unique_ptr<HuffmanNode>& lhs, const std::unique_ptr<HuffmanNode>& rhs) const {
        if (lhs->frequency == rhs->frequency) {
            // Strict tie-breaker to ensure deterministic tree building regardless of unordered_map iteration order
            return lhs->min_byte > rhs->min_byte; 
        }
        return lhs->frequency > rhs->frequency;
    }
};

class HuffmanTree {
public:
    HuffmanTree() = default;

    // 1. Count the frequency of every byte in the input file
    bool buildFrequencyMap(const std::string& inputFilename);

    // 2. Build the Huffman Tree using a Min-Heap
    void buildTree();

    // 3. Generate variable-length prefix codes
    void generateCodes();

    // 4. Compress the input file to output file (Phase 2)
    bool compress(const std::string& inputFilename, const std::string& outputFilename);

    // 5. Decompress the input file to output file (Phase 3)
    bool decompress(const std::string& inputFilename, const std::string& outputFilename);

    // Print the generated codes (for verification)
    void printCodes() const;

    // Getters for potential external usage/verification
    const std::unordered_map<uint8_t, uint64_t>& getFrequencies() const { return frequencies; }
    const std::unordered_map<uint8_t, std::string>& getCodes() const { return codes; }

private:
    // Helper function to recursively generate codes
    void generateCodesRecursive(const HuffmanNode* node, const std::string& currentCode);

    std::unordered_map<uint8_t, uint64_t> frequencies;
    std::unique_ptr<HuffmanNode> root;
    std::unordered_map<uint8_t, std::string> codes;
};

#endif // HUFFMAN_TREE_HPP
