// dictmatch.cpp
// KANJIDIC2とJMdictを突合させて、差分を出力するプログラム
// KANJIDIC2にしかないものと、JMdict側にしかないものを表示する。

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <sqlite3.h>
#include <stdint.h>
#include <string>
#include <sys/stat.h>
#include <vector>

struct Reading {
    std::string tp;
    std::string yomi;
};

static uint32_t next_cp(const std::string& s, size_t& i)
{
    unsigned char c = s[i];
    if (c < 0x80) {
        i += 1;
        return c;
    }
    if ((c & 0xE0) == 0xC0) {
        uint32_t cp = ((s[i] & 0x1F) << 6) | (s[i + 1] & 0x3F);
        i += 2;
        return cp;
    }
    if ((c & 0xF0) == 0xE0) {
        uint32_t cp = ((s[i] & 0x0F) << 12) |
                      ((s[i + 1] & 0x3F) << 6) |
                      (s[i + 2] & 0x3F);
        i += 3;
        return cp;
    }
    i += 1;
    return c;
}

static std::string cp_to_utf8(uint32_t cp)
{
    std::string s;
    if (cp < 0x80) {
        s.push_back((char)cp);
    } else if (cp < 0x800) {
        s.push_back((char)(0xC0 | (cp >> 6)));
        s.push_back((char)(0x80 | (cp & 0x3F)));
    } else {
        s.push_back((char)(0xE0 | (cp >> 12)));
        s.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
        s.push_back((char)(0x80 | (cp & 0x3F)));
    }
    return s;
}

static std::vector<uint32_t> to_cps(const std::string& s)
{
    std::vector<uint32_t> out;
    for (size_t i = 0; i < s.size();)
        out.push_back(next_cp(s, i));
    return out;
}

static std::string cps_to_string(const std::vector<uint32_t>& cps, size_t pos, size_t len)
{
    std::string out;
    for (size_t i = 0; i < len; ++i)
        out += cp_to_utf8(cps[pos + i]);
    return out;
}

static bool is_kanji(uint32_t cp)
{
    return cp >= 0x4E00 && cp <= 0x9FAF;
}

static bool is_kana(uint32_t cp)
{
    return (cp >= 0x3041 && cp <= 0x3096) ||
           (cp >= 0x30A1 && cp <= 0x30F6) ||
           cp == 0x30FC;
}

static uint32_t kata_to_hira_cp(uint32_t cp)
{
    if (cp >= 0x30A1 && cp <= 0x30F6)
        return cp - 0x60;
    return cp;
}

static std::string kata_to_hira(const std::string& s)
{
    std::string out;
    for (uint32_t cp : to_cps(s))
        out += cp_to_utf8(kata_to_hira_cp(cp));
    return out;
}

static bool starts_with(const std::vector<uint32_t>& text, size_t pos, const std::vector<uint32_t>& pat)
{
    if (pos + pat.size() > text.size())
        return false;
    for (size_t i = 0; i < pat.size(); ++i) {
        if (text[pos + i] != pat[i])
            return false;
    }
    return true;
}

static bool all_kana(const std::vector<uint32_t>& text, size_t pos, size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        if (!is_kana(text[pos + i]))
            return false;
    }
    return true;
}

static void add_example(std::map<std::string, std::vector<std::string>>& examples,
                        const std::string& key,
                        const std::string& value,
                        size_t limit = 5)
{
    auto& bucket = examples[key];
    if (limit != 0 && bucket.size() >= limit)
        return;
    if (std::find(bucket.begin(), bucket.end(), value) == bucket.end())
        bucket.push_back(value);
}

static void print_examples(const std::string& yomi, const std::vector<std::string>& examples, bool missing)
{
    for (const auto& ex : examples) {
        size_t sep = ex.find('/');
        if (sep == std::string::npos) {
            std::cout << "\t" << (missing ? "T" : "F") << "\t" << yomi << "\t" << ex << "\t\n";
        } else {
            std::cout << "\t" << (missing ? "T" : "F") << "\t" << yomi << "\t" << ex.substr(0, sep) << "\t" << ex.substr(sep + 1) << "\n";
        }
    }
}

