//
//  main.cpp
//  Huffman Code
//
//  Created by Alexander O'Hare on 5/3/17.
//  Copyright © 2017 Alexander O'Hare. All rights reserved.
//

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <sys/stat.h>
using namespace std;

bool FileExists(const string& filename)
{
	struct stat buf;
	if (stat(filename.c_str(), &buf) != -1)
	{
		return true;
	}
	return false;
}
// Huffman Tree Node Struct
struct HuffmanNode {
	char c;
	int freq;
	HuffmanNode *left, *right;	// Children
	
	// Constructors
	HuffmanNode(char data, int f) {
		left = nullptr;
		right = nullptr;
		c = data;
		freq = f;
	}
	HuffmanNode(int f) {
		left = nullptr;
		right = nullptr;
		c = NULL;
		freq = f;
	}
};
map<char, int> CreateMap(string filePath) {
	map<char, int> m;
	ofstream output;
	ifstream input(filePath);
	stringstream ss;
	
	// Read file into stringstream to include spaces and newlines
	ss << input.rdbuf();
	// Turn stringstream into string then create map of frequencies
	string s(ss.str());
	for (int i = 0 ; i < s.size(); i++) {
		m[(s[i])]++;
	}
	input.close();
	return m;
}
HuffmanNode* CreateHuffmanTree(map<char, int> m) {
	map<char, int>::iterator it;
	HuffmanNode *parent = nullptr;
	vector<HuffmanNode*> unparentedNodes;
	
	// Iterate through map and create nodes for each character
	for ( it = m.begin(); it != m.end(); it++ )
	{
		// it->first == key(character)	// it->second == value(frequency)
		unparentedNodes.push_back(new HuffmanNode(it->first, it->second));
	}
	// Iterate through nodes and create parents of lowest value
	// until only one un-parented node exists
	while (unparentedNodes.size() > 1) {
		// Find lowest value pair
		int			p1	   = 0,
					p2     = 1;
		HuffmanNode *node1 = unparentedNodes[0],
					*node2 = unparentedNodes[1];
		for (int i = 2; i < unparentedNodes.size(); i++) {
			if (node1->freq < node2->freq) {
				if(unparentedNodes[i]->freq < node2->freq) {
					node2 = unparentedNodes[i];
					p2 = i;
				}
			} else {
				if(unparentedNodes[i]->freq < node1->freq) {
					node1 = unparentedNodes[i];
					p1 = i;
				}
			}
		}
		
		// Create parent node combining values, add parent to vector, remove children and clear nodes
		parent = new HuffmanNode((node1->freq + node2->freq));
		parent->left = node1;
		parent->right = node2;
		if (p1 > p2) {
			unparentedNodes.erase(unparentedNodes.begin() + p1);
			unparentedNodes.erase(unparentedNodes.begin() + p2);
		}
		else {
			unparentedNodes.erase(unparentedNodes.begin() + p2);
			unparentedNodes.erase(unparentedNodes.begin() + p1);
		}
		unparentedNodes.push_back(parent);
	}
	
	// Return the last node (root)
	return parent;
}

void Traverse(string b,HuffmanNode* root, map<char, string> &m) {
	if(root != nullptr) {
		if (root->c) {
			// Character found, add bitset to map
			m[root->c] = b;
			delete root;
			root = nullptr;
			return;
		}
		// Left, add 0 to bitset
		// Right, add 1 to bitset
		Traverse(b + "0", root->left, m);
		Traverse(b + "1", root->right, m);
		delete root;
		root = nullptr;
	}
}
void CreateCode(HuffmanNode* root, map<char, string> &m) {
	string b;
	// Traverse the tree, every left is 0, every right is 1
	// When c-value is found, insert bits into map
	Traverse(b, root, m);
}

// Create encoded header for file
string CreateHeader(map<char, string> m) {
	map<char, string>::iterator it;
	string encode;
	stringstream ss;
	bitset<8> breaker = 00000000;
	// Turn every character (including the binary integers) into 8-bit binary characters
	for (it = m.begin(); it != m.end(); it++) {
		// Put in character
		ss << bitset<8>(it->first);
		// Put in code
		for (int i = 0; i < it->second.size(); i++) {
			ss << bitset<8>(it->second[i]);
		}
		// Put in breaker
		ss << breaker;
	}
	// Put in final breaker
	ss << breaker;
	string s(ss.str());
	return s;
}

// Encode the file @ filePath and put compressed copy in same directory with ext .cmp
void HuffmanEncode(string filePath, map<char, string> m) {
	if (!FileExists(filePath)) {
		cout << "The path you entered isn't valid.\n\n";
		return;
	}
	ifstream input;
	ofstream output;
	stringstream ss;
	string outfilePath;
	
	// Read file into stringstream to include spaces and newlines
	input.open(filePath);
	ss << input.rdbuf();
	input.close();
	// Turn stringstream into string then for each character, put it into encode
	string s(ss.str());
	string encode = "";
	encode += CreateHeader(m);
	for (int i = 0 ; i < s.size(); i++)
		encode += m[s[i]];
	stringstream ss2(encode);
	encode.clear();
	
	// Get every 8 bits and turn into byte, then store in another array
	while (ss2.good()) {
		bitset<8> bits;
		ss2 >> bits;
		char c = char(bits.to_ulong());
		encode += c;
	}
	
	// Output product to file
	outfilePath = filePath;
	outfilePath.erase(filePath.find_last_of("."), string::npos);
	outfilePath += "com.txt";
	output.open(outfilePath);
	output << encode;
	output.close();
	cout << filePath << " has been compressed to the path " << outfilePath << ".\n";
}

