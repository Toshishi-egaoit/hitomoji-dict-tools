#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>

// =========================
// Structs
// =========================

struct DictHeader {
    uint32_t magic;
    uint32_t kanjiCount;
    uint32_t yomiCount;
    uint32_t kanjiOffset;
    uint32_t yomiOffset;
    uint32_t listOffset;
};

struct YomiEntry {
    char16_t yomi[6];
    uint16_t count;
    uint32_t offset;
};

struct Dict {
    std::vector<YomiEntry> yomi;
    std::vector<uint32_t> kanjiList;
};

// =========================
// UTF
// =========================

std::u16string utf8_to_utf16_fixed6(const std::string& s)
{
    std::u16string out(6, 0);

    size_t i=0;
    int j=0;

    while(i < s.size() && j < 5)
    {
        uint32_t cp=0;

        if((s[i] & 0x80)==0)
            cp = s[i++];
        else if((s[i] & 0xE0)==0xC0)
        {
            cp = ((s[i]&0x1F)<<6) | (s[i+1]&0x3F);
            i+=2;
        }
        else
        {
            cp = ((s[i]&0x0F)<<12) |
                 ((s[i+1]&0x3F)<<6) |
                 (s[i+2]&0x3F);
            i+=3;
        }

        out[j++] = (char16_t)cp;
    }

    return out;
}

std::string ucs4_to_utf8(uint32_t cp)
{
    std::string s;

    if(cp < 0x80)
        s.push_back(cp);
    else if(cp < 0x800)
    {
        s.push_back(0xC0 | (cp >> 6));
        s.push_back(0x80 | (cp & 0x3F));
    }
    else
    {
        s.push_back(0xE0 | (cp >> 12));
        s.push_back(0x80 | ((cp >> 6) & 0x3F));
        s.push_back(0x80 | (cp & 0x3F));
    }

    return s;
}

// =========================
// Compare
// =========================

int cmp_fixed6(const char16_t* a, const char16_t* b)
{
    return std::memcmp(a, b, 6 * sizeof(char16_t));
}

// =========================
// Lookup
// =========================

std::vector<uint32_t> lookup(const char16_t key[6],
                             const std::vector<YomiEntry>& entries,
                             const std::vector<uint32_t>& kanjiList)
{
    int left = 0;
    int right = (int)entries.size() - 1;

    while(left <= right)
    {
        int mid = (left + right) / 2;
        const auto& e = entries[mid];

        int c = cmp_fixed6(key, e.yomi);

        if(c == 0)
        {
            std::vector<uint32_t> result;
            for(int i=0;i<e.count;i++)
                result.push_back(kanjiList[e.offset + i]);
            return result;
        }
        else if(c < 0)
            right = mid - 1;
        else
            left = mid + 1;
    }

    return {};
}

// =========================
// Load
// =========================

Dict loadDict(const std::string& path)
{
    std::ifstream ifs(path, std::ios::binary);
    if(!ifs) throw std::runtime_error("cannot open dict");

    DictHeader header;
    ifs.read((char*)&header, sizeof(header));

    Dict d;

    ifs.seekg(header.yomiOffset);
    d.yomi.resize(header.yomiCount);
    ifs.read((char*)d.yomi.data(), header.yomiCount * sizeof(YomiEntry));

    ifs.seekg(header.listOffset);
    auto cur = ifs.tellg();
    ifs.seekg(0, std::ios::end);
    size_t bytes = (size_t)(ifs.tellg() - cur);

    d.kanjiList.resize(bytes / sizeof(uint32_t));
    ifs.seekg(header.listOffset);
    ifs.read((char*)d.kanjiList.data(), bytes);

    return d;
}

// =========================
// Keymap
// =========================

std::string remove_spaces(std::string s)
{
    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
    return s;
}

std::string default_keymap()
{
    return remove_spaces(
        "dk fj sl gh ei ru a; wo qp ty c, vm x. z/ bn 38 47 29 10 56"
    );
}

std::string resolve_keymap(const char* opt)
{
    if(opt) return remove_spaces(opt);
    const char* env = std::getenv("SLOT_MAP");
    if(env) return remove_spaces(env);
    return default_keymap();
}

// =========================
// Render
// =========================

void render_list(const std::vector<uint32_t>& list)
{
    for(size_t i=0;i<list.size();i++)
        std::cout << i << ": " << ucs4_to_utf8(list[i]) << "\n";
}

void render_keyboard(const std::vector<uint32_t>& list,
                     const std::string& keymap,
                     int page)
{
    const std::string empty = "〇";

    size_t pageSize = keymap.size();
    size_t start = (page-1)*pageSize;

    // 101キーボード配列（表示位置）
    std::string rows[4] = {
        "1234567890",
        "qwertyuiop",
        "asdfghjkl;",
        "zxcvbnm,./"
    };

    for(int r=0;r<4;r++)
    {
        for(char key : rows[r])
        {
            // keymap内の位置（slot順位）を探す
            auto pos_it = keymap.find(key);

            if(pos_it == std::string::npos)
            {
                std::cout << empty << " ";
                continue;
            }

            size_t slotIndex = pos_it;
            size_t listIndex = start + slotIndex;

            if(listIndex < list.size()) {
                uint32_t cp = list[listIndex];
                if(cp == 0)
                    std::cout << empty << " ";
                else
                    std::cout << ucs4_to_utf8(cp) << " ";
            } else
                std::cout << empty << " ";
        }
        std::cout << "\n";
    }
}

// =========================
// Main
// =========================

int main(int argc, char* argv[])
{
    std::string yomi;
    const char* opt_k = nullptr;
    int page = 1;
    bool listMode = false;

    for(int i=1;i<argc;i++)
    {
        std::string a = argv[i];

        if(a == "-k" && i+1 < argc)
            opt_k = argv[++i];
        else if(a == "-p" && i+1 < argc)
            page = std::stoi(argv[++i]);
        else if(a == "-l")
            listMode = true;
        else
            yomi = a;
    }

    if(yomi.empty())
    {
        std::cerr << "usage: dictview [-k map] [-p n] [-l] <yomi>\n";
        return 1;
    }

    auto y16 = utf8_to_utf16_fixed6(yomi);

    char16_t key[6];
    std::memcpy(key, y16.data(), 6*sizeof(char16_t));

    auto dict = loadDict("hitomoji.dic");
    auto list = lookup(key, dict.yomi, dict.kanjiList);

    if(listMode)
    {
        render_list(list);
    }
    else
    {
        render_keyboard(list, resolve_keymap(opt_k), page);
    }

    return 0;
}


