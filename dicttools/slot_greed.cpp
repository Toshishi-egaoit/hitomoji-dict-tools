#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <bitset>
#include <sqlite3.h>

constexpr int MAX_SLOT = 120;

struct Kanji {
        int cp;
        std::string literal;
        int freq;
        std::vector<int> yomi_ids;
        int slot = -1;
};

std::string kana_to_romaji(const std::string& input);
std::string katakana_to_hiragana(const std::string& input);

int main(int argc, char** argv) {

        std::string dbname = "kanji.db";
        bool output_detail = false;

        for (int i = 1; i < argc; ++i) {
                if (std::string(argv[i]) == "-o")
                        output_detail = true;
                if (std::string(argv[i]) == "-d" && i+1 < argc)
                        dbname = argv[i+1];
        }

        sqlite3* db = nullptr;
        if (sqlite3_open(dbname.c_str(), &db) != SQLITE_OK) {
                std::cerr << "cannot open database\n";
                return 1;
        }

        const char* sql =
                "select k.cp, k.letter, y.yomi, k.freq "
                "from y join k on k.cp = y.cp "
                "order by k.freq asc";

        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
                std::cerr << "prepare failed\n";
                return 1;
        }

        std::unordered_map<int, int> kanji_index;
        std::unordered_map<std::string, int> yomi_index;

        std::vector<std::string> yomi_list;
        std::vector<Kanji> kanjis;
        std::vector<std::vector<int>> yomi_to_kanji;

        int max_group_size = 0;

        while (sqlite3_step(stmt) == SQLITE_ROW) {

                int cp = sqlite3_column_int(stmt, 0);
                std::string literal =
                        (const char*)sqlite3_column_text(stmt, 1);
                std::string yomi =
                        (const char*)sqlite3_column_text(stmt, 2);
                int freq = sqlite3_column_int(stmt, 3);

                if (kanji_index.find(cp) == kanji_index.end()) {
                        int id = kanjis.size();
                        kanji_index[cp] = id;

                        Kanji k;
                        k.cp = cp;
                        k.literal = literal;
                        k.freq = freq;

                        kanjis.push_back(k);
                }

                int k_id = kanji_index[cp];

                std::string normalized = katakana_to_hiragana(yomi);

                if (yomi_index.find(normalized) == yomi_index.end()) {
                        int id = yomi_to_kanji.size();
                        yomi_index[normalized] = id;
                        yomi_to_kanji.emplace_back();
                        yomi_list.push_back(normalized);
                }

                int y_id = yomi_index[normalized];

                // avoid duplicate (cp,yomi) pairs just in case DB/view produces duplicates
                bool exists = false;
                for (int x : yomi_to_kanji[y_id]) {
                        if (x == k_id) { exists = true; break; }
                }

                if (!exists) {
                        kanjis[k_id].yomi_ids.push_back(y_id);
                        yomi_to_kanji[y_id].push_back(k_id);
                }

                max_group_size = std::max(
                        max_group_size,
                        (int)yomi_to_kanji[y_id].size());
        }

        sqlite3_finalize(stmt);

        // keep DB open for slot update later

        std::cout << "max_group_size = " << max_group_size << std::endl;

        std::vector<int> yomi_order(yomi_to_kanji.size());
        for (int i = 0; i < yomi_order.size(); ++i)
                yomi_order[i] = i;

        std::sort(yomi_order.begin(), yomi_order.end(),
                [&](int a, int b) {
                        return yomi_to_kanji[a].size() > yomi_to_kanji[b].size();
                });

        for (int y_id : yomi_order) {
                for (int k_id : yomi_to_kanji[y_id]) {

                        if (kanjis[k_id].slot != -1)
                                continue;

                        std::bitset<MAX_SLOT + 1> forbidden;

                        for (int other_y : kanjis[k_id].yomi_ids) {
                                for (int other_k : yomi_to_kanji[other_y]) {
                                        int s = kanjis[other_k].slot;
                                        if (s != -1)
                                                forbidden.set(s);
                                }
                        }

                        bool assigned = false;

                        for (int s = 1; s <= MAX_SLOT; ++s) {
                                if (!forbidden.test(s)) {
                                        kanjis[k_id].slot = s;
                                        assigned = true;
                                        break;
                                }
                        }

                        if (!assigned) {
                                std::cerr << "FAILED at " << kanjis[k_id].literal << "\n";
                                return 1;
                        }
                }
        }

        int max_slot = 0;
        for (const auto& k : kanjis)
                max_slot = std::max(max_slot, k.slot);

        std::cout << "Kanji count: " << kanjis.size() << "\n";
        std::cout << "Yomi count: " << yomi_to_kanji.size() << "\n";
        std::cout << "Max slot used: " << max_slot << "\n";

        if (output_detail) {
                for (const auto& k : kanjis) {
                        for (int y_id : k.yomi_ids) {
                                std::string yomi = yomi_list[y_id];
                                std::string romaji = kana_to_romaji(yomi);

                                std::cout << romaji << " " << k.slot
                                          << "=" << k.literal
                                          << "\n";
                        }
                }
        }

                // --- write slots back to SQLite ---
        const char* sql_update = "update k set slot=? where cp=?";
        sqlite3_stmt* stmt_update;

        if (sqlite3_prepare_v2(db, sql_update, -1, &stmt_update, nullptr) != SQLITE_OK) {
                std::cerr << "prepare update failed\n";
                return 1;
        }

        sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr);

        // reset existing slots first
        sqlite3_exec(db, "update k set slot=NULL", nullptr, nullptr, nullptr);

        for (const auto& k : kanjis) {
                sqlite3_bind_int(stmt_update, 1, k.slot);
                sqlite3_bind_int(stmt_update, 2, k.cp);

                if (sqlite3_step(stmt_update) != SQLITE_DONE) {
                        std::cerr << "update failed for cp=" << k.cp << "\n";
                }

                sqlite3_reset(stmt_update);
        }

        sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);

        sqlite3_finalize(stmt_update);
        sqlite3_close(db);

        return 0;
}