static std::vector<Reading> load_target_readings(sqlite3* db, const std::string& target)
{
    std::vector<Reading> out;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "select tp, yomi from y where letter=? order by tp, yomi";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return out;

    sqlite3_bind_text(stmt, 1, target.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Reading r;
        r.tp = (const char*)sqlite3_column_text(stmt, 0);
        r.yomi = kata_to_hira((const char*)sqlite3_column_text(stmt, 1));
        out.push_back(r);
    }
    sqlite3_finalize(stmt);
    return out;
}

static std::map<std::string, std::vector<std::string>> load_all_readings(sqlite3* db)
{
    std::map<std::string, std::vector<std::string>> out;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "select distinct letter, yomi from y order by letter, length(yomi) desc, yomi";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return out;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string letter = (const char*)sqlite3_column_text(stmt, 0);
        std::string yomi = kata_to_hira((const char*)sqlite3_column_text(stmt, 1));
        out[letter].push_back(yomi);
    }
    sqlite3_finalize(stmt);
    return out;
}

static bool align_one(const std::vector<uint32_t>& keb,
                      const std::vector<uint32_t>& reb,
                      size_t kpos,
                      size_t rpos,
                      uint32_t target_cp,
                      const std::map<std::string, std::vector<std::string>>& all_readings,
                      std::set<std::string>& target_segments)
{
    if (kpos == keb.size())
        return rpos == reb.size();

    uint32_t kcp = keb[kpos];

    if (kcp == target_cp) {
        for (size_t len = 1; len <= 5 && rpos + len <= reb.size(); ++len) {
            if (!all_kana(reb, rpos, len))
                continue;

            std::set<std::string> sub_segments;
            if (align_one(keb, reb, kpos + 1, rpos + len, target_cp, all_readings, sub_segments)) {
                target_segments.insert(cps_to_string(reb, rpos, len));
                target_segments.insert(sub_segments.begin(), sub_segments.end());
                return true;
            }
        }
        return false;
    }

    if (is_kanji(kcp)) {
        std::string letter = cp_to_utf8(kcp);
        auto it = all_readings.find(letter);
        if (it == all_readings.end())
            return false;

        bool matched = false;
        size_t best_len = 0;
        for (const auto& yomi : it->second) {
            std::vector<uint32_t> ycps = to_cps(yomi);
            if (!starts_with(reb, rpos, ycps))
                continue;
            if (best_len != 0 && ycps.size() < best_len)
                break;

            best_len = ycps.size();
            if (align_one(keb, reb, kpos + 1, rpos + ycps.size(), target_cp, all_readings, target_segments))
                matched = true;
        }
        return matched;
    }

    uint32_t normalized = kata_to_hira_cp(kcp);
    if (rpos < reb.size() && normalized == reb[rpos])
        return align_one(keb, reb, kpos + 1, rpos + 1, target_cp, all_readings, target_segments);

    return false;
}

static int count_target(const std::vector<uint32_t>& keb, uint32_t target_cp)
{
    int count = 0;
    for (uint32_t cp : keb) {
        if (cp == target_cp)
            ++count;
    }
    return count;
}

static bool option_value_missing(int argc, char** argv, int i)
{
    return i + 1 >= argc || argv[i + 1][0] == '-';
}

