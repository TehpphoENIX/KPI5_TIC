// L2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <bitset>
#include <array>
#include <bitset>
#include <queue>


using std::cout;
using std::vector;
using std::ifstream;
using std::unordered_map;
using std::string;
using std::array;
using std::getline;

enum NodeType {
    leaf,
    branch
};
struct TreeNode
{
    bool type;
};
struct Leaf : TreeNode
{
    Leaf() { type = leaf; }
    Leaf(unsigned char name) 
    { 
        type = leaf; 
        this->name = name;
    }
    unsigned char name;
};
struct Branch : TreeNode
{
    Branch() { type = branch; }
    TreeNode* leftChild;
    TreeNode* rightChild;
};

bool comareFunc(const std::pair<char, unsigned long>& first, const std::pair<char, unsigned long>& second) { return (first.second > second.second) ? true : false; }

class PrefixCode
{
private:
    //dictionary
    TreeNode* root;
    unordered_map<unsigned char, string> dictionary;

    vector<unsigned char> code;
    unsigned long charCount;
public:
    std::array<double,2> encode(const vector<unsigned char> text)
    {
        //geting primary dictionary
        charCount = text.size();
        unordered_map<char, unsigned long> table;
        for (auto i : text)
        {
            auto searchResult = table.find(i);
            if (searchResult == table.end())
            {
                table.insert({ i,1 });
            }
            else
                table[i]++;
        }

        //creating final dictionary
        {
            struct HeapNode {
                TreeNode* node;
                unsigned long freq;
            };

            struct compare {

                bool operator()(HeapNode* l, HeapNode* r)

                {
                    return (l->freq > r->freq);
                }
            };

            std::priority_queue<HeapNode, std::queue<HeapNode>, compare> queue;
            for(auto item : table)
            {
                queue.push({ new Leaf(item.first), item.second });
            }

            while (queue.size() > 1)
            {
               HeapNode first, second;
                first = queue.top();
                queue.pop();
                second = queue.top();
                queue.pop();
                Branch* branch = new Branch();
                branch->leftChild = first.node;
                branch->rightChild = second.node;
                queue.push({ branch, first.freq + second.freq });
            }
        }

        unordered_map<unsigned char, string> symbolCodes = getDictionary();

        vector<string> buffer;
        for (auto i : text)
        {
            buffer.push_back(symbolCodes[i]);
        }

        code = compressToBitCode(buffer);

        double entropy = 0;
        double avgCodeLength = 0;
        for (auto item : table)
        {
            double probability = ((double)item.second) / charCount;
            avgCodeLength += probability * symbolCodes[item.first].length();
            entropy += probability * log2(probability);
        }

        return { -entropy, avgCodeLength };
    }
    const vector<unsigned char> decode()
    {
        unsigned long countdown = charCount;
        vector<unsigned char> output;
        TreeNode* pointer = root;
        for (const unsigned char item : code)
        {
            unsigned char copy = item;
            for (unsigned short i = 0; i < 8;)
            {
                if (pointer->type == branch)
                {
                    if (copy & 128)
                    {//1
                        pointer = ((Branch*)pointer)->rightChild;
                    }
                    else
                    {//0
                        pointer = ((Branch*)pointer)->leftChild;
                    }
                    copy <<= 1;
                    i++;
                }
                else
                {
                    output.push_back(((Leaf*)pointer)->name);
                    countdown--;
                    if (countdown < 1)
                        return output;
                    pointer = root;
                }
            }
        }
        return output;
    }
    int open(string filepath)
    {
        std::list<bool> bitcode;
        std::list<unsigned char> charArray;
        {
            unsigned long numberOfNodes;
            vector<unsigned char> trueBitcode;
            unsigned long numberOfLeafs = 0;

            ifstream file(filepath, std::ios::binary);
            //get file size
            file.seekg(0, std::ios::end);
            size_t size = (size_t)file.tellg();
            file.seekg(0, std::ios::beg);
            //read number of nodes
            file.read((char*)&numberOfNodes, sizeof(numberOfNodes));
            //read bitcode
            {
                size_t block_size = (size_t)ceil((double)numberOfNodes / 8);
                char* buffer = new char[block_size];
                file.read(buffer, block_size);
                for (size_t i = 0; i < block_size; i++)
                {
                    trueBitcode.push_back(buffer[i]);
                }
                delete[] buffer;
                size -= block_size;
            }
            //get number of leafs + decrypt bitcode
            unsigned long j = 0;
            for (const unsigned char item : trueBitcode)
            {
                unsigned char copy = item;
                for (unsigned short i = 0; i < 8; i++)
                {
                    if (copy & 128)
                    {
                        bitcode.push_back(branch);
                    }
                    else
                    {
                        bitcode.push_back(leaf);
                        numberOfLeafs++;
                    }
                    copy <<= 1;
                    j++;
                    if (j >= numberOfNodes) break;
                }
                if (j >= numberOfNodes) break;
            }
            //read leaf values
            {
                size_t block_size = numberOfLeafs;
                char* buffer = new char[block_size];
                file.read(buffer, block_size);
                for (size_t i = 0; i < size; i++)
                {
                    charArray.push_back(buffer[i]);
                }
                delete[] buffer;
                size -= block_size;
            }
            //read 
            file.read((char*)&charCount, sizeof(charCount));
            //read code
            size -= sizeof(numberOfNodes) + sizeof(charCount);
            {
                char* buffer = new char[size];
                file.read(buffer, size);
                for (size_t i = 0; i < size; i++)
                {
                    code.push_back(buffer[i]);
                }
                delete[] buffer;
            }
            file.close();
        }
        createDictionary(&bitcode, &charArray, &root);
        return 0;
    }
    int save(string filepath)
    {
        unsigned long numberOfNodes;
        vector<unsigned char> charArray;
        vector<unsigned char> trueBitcode;
        {
            string bitcode;
            compressTree(&bitcode, &charArray, root);
            trueBitcode = compressToBitCode({ bitcode });
            numberOfNodes = bitcode.length();
        }
        std::ofstream file(filepath, std::ios::binary);
        file.write((char*)&numberOfNodes, sizeof(numberOfNodes));
        file.write((char*)&trueBitcode[0], trueBitcode.size());
        file.write((char*)&charArray[0], charArray.size());
        file.write((char*)&charCount, sizeof(charCount));
        file.write((char*)&code[0], code.size());
        file.close();
        return 0;
    }
    unordered_map<unsigned char, string> getDictionary()
    {
        if (dictionary.size() != 0)
            return dictionary;
        else
            return dictionary = findcode(root);
    }
    vector<unsigned char> getCode()
    {
        return code;
    }
private:
    int createDictionary(vector<std::pair<char, unsigned long>> array, unsigned long charCount, TreeNode** pointer)
    {
        if (array.size() == 1)
        {
            *pointer = new Leaf;
            ((Leaf*)(*pointer))->name = array[0].first;
            //symbols.insert({ array[0].first,((Leaf*)(*pointer)) });
            return 0;
        }
        else
        {
            vector<std::pair<char, unsigned long>> arrayright;
            unsigned long size = 0;
            for (auto it = array.begin(); it != array.end(); it++)
            {
                if(it == array.begin() || size + it->second <= charCount / 2)
                {
                    arrayright.push_back(*it);
                    size += it->second;
                }
                else
                {
                    *pointer = new Branch;
                    
                    return createDictionary(arrayright, size, &(((Branch*)(*pointer))->rightChild)) &
                        createDictionary(vector<std::pair<char, unsigned long>>(it, array.end()), charCount - size, &(((Branch*)(*pointer))->leftChild));
                }
            }
        }
        return 1;
    }
    int createDictionary(std::list<bool>* bitcode, std::list<unsigned char>* charArray, TreeNode** pointer)
    {
        if (bitcode->front() == leaf)
        {
            *pointer = new Leaf;
            ((Leaf*)(*pointer))->name = charArray->front();
            charArray->pop_front();
            //symbols.insert({ array[0].first,((Leaf*)(*pointer)) });
            bitcode->pop_front();
            return 0;
        }
        else
        {
            bitcode->pop_front();
            *pointer = new Branch;

            return createDictionary(bitcode, charArray, &(((Branch*)(*pointer))->leftChild)) &
                createDictionary(bitcode, charArray, &(((Branch*)(*pointer))->rightChild));
        }
        return 1;
    }
    unordered_map<unsigned char, string> findcode(TreeNode* pointer, string partialCode="")
    {
        if (pointer == nullptr)
            throw std::bad_exception();
        if (pointer->type == branch)
        {

            unordered_map<unsigned char, string> rightResult = findcode(((Branch*)pointer)->rightChild, partialCode + "1"),
                leftResult = findcode(((Branch*)pointer)->leftChild, partialCode + "0");

            rightResult.insert(leftResult.begin(), leftResult.end());
            return  rightResult;
        }
        else
        {
            unordered_map<unsigned char, string> buffer;
            buffer.insert({ ((Leaf*)pointer)->name,partialCode });
            return buffer;
        }
        return unordered_map<unsigned char, string>();
    }
    vector<unsigned char> compressToBitCode(vector<string> input)
    {
        vector<unsigned char> output;
        unsigned char codeBuffer = 0;
        unsigned short i = 0;
        for (auto item : input)
        {
            for (char jtem : item)
            {
                if (i > 7)
                {
                    output.push_back(codeBuffer);
                    codeBuffer = 0;
                    i = 0;
                }
                if (jtem == '1')
                    codeBuffer = (codeBuffer << 1) | 1;
                else
                    codeBuffer <<= 1;
                i++;
            }
        }
        for (; i < 8; i++)
            codeBuffer <<= 1;
        output.push_back(codeBuffer);
        return output;
    }
    int compressTree(string* bitcode, vector<unsigned char>* charArray, TreeNode* pointer)
    {
        if (pointer == nullptr)
            throw std::bad_exception();
        if (pointer->type == branch)
        {
            bitcode->push_back('1');
            return compressTree(bitcode, charArray, ((Branch*)pointer)->leftChild) | compressTree(bitcode, charArray, ((Branch*)pointer)->rightChild);
        }
        else
        {
            charArray->push_back(((Leaf*)pointer)->name);
            bitcode->push_back('0');
        }
    }
};

