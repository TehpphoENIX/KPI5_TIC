#define HELP "  -s string -- examine string \n-f filename -- examine text file"

#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <fstream>

using std::cout;
using std::cin;
using std::string;
using std::unordered_map;

struct Row
{
    int count;
    double probability;
};

class Task
{
private:
    int m = 0;
    int n = 0;
    int b = 2;
    unordered_map<char, Row> table;
public:
    void getString(string text, int base) 
    {
        {
            n = text.size();
            b = base;
            unordered_map<char, Row>::iterator j;
            for (std::string::iterator i = text.begin(); i < text.end(); i++)
            {
                j = table.find(*i);
                if (j == table.end())
                {
                    table.insert({ *i,{1,0} });
                    m++;
                }
                else
                    table[*i].count++;
            }
        }
        for (unordered_map<char, Row>::iterator i = table.begin(); i != table.end(); i++)
            i->second.probability = ((double)i->second.count) / n;
    }
    void getFile(string filename, int base)
    {
        {
            b = base;
            std::ifstream file(filename);
            string line;
            unordered_map<char, Row>::iterator j;
            while (std::getline(file, line))
            {
                n += line.size();
                for (std::string::iterator i = line.begin(); i < line.end(); i++)
                {
                    j = table.find(*i);
                    if (j == table.end())
                    {
                        table.insert({ *i,{1,0} });
                        m++;
                    }
                    else
                        table[*i].count++;
                }
            }
        }
        for (unordered_map<char, Row>::iterator i = table.begin(); i != table.end(); i++)
            i->second.probability = ((double)i->second.count) / n;
    }

    void print()
    {
        cout << '\n';
        for (unordered_map<char, Row>::iterator i = table.begin(); i != table.end(); i++)
        {
            cout << "+---+--------+--------------+\n| ";
            cout << i->first << " | ";
            cout.width(6);
            cout << std::right << i->second.count << " | ";
            cout.width(12);
            cout.right;
            cout << i->second.probability << " |\n";
        }
        cout << "+---+--------+--------------+" << std::endl;
    }

    double entropy()
    {
        double e = 0;
        Row a;
        for (auto i = table.begin(); i != table.end(); i++)
        {
            e += i->second.probability * log2(i->second.probability) / log2(b);
        }
        return -e;
    }
    double maxEntropy() { return log2(m) / log2(b); }
    double quantity() { return entropy() * n; }
    double absoluteRedundancy() { return maxEntropy() - entropy(); }
    double relativeRedundancy() { return absoluteRedundancy() / maxEntropy(); }
};

int main()
{
    system("chcp 855");
    cout << "\n\n" << std::endl;
    string input;
    cout << "Mode: ";
    cin >> input;
    Task A;
    if (input == "/h")
    {
        cout << HELP << std::endl;
        system("pause");
        return 0;
    }
    else if (input == "/s")
    {
        cout << "String: ";
        cin.get();
        getline(cin, input);
        A.getString(input, 2);
    }
    else if (input == "/f")
    {
        cout << "Filename: ";
        cin.get();
        getline(cin, input);
        A.getFile(input, 2);
    }
    else
        return 1;

    A.print();
    cout << "            Entropy: " << A.entropy() << '\n';
    cout << "        Max Entropy: " << A.maxEntropy() << '\n';
    cout << "           Quantity: " << A.quantity() << '\n';
    cout << "Absolute Redundancy: " << A.absoluteRedundancy() << '\n';
    cout << "Relative Redundancy: " << A.relativeRedundancy() << std::endl;
    system("pause");
}