#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>

// =========================
// Structs
// =========================

struct YomiEntry
{
    char16_t yomi[6];   // UTF-16 (max 6 chars)
    uint16_t length;    // actual length
    uint16_t count;     // number of kanji
    uint32_t offset;    // offset into kanji list
};

struct Dict
{
    std::vector<YomiEntry> yomi;
    std::vector<uint32_t> kanji;
};

// =========================
// UTF conversions
// =========================

std::u16string utf8_to_utf16(const std::string& s)
{
    std::u16string out;

    for(size_t i=0;i<s.size();)
    {
        uint32_t cp=0;

        if((s[i] & 0x80)==0)
        {
            cp = s[i++];
        }
        else if((s[i] & 0xE0)==0xC0)
        {
            cp = ((s[i]&0x1F)<<6) |
                 (s[i+1]&0x3F);
            i+=2;
        }
        else
        {
            cp = ((s[i]&0x0F)<<12) |
                 ((s[i+1]&0x3F)<<6) |
                 (s[i+2]&0x3F);
            i+=3;
        }

        out.push_back((char16_t)cp);
    }

    return out;
}

std::string ucs4_to_utf8(uint32_t cp)
{
    std::string s;

    if(cp < 0x80)
    {
        s.push_back(cp);
    }
    else if(cp < 0x800)
    {
        s.push_back(0xC0 | (cp >> 6));
        s.push_back(0x80 | (cp & 0x3F));
    }
    else if(cp < 0x10000)
    {
        s.push_back(0xE0 | (cp >> 12));
        s.push_back(0x80 | ((cp >> 6) & 0x3F));
        s.push_back(0x80 | (cp & 0x3F));
    }
    else
    {
        s.push_back(0xF0 | (cp >> 18));
        s.push_back(0x80 | ((cp >> 12) & 0x3F));
        s.push_back(0x80 | ((cp >> 6) & 0x3F));
        s.push_back(0x80 | (cp & 0x3F));
    }

    return s;
}

// =========================
// Compare
// =========================

int cmp(const char16_t* a, uint16_t lenA,
        const char16_t* b, uint16_t lenB)
{
    int n = std::min(lenA, lenB);
    int r = std::memcmp(a, b, n * sizeof(char16_t));
    if(r != 0) return r;
    return (int)lenA - (int)lenB;
}

// =========================
// Lookup
// =========================

std::vector<uint32_t> lookup(
    const std::u16string& yomi,
    const YomiEntry* entries,
    size_t entryCount,
    const uint32_t* kanjiList)
{
    int left = 0;
    int right = (int)entryCount - 1;

    while(left <= right)
    {
        int mid = (left + right) / 2;
        const auto& e = entries[mid];

        int c = cmp(
            yomi.data(), yomi.size(),
            e.yomi, e.length);

        if(c == 0)
        {
            std::vector<uint32_t> result;

            for(int i=0;i<e.count;i++)
            {
                result.push_back(
                    kanjiList[e.offset + i]);
            }
            return result;
        }
        else if(c < 0)
        {
            right = mid - 1;
        }
        else
        {
            left = mid + 1;
        }
    }

    return {};
}

// =========================
// Load Dict (simple binary)
// =========================

Dict loadDict(const std::string& path)
{
    Dict d;

    std::ifstream ifs(path, std::ios::binary);
    if(!ifs)
    {
        throw std::runtime_error("cannot open dict file");
    }

    uint32_t yomiCount;
    uint32_t kanjiCount;

    ifs.read((char*)&yomiCount, sizeof(yomiCount));
    ifs.read((char*)&kanjiCount, sizeof(kanjiCount));

    d.yomi.resize(yomiCount);
    d.kanji.resize(kanjiCount);

    ifs.read((char*)d.yomi.data(), yomiCount * sizeof(YomiEntry));
    ifs.read((char*)d.kanji.data(), kanjiCount * sizeof(uint32_t));

    return d;
}

// =========================
// Print
// =========================

void printList(const std::string& yomi,
               const std::vector<uint32_t>& list)
{
    std::cout << "[" << yomi << "] "
              << list.size() << "件\n\n";

    for(size_t i=0;i<list.size();i++)
    {
        std::cout << i << ": "
                  << ucs4_to_utf8(list[i])
                  << "\n";
    }
}

// =========================
// Main
// =========================

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cerr << "usage: dictview <yomi>\n";
        return 1;
    }

    std::string input = argv[1];
    std::u16string yomi = utf8_to_utf16(input);

    Dict dict = loadDict("dict.bin");

    auto list = lookup(yomi,
                       dict.yomi.data(),
                       dict.yomi.size(),
                       dict.kanji.data());

    printList(input, list);

    return 0;
}
