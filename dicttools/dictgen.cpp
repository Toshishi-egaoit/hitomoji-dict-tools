// ===== dictgen.cpp (cleaned & slot-based version) =====
//  v0.3.

#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <cstring>
#include <sqlite3.h>

constexpr int MAX_SLOT = 120;

struct DictHeader {
    uint32_t magic;
    uint32_t kanjiCount;
    uint32_t yomiCount;
    uint32_t kanjiOffset;
    uint32_t yomiOffset;
    uint32_t listOffset;
};

struct KanjiEntry {
    uint32_t codepoint;
    uint16_t grade;
    uint16_t slot;
};

struct YomiEntry {
    char16_t yomi[6];
    uint16_t count;
    uint32_t offset;
};

static void utf8_to_utf16_6(const char *src, char16_t dst[6])
{
    std::memset(dst,0,12);

    int i=0;
    while(*src && i<5)
    {
        unsigned char c=*src;

        if(c<0x80)
        {
            dst[i++]=c;
            src++;
        }
        else if((c&0xE0)==0xE0)
        {
            uint16_t u =
                ((src[0]&0x0F)<<12) |
                ((src[1]&0x3F)<<6)  |
                (src[2]&0x3F);

            dst[i++]=u;
            src+=3;
        }
        else
        {
            src++;
        }
    }
}

int main(int argc,char **argv)
{
    if(argc<3)
    {
        std::cerr<<"usage: dictgen dbfile outfile\n";
        return 1;
    }

    sqlite3 *db;

    if(sqlite3_open(argv[1],&db)!=SQLITE_OK)
    {
        std::cerr<<"db open error\n";
        return 1;
    }

    std::vector<KanjiEntry> kanji;
    std::vector<YomiEntry> yomi;
    std::vector<uint32_t> list;

    // --- kanji table ---
    {
        sqlite3_stmt *stmt;

        sqlite3_prepare_v2(db,
            "select cp,grade,slot from k order by slot",
            -1,&stmt,nullptr);

        while(sqlite3_step(stmt)==SQLITE_ROW)
        {
            KanjiEntry e;

            e.codepoint = sqlite3_column_int(stmt,0);
            e.grade     = sqlite3_column_int(stmt,1);
            e.slot      = sqlite3_column_int(stmt,2);

            kanji.push_back(e);
        }

        sqlite3_finalize(stmt);
    }

    // --- yomi mapping (WITH RECURSIVE) ---
    std::map<std::string, std::vector<uint32_t>> mapy_slots;

    {
        sqlite3_stmt *stmt;

        std::string sql_cte =
            "WITH RECURSIVE slots(n) AS ("
            "  SELECT 1 "
            "  UNION ALL "
            "  SELECT n+1 FROM slots WHERE n < " + std::to_string(MAX_SLOT) +
            ") "
            "SELECT y.yomi, slots.n AS slot, k.cp "
            "FROM (SELECT DISTINCT yomi FROM y) y "
            "JOIN slots "
            "LEFT JOIN (SELECT yomi, k.cp, k.slot FROM y JOIN k ON y.cp = k.cp) k "
            "  ON k.yomi = y.yomi AND k.slot = slots.n "
            "ORDER BY y.yomi, slots.n";

        if (sqlite3_prepare_v2(db, sql_cte.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "prepare CTE failed\n";
            return 1;
        }

        std::string current_y;
        std::vector<uint32_t> slots(MAX_SLOT + 1, 0);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char *y = (const char*)sqlite3_column_text(stmt, 0);
            int slot = sqlite3_column_int(stmt, 1);
            uint32_t cp = (sqlite3_column_type(stmt, 2) == SQLITE_NULL)
                            ? 0
                            : (uint32_t)sqlite3_column_int(stmt, 2);

            std::string ystr = y ? std::string(y) : std::string();

            if (current_y.empty()) {
                current_y = ystr;
                std::fill(slots.begin(), slots.end(), 0);
            }

            if (ystr != current_y) {
                mapy_slots[current_y] = slots;
                current_y = ystr;
                std::fill(slots.begin(), slots.end(), 0);
            }

            if (slot >= 1 && slot <= MAX_SLOT) {
                slots[slot] = cp;
            }
        }

        if (!current_y.empty()) {
            mapy_slots[current_y] = slots;
        }

        sqlite3_finalize(stmt);
    }

    // --- build yomi/list (variable length, trim tail) ---
    for (auto &p : mapy_slots)
    {
        YomiEntry e;

        utf8_to_utf16_6(p.first.c_str(), e.yomi);

        const auto &slots = p.second;

        int last = 0;
        for (int s = MAX_SLOT; s >= 1; --s) {
            if (slots[s] != 0) { last = s; break; }
        }

        e.offset = list.size();
        e.count  = last;

        for (int s = 1; s <= last; ++s) {
            list.push_back(slots[s]);
        }

        yomi.push_back(e);
    }

    sqlite3_close(db);

    // --- write file ---
    std::ofstream out(argv[2],std::ios::binary);

    DictHeader header;

    header.magic = 0x314A4D48; // HMJ1

    header.kanjiCount = kanji.size();
    header.yomiCount  = yomi.size();

    header.kanjiOffset = sizeof(DictHeader);
    header.yomiOffset  = header.kanjiOffset + kanji.size()*sizeof(KanjiEntry);
    header.listOffset  = header.yomiOffset + yomi.size()*sizeof(YomiEntry);

    out.write((char*)&header,sizeof(header));
    out.write((char*)kanji.data(),kanji.size()*sizeof(KanjiEntry));
    out.write((char*)yomi.data(),yomi.size()*sizeof(YomiEntry));
    out.write((char*)list.data(),list.size()*sizeof(uint32_t));

    out.close();

    std::cout<<"kanji "<<kanji.size()<<"\n";
    std::cout<<"yomi  "<<yomi.size()<<"\n";
    std::cout<<"list  "<<list.size()<<"\n";

    return 0;
}