void HuffmanDecode(string filePath) {
	ifstream input;
	ofstream output;
	stringstream ss;
	
	// Read file into stringstream all at once
	input.open(filePath);
	ss << input.rdbuf();
	input.close();
	
	// Create map from header
	map<char, string> m;
	bool headDone = false;
	bool breaked = true;
	bool first = true;
	char key;
	string encode;
	while (!headDone) {
		char byte;
		bitset<8> checkBits = 00000000;
		ss.get(byte);
		
		// Check for breaker characters
		if (byte == (char)checkBits.to_ulong()) {
			if (breaked) {
				headDone = true;
				break;
			}
			m[key] = encode;
			encode = "";
			breaked = true;
			first = true;
		} else
			breaked = false;
		
		// First byte is character, rest are the encode
		if (!breaked) {
			if (first) {
				key = byte;
				first = false;
			}
			else
				encode += byte;
		}
	}
	string s(ss.str());
	// Create reverse map to check characters
	map<char, string>::iterator it;
	// Remove buffer characters from original map
//	it = m.find('\xbb');
//	m.erase(it);
//	it = m.find('\xbf');
//	m.erase(it);
//	it = m.find('\xef');
//	m.erase(it);
	
	map<string, char> rm;
	for (it = m.begin(); it != m.end(); it++) {
		rm[it->second] = it->first;
	}
	
	// Get all characters and turn to bits
	string bits = "";
	while (ss.good()) {
		char byte;
		ss.get(byte);
		bitset<8> set = bitset<8>(byte);
		bits += set.to_string();
	}
	
	
	
	// Iterate through the string and check to see if matches code
	// When matched, removed saved bits and add character to stringstream
	
	string decoded;
	string check;
	int decodeCount = 0;
	for (int i = 0; i < bits.size(); i++) {
		check += bits[i];
		// If found
		if (rm.find(check) != rm.end()) {
			decoded += rm[check];
			decodeCount++;
			check = "";
		}
	}
	
	// Output decoded product
	filePath.erase(filePath.find_last_of("com."), string::npos);
	filePath += ".txt";
	output.open(filePath);
	output << decoded;
	output.close();
}

// Print the frequencies of each character (FOR TESTING)
void PrintFrequencies(map<char, int> m) {
	map<char, int>::iterator it;
	cout << "The frequencies are the following: " << endl;
	for(it = m.begin(); it != m.end(); it++) {
		cout << it->first << " " << it->second << endl;
	}
	cout << endl;
}

// Print the encoded codes for each character (FOR TESTING)
void PrintCodes(map<char,string> m) {
	map<char, string>::iterator it;
	cout << "The codes are the following: " << endl;
	for(it = m.begin(); it != m.end(); it++) {
		cout << it->first << " " << it->second << endl;
	}
	cout << endl;
}

// Print the characters of the shorted codes (FOR TESTING)
void PrintCharMap(map<char, string> m) {
	map<char, string>::iterator it;
	cout << "The characters are the following: " << endl;
	for(it = m.begin(); it != m.end(); it++) {
		int i = stoi(it->second, nullptr, 2);
		cout << (char)i << " " << it->second << endl;
	}
	cout << endl;
}
void CompressFile() {
	string filePath;
	map<char, string> encodedValues;
	cout << "Enter the full file path for the file to be compressed. By DEFAULT the compressed file will be put in the same directory as a .cmp file.\n\n";
	cin >> filePath;
	cout << endl;
	CreateCode(CreateHuffmanTree(CreateMap(filePath)), encodedValues);
	HuffmanEncode(filePath, encodedValues);
	return;
}
void DecompressFile() {
	string filePath;
	cout << "Enter the full file path for the file to be decompressed. by DEFAULT the decompressed file will be put in the same directory as a text file.\n\n";
	cin >> filePath;
	cout << endl;
	HuffmanDecode(filePath);
	return;
}
int DisplayOptions() {
	int choice;
	cout << "Select one of the following option by pressing the associated number:\n\n";
	cout << "1) Compress a file" << endl << "2) Decompress a file" << endl << "3) Exit the application\n\n";
	cin >> choice;
	cout << endl;
	return choice;
}
int main()
{
	wcout << "This is a compression/decompression application. Copyright 2017 Alexander O'Hare" << endl;
	cout << string(50, '-') << endl;
	bool running = true;
	while (running) {
		int choice;
		cout << "Select one of the following option by pressing the associated number:\n\n";
		cout << "1) Compress a file" << endl << "2) Decompress a file" << endl << "3) Exit the application\n\n";
		cin >> choice;
		cout << endl;
		switch (choice) {
			case 1:
				CompressFile();
				break;
			case 2:
				DecompressFile();
				break;
			case 3:
				running = false;
				break;
			default:
				break;
		}
	}
	return 0;
}