// {mode}{input} [/a]
int main(int argc, char* argv[])
{
    system("chcp 855");
    PrefixCode code;
    std::list<string> params;

    for (int i = 1; i < argc; i++)
    {
        params.push_back(string(argv[i]));
    }

    if (params.front() == "/e")
    {
        params.pop_front();
        vector<unsigned char> buffer;
        if (params.empty())
        {
            return -1;
        }
        else if (params.front() == "/s")
        {
            params.pop_front();
            cout << "input string : \n";
            string str;
            getline(std::cin, str);
            buffer = vector<unsigned char>(str.begin(), str.end());
        }
        else if (params.front() == "/f")
        {
            params.pop_front();
            cout << "input filepath : \n";
            string str;
            getline(std::cin, str);
            ifstream file(str, std::ios::binary);
            std::istreambuf_iterator<char> start(file), eos;
            buffer = vector<unsigned char>(start, eos);
        }

        std::array<double, 2> array = code.encode(buffer);
        cout << "encrypted\n";


        if (!params.empty() && params.front() == "/a")
        {
            params.pop_front();
            cout <<
                "   entropy = " << array[0] <<
                "\n   avarage code length = " << array[1] <<
                "\n   compresion coeficient (8 bit code) = " << 8 / array[1] <<
                "\n   compresion coeficient (dictionary) = " << ceil(log2(code.getDictionary().size())) / array[1] <<
                "\n   efficiency coeficient = " << array[0] / array[1] << std::endl;
        }
        if (!params.empty() && params.front() == "/row")
        {
            cout << "Dictionary:\n";
            for (auto item : code.getDictionary())
            {
                cout << item.first << " " << item.second << "\n";
            }
            cout << "Row view:\n";
            for (auto item : code.getCode())
            {
                std::bitset<8> bitview = item;
                cout << bitview;
            }
            cout << std::endl;
        }
        if (!params.empty() && params.front() == "/save")
        {
            params.pop_front();
            cout << "input filepath : \n";
            string str;
            getline(std::cin, str);
            code.save(str);
        }

    }
    else if (params.front() == "/d")
    {
        params.pop_front();
        cout << "input filepath : \n";
        string str;
        getline(std::cin, str);
        code.open(str);
        vector<unsigned char> buffer = code.decode();
        cout << "decoded\n";

        if (!params.empty() && params.front() == "/a")
        {
            params.pop_front();
            cout << std::endl;
            cout << "Contents:\n";
            for (auto item : buffer)
            {
                cout << item;
            }
            cout << std::endl;
        }
        if (!params.empty() && params.front() == "/row")
        {
            cout << "Dictionary:\n";
            for (auto item : code.getDictionary())
            {
                cout << item.first << " " << item.second << "\n";
            }
            cout << "Row view:\n";
            for (auto item : code.getCode())
            {
                std::bitset<8> bitview = item;
                cout << bitview;
            }
            cout << std::endl;
        }
        if (!params.empty() && params.front() == "/save")
        {
            params.pop_front();
            cout << "input filepath : \n";
            getline(std::cin, str);
            std::ofstream file(str, std::ios::binary);
            file.write((char*)(&buffer[0]), buffer.size());
        }

    }
    else if (params.front() == "/h")
    {
    cout << "{/e, /d, /h} {/s, /f} [/a] [/row] [/save]\n" <<
        "/e -- encrypt file\n" <<
        "/d -- decrypt file\n" <<
        "/h -- get help\n"
        "/s -- get input from string (only add when /e selected)\n" <<
        "/f -- get input from file (only add when /e selected)\n" <<
        "/a -- get additional info\n" <<
        "/row -- show encryption data"
        "/save -- save output to a file" << std::endl;
    }
    else
    {
        return -1;
    }

    //system("pause");
}