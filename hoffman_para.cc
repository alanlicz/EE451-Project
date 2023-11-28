#include <iostream>
#include <queue>
#include <unordered_map>

#include "file_reader.h"

using namespace std;

// A Huffman tree node
struct Node {
    char ch;
    int freq;
    Node *left, *right;

    Node(char ch, int freq, Node *left = nullptr, Node *right = nullptr)
        : ch(ch), freq(freq), left(left), right(right) {}
};

// Comparison object to be used to order the heap
struct Compare {
    bool operator()(Node *l, Node *r) { return l->freq > r->freq; }
};

// Function to encode the input string
void encode(Node *root, string str, unordered_map<char, string> &huffmanCode) {
    if (root == nullptr) return;

    if (!root->left && !root->right) {
        huffmanCode[root->ch] = str;
    }

    encode(root->left, str + "0", huffmanCode);
    encode(root->right, str + "1", huffmanCode);
}

// Function to decode the encoded string
void decode(Node *root, int &index, string str) {
    if (root == nullptr) {
        return;
    }

    if (!root->left && !root->right) {
        cout << root->ch;
        return;
    }

    index++;

    if (str[index] == '0')
        decode(root->left, index, str);
    else
        decode(root->right, index, str);
}

// Main function to build the Huffman Tree and decode given input text
void buildHuffmanTree(string text) {
    // Count frequency of appearance of each character and store it in a map
    unordered_map<char, int> freq;
    int n = text.length();

// Parallel frequency count
#pragma omp parallel
    {
        unordered_map<char, int> freq_private;
#pragma omp for nowait
        for (int i = 0; i < n; ++i) {
            freq_private[text[i]]++;
        }

#pragma omp critical
        for (auto &pair : freq_private) {
            freq[pair.first] += pair.second;
        }
    }

    // Create a priority queue to store live nodes of the Huffman tree
    priority_queue<Node *, vector<Node *>, Compare> pq;

    // Create a leaf node for each character and add it to the priority queue
    for (auto pair : freq) {
        pq.push(new Node(pair.first, pair.second));
    }

    // Iterate until there is more than one node in the queue
    while (pq.size() != 1) {
        // Remove the two nodes of highest priority (lowest frequency) from the
        // queue
        Node *left = pq.top();
        pq.pop();
        Node *right = pq.top();
        pq.pop();

        // Create a new internal node with these two nodes as children and with
        // frequency equal to the sum of the two nodes' frequencies. Add the new
        // node to the priority queue
        int sum = left->freq + right->freq;
        pq.push(new Node('\0', sum, left, right));
    }

    // The root of the tree is the only node in the queue now
    Node *root = pq.top();

    // Traverse the Huffman Tree and store Huffman Codes in a map
    unordered_map<char, string> huffmanCode;
    encode(root, "", huffmanCode);

    cout << "Huffman Codes are :\n";
    for (auto pair : huffmanCode) {
        cout << pair.first << " " << pair.second << "\n";
    }

    cout << "\nOriginal string was :\n" << text << "\n";

    // Parallel encoding
    string str = "";
#pragma omp parallel
    {
        string local_str;
#pragma omp for nowait
        for (int i = 0; i < n; ++i) {
            local_str += huffmanCode[text[i]];
        }

#pragma omp critical
        str += local_str;
    }

    cout << "\nEncoded string is :\n" << str << "\n";

    // Decode the encoded string
    int index = -1;
    cout << "\nDecoded string is: \n";
    while (index < (int)str.size() - 2) {
        decode(root, index, str);
    }
    cout << endl;
}

// Driver code
int main() {
    FileReader reader;
    std::string input;
    try {
        input =
            reader.readFile("text_file.txt");  // Replace with your file name
    } catch (const std::runtime_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    // string text = "Huffman coding is a data compression algorithm.";
    struct timespec start, stop;
    double time;

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        perror("clock gettime");
    }
    time = (stop.tv_sec - start.tv_sec) +
           (double)(stop.tv_nsec - start.tv_nsec) / 1e9;

    buildHuffmanTree(input);

    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
        perror("clock gettime");
    }
    time = (stop.tv_sec - start.tv_sec) +
           (double)(stop.tv_nsec - start.tv_nsec) / 1e9;

    std::cout << "Time: " << time << std::endl;

    return 0;
}
