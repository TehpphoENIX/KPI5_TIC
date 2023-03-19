#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <bitset>
#include <random>
#include <memory>
#include <list>

#define PRINT_THING if (extended)\
    {\
        for (auto& item : h.get())\
        {\
            cout << item << ' ';\
        }\
    }\
    else\
    {\
        for (auto& item : h.get7())\
        {\
            cout << item << ' ';\
        }\
    }\
    cout << std::endl;\

using std::vector;
using std::bitset;
using std::cout;

class HamingCode8
{
private:
    const bool extended;
    vector<bitset<8>> codedMessage;
public:
    HamingCode8(bool extended, vector<unsigned char> message = {}):
        extended(extended)
    {
        if (!message.empty())
            encode(message);
    }
    int encode(vector<unsigned char> message)
    {
        for (const auto& i : message)
        {
            bitset<4> pieces[] = {i >> 4 & 15,i & 15};
            for (const auto& j : pieces)
            {
                bitset<8> code = 0;
                for (unsigned int k = 1, delta=0; k < 8; k++)
                {
                    if (k & (k - 1))
                    {
                        code[k] = j[j.size() - 1 - delta] & 1;
                        delta++;
                    }
                }
                codedMessage.push_back(code);
            }
        }
        for (auto& item : codedMessage)
        {
            bitset<3> bits = getcheck(item);
            for (unsigned int i = 1, j = 0; i < 8; i++)
            {
                if (!(i & (i - 1)))
                {
                    item[i] = bits[j];
                    j++;
                }
            }
        }
        if (extended)
        {
            for (auto& item : codedMessage)
            {
                item[0] = getparity(item);
            }
        }
        return 0;
    }
    vector<bitset<8>> get()
    {
        vector<bitset<8>>result;
        for (auto& item : codedMessage)
        {
            bitset<8> byte;
            for (unsigned int i = 0; i < byte.size(); i++)
            {
                byte[byte.size() - 1 - i] = item[i];
            }
            result.push_back(byte);
        }
        return result;
    }
    vector<bitset<7>> get7()
    {
        vector<bitset<7>>result;
        for (auto& item : codedMessage)
        {
            bitset<7> byte;
            for (unsigned int i = 1; i < item.size(); i++)
            {
                byte[byte.size() - i] = item[i];
            }
            result.push_back(byte);
        }
        return result;
    }
    int fix()
    {
        int error = 0;
        for (unsigned int i = 0; i < codedMessage.size(); i++)
        {
            bool parity = getparity(codedMessage[i]);
            bitset<3> bits = getcheck(codedMessage[i]);
            if (extended)
            {
                if (bits.any())
                {
                    if (parity)
                    {
                        error += 1;
                        //there is an error
                        cout << "error in block " << i << "\n";
                        unsigned int position = bits.to_ullong();
                        codedMessage[i].flip(position);
                    }
                    else
                    {
                        error += 1;
                        //there is an error
                        cout << "fatal error in block " << i << "!\n";
                    }
                }
                if ()
            }
            else
            {
                if (bits.any())
                {
                    error += 1;
                    //there is an error
                    cout << "error in block " << i << "\n";
                    unsigned int position = bits.to_ullong();
                    codedMessage[i].flip(position);
                }
            }
        }
        return error;
    }
    int decode(vector<unsigned char>& result) {
        int error = fix();
        result = {};
        bool lower = 0;
        unsigned char insert = 0;
        for (unsigned int i = 0; i < codedMessage.size(); i++)
        {
            for (unsigned int k = 1, delta = 0; k < 8; k++)
            {
                if (k & (k - 1))
                {
                    insert <<= 1;
                    insert |= codedMessage[i][k] & 1;
                }
            }
            if (lower)
            {
                result.push_back(insert);
                lower = false;
            }
            else
                lower = true;
        }
        return error;
    }
    //damage bit manualy
    int manualDamage(unsigned int position)
    {
        const unsigned char stride = (extended) ? 8 : 7;
        if (position >= codedMessage.size() * stride) return 0;
        unsigned int posx = position / stride, posy = position % stride;
        if (stride == 7) posy++;
        codedMessage[posx].flip(posy);
        return 1;
    }
    //damage exact ammount of bits each block
    int autoDamage(unsigned int sizeOfDamage)
    {
        unsigned int errors = 0;
        const unsigned char stride = (extended) ? 8 : 7;
        std::default_random_engine generator;
        std::uniform_int_distribution<int> distribution(0, stride-1);

        for (auto& item : codedMessage)
        {
            vector<unsigned int> errorList;
            unsigned int random;
            for (unsigned i = 0; i < sizeOfDamage; i++)
            {
                random = distribution(generator);
                while (std::find(errorList.begin(), errorList.end(), random) != errorList.end())
                {
                    random = (random + 1) % stride;
                }
                errorList.push_back(random);
                if (stride == 7) random++;
                item.flip(random);
                errors++;
            }
        }
        return errors;
    }
private:
    bitset<3> getcheck(bitset<8> item)
    {
        char code = 0;
        for (unsigned int i = 1; i < 8; i++)
        {
            code ^= item[i] * i;
        }
        return code;
    }
    bool getparity(bitset<8> item)
    {
        bool sum = 0;
        for (unsigned int i = 0; i < item.size(); i++)
        {
            sum ^= item[i];
        }
        return sum;
    }
};

//{input mode}{damage mode}
int main(int argc, char* argv[])
{
    system("chcp 1251");
    std::list<std::string> params;
    for (int i = 1; i < argc; i++)
    {
        params.push_back(std::string(argv[i]));
    }
    //input
    vector<unsigned char> buffer;
    if (params.front() == "/s")
    {
        cout << "input from string:";
        std::string str;
        std::getline(std::cin, str);
        buffer = vector<unsigned char>(str.begin(), str.end());
    }
    else if (params.front() == "/f")
    {
        cout << "input from file:";
        std::string str;
        std::getline(std::cin, str);
        std::ifstream file(str, std::ios::binary);
        std::istreambuf_iterator<char> start(file), eos;
        buffer = vector<unsigned char>(start, eos);
    }
    else
    {
        return -1;
    }
    params.pop_front();
    //extended?
    cout << "extended? (0 - no,1 - yes):";
    bool extended;
    std::cin >> extended;
    //encryption
    HamingCode8 h(extended, buffer);
    cout << "encrypted:\n";
    PRINT_THING;
    //damage
    if (params.front() == "/a")
    {
        cout << "input ammount of errors in each block:";
        unsigned int ammount;
        std::cin >> ammount;
        h.autoDamage(ammount);
        cout << "damage done\n";
    }
    else if (params.front() == "/m")
    {
        cout << "manual error input:";
        vector<unsigned int> damage;
        bool more = 0;
        do 
        {
            unsigned int pos;
            std::cin >> pos;
            damage.push_back(pos);
            cout << "more?";
            std::cin >> more;
        } while (more);
        for (auto& item : damage)
        {
            if (h.manualDamage(item))
                cout << "damaged bit " << item << "\n";
        }
    }
    else
    {
        return -1;
    }
    cout << "damaged:\n";
    PRINT_THING;
    //fix
    cout << "fixing...\n";
    h.fix();
    cout << "fixed:\n";
    PRINT_THING;
    vector<unsigned char> recieved;
    h.decode(recieved);
    cout << "decoded message:\n";
    for (auto& item : recieved)
    {
        cout << item;
    }
}