static bool file_exists(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

int main(int argc, char** argv)
{
    std::string dict_db = "../dict.db";
    std::string jmdict_db = "jmdict.db";
    std::string target;
    bool verbose = false;
    int min_count = 0;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-d") {
            if (option_value_missing(argc, argv, i)) {
                std::cerr << "ERROR\t-d requires dict-db\n";
                return 2;
            }
            dict_db = argv[++i];
        } else if (arg == "-j") {
            if (option_value_missing(argc, argv, i)) {
                std::cerr << "ERROR\t-j requires JMdict-db\n";
                return 2;
            }
            jmdict_db = argv[++i];
        } else if (arg == "-l") {
            if (option_value_missing(argc, argv, i)) {
                std::cerr << "ERROR\t-l requires min-count\n";
                return 2;
            }
            min_count = std::stoi(argv[++i]);
        } else if (arg == "-v") {
            verbose = true;
        } else {
            target = arg;
        }
    }

    if (target.empty()) {
        std::cerr << "usage: dictmatch [-v] [-l min-count] [-d dict-db] [-j JMdict-db] <kanji>\n";
        return 2;
    }

    std::vector<uint32_t> target_cps = to_cps(target);
    if (target_cps.size() != 1) {
        std::cerr << "ERROR\ttarget must be one character\t" << target << "\n";
        return 2;
    }
    uint32_t target_cp = target_cps[0];

    if (!file_exists(dict_db)) {
        std::cerr << "ERROR\tdict db does not exist\t" << dict_db << "\n";
        return 2;
    }

    if (!file_exists(jmdict_db)) {
        std::cerr << "ERROR\tJMdict db does not exist\t" << jmdict_db << "\n";
        return 2;
    }

    sqlite3* db = nullptr;
    if (sqlite3_open_v2(dict_db.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        std::cerr << "ERROR\tcannot open db\t" << dict_db << "\n";
        return 2;
    }

    std::string attach = "attach database '" + jmdict_db + "' as jm";
    if (sqlite3_exec(db, attach.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
        std::cerr << "ERROR\tcannot attach jmdict db\t" << jmdict_db << "\n";
        sqlite3_close(db);
        return 2;
    }

    std::vector<Reading> target_readings = load_target_readings(db, target);
    std::map<std::string, std::vector<std::string>> all_readings = load_all_readings(db);
    std::set<std::string> existing;
    for (const auto& r : target_readings)
        existing.insert(r.yomi);

    std::map<std::string, int> evidence_count;
    std::map<std::string, std::vector<std::string>> evidence_examples;
    std::map<std::string, int> candidate_count;
    std::map<std::string, std::vector<std::string>> candidate_examples;

    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "select keb, reb from jm.jm_pair "
        "where keb like ? "
        "order by keb, reb";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "ERROR\tprepare failed\t(jmdict.jm_pair is not exist ?)\n";
        sqlite3_close(db);
        return 2;
    }

    std::string like = "%" + target + "%";
    sqlite3_bind_text(stmt, 1, like.c_str(), -1, SQLITE_TRANSIENT);

    size_t example_limit = verbose ? 0 : 5;
    int pair_count = 0;
    int aligned_count = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string keb = (const char*)sqlite3_column_text(stmt, 0);
        std::string reb = kata_to_hira((const char*)sqlite3_column_text(stmt, 1));

        std::vector<uint32_t> keb_cps = to_cps(keb);
        if (count_target(keb_cps, target_cp) != 1)
            continue;

        ++pair_count;
        std::vector<uint32_t> reb_cps = to_cps(reb);
        std::set<std::string> segments;

        if (!align_one(keb_cps, reb_cps, 0, 0, target_cp, all_readings, segments))
            continue;

        ++aligned_count;
        for (const auto& seg : segments) {
            candidate_count[seg]++;
            add_example(candidate_examples, seg, keb + "/" + reb, example_limit);
            if (existing.count(seg)) {
                evidence_count[seg]++;
                add_example(evidence_examples, seg, keb + "/" + reb, example_limit);
            }
        }
    }
    sqlite3_finalize(stmt);

    std::cout << "target\t" << target << "\n";
    std::cout << "jm_pair\t" << pair_count << "\n";
    std::cout << "aligned\t" << aligned_count << "\n";
    std::cout << "\n[readings]\n";

    bool has_issue = false;
    for (const auto& r : target_readings) {
        if (evidence_count[r.yomi] > 0) {
            std::cout << r.tp << "\t" << r.yomi << "\tOK\t" << evidence_count[r.yomi] << "\n";
            print_examples(r.yomi, evidence_examples[r.yomi], false);
        } else {
            std::cout << r.tp << "\t" << r.yomi << "\tNO_EVIDENCE\t0\n";
            std::cerr << "NO_EVIDENCE\t" << target << "\t" << r.tp << "\t" << r.yomi << "\n";
            has_issue = true;
        }
    }

    std::cout << "\n[candidates]\n";
    for (const auto& p : candidate_count) {
        bool missing = !existing.count(p.first);
        if (missing && p.second < min_count)
            continue;

        if (!missing) {
            std::cerr << "EXIST\t" << target << "\t" << p.first << "\t" << p.second << "\n";
        } else {
            std::cerr << "MISSING_CANDIDATE\t" << target << "\t" << p.first << "\t" << p.second << "\n";
            has_issue = true;
        }
        print_examples(p.first, candidate_examples[p.first], missing);
    }

    sqlite3_close(db);
    return has_issue ? 1 : 0;
}