std::string kana_to_romaji(const std::string& input) {
        static std::unordered_map<std::string, std::string> table = {
                {"あ","a"},{"い","i"},{"う","u"},{"え","e"},{"お","o"},
                {"か","ka"},{"き","ki"},{"く","ku"},{"け","ke"},{"こ","ko"},
                {"さ","sa"},{"し","si"},{"す","su"},{"せ","se"},{"そ","so"},
                {"た","ta"},{"ち","ti"},{"つ","tsu"},{"て","te"},{"と","to"},
                {"な","na"},{"に","ni"},{"ぬ","nu"},{"ね","ne"},{"の","no"},
                {"は","ha"},{"ひ","hi"},{"ふ","fu"},{"へ","he"},{"ほ","ho"},
                {"ま","ma"},{"み","mi"},{"む","mu"},{"め","me"},{"も","mo"},
                {"や","ya"},{"ゆ","yu"},{"よ","yo"},
                {"ら","ra"},{"り","ri"},{"る","ru"},{"れ","re"},{"ろ","ro"},
                {"わ","wa"},{"を","wo"},{"ん","n"},
                {"が","ga"},{"ぎ","gi"},{"ぐ","gu"},{"げ","ge"},{"ご","go"},
                {"ざ","za"},{"じ","ji"},{"ず","zu"},{"ぜ","ze"},{"ぞ","zo"},
                {"だ","da"},{"ぢ","di"},{"づ","du"},{"で","de"},{"ど","do"},
                {"ば","ba"},{"び","bi"},{"ぶ","bu"},{"べ","be"},{"ぼ","bo"},
                {"ぱ","pa"},{"ぴ","pi"},{"ぷ","pu"},{"ぺ","pe"},{"ぽ","po"},
                {"きゃ","kya"},{"きゅ","kyu"},{"きょ","kyo"},
                {"しゃ","sha"},{"しゅ","shu"},{"しょ","sho"},
                {"ちゃ","cha"},{"ちゅ","chu"},{"ちょ","cho"},
                {"にゃ","nya"},{"にゅ","nyu"},{"にょ","nyo"},
                {"ひゃ","hya"},{"ひゅ","hyu"},{"ひょ","hyo"},
                {"みゃ","mya"},{"みゅ","myu"},{"みょ","myo"},
                {"りゃ","rya"},{"りゅ","ryu"},{"りょ","ryo"},
                {"ぎゃ","gya"},{"ぎゅ","gyu"},{"ぎょ","gyo"},
                {"じゃ","ja"},{"じゅ","ju"},{"じょ","jo"},
                {"びゃ","bya"},{"びゅ","byu"},{"びょ","byo"},
                {"ぴゃ","pya"},{"ぴゅ","pyu"},{"ぴょ","pyo"},
                {"っ","t"},
        };

        std::string result;
        for (size_t i = 0; i < input.size();) {
                std::string two = input.substr(i, 6); // UTF-8 2文字対応（簡易）
                if (i + 6 <= input.size() && table.count(input.substr(i,6))) {
                        result += table[input.substr(i,6)];
                        i += 6;
                } else {
                        std::string one = input.substr(i,3);
                        if (table.count(one))
                                result += table[one];
                        i += 3;
                }
        }

        return result;
  }

std::string katakana_to_hiragana(const std::string& input) {
    std::string out;
    for (size_t i = 0; i < input.size();) {
        unsigned char c = input[i];

        if ((c & 0xF0) == 0xE0) { // UTF-8 3バイト
            std::string ch = input.substr(i, 3);

            unsigned int code =
                ((ch[0] & 0x0F) << 12) |
                ((ch[1] & 0x3F) << 6) |
                (ch[2] & 0x3F);

            // カタカナ範囲ならひらがなへ
            if (code >= 0x30A1 && code <= 0x30F6) {
                code -= 0x60;

                out += char(0xE0 | ((code >> 12) & 0x0F));
                out += char(0x80 | ((code >> 6) & 0x3F));
                out += char(0x80 | (code & 0x3F));
            } else {
                out += ch;
            }

            i += 3;
        } else {
            out += input[i++];
        }
    }
    return out;
}

