// p1filter.cpp
// p1 自動登録用の保守的な読み候補フィルタ。

#include <iostream>
#include <map>
#include <set>
#include <sqlite3.h>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

struct RuleOptions {
    bool onbin = true;
    bool okuri = false;
    bool renyo = false;
    bool renyo_ru = false;
    bool renyo_ichidan_e = false;
    bool renyo_rendaku = false;
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

static std::string cps_to_string(const std::vector<uint32_t>& cps)
{
    std::string out;
    for (uint32_t cp : cps)
        out += cp_to_utf8(cp);
    return out;
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

static std::vector<std::string> split_tab(const std::string& line)
{
    std::vector<std::string> out;
    std::string field;
    std::istringstream in(line);
    while (std::getline(in, field, '\t'))
        out.push_back(field);
    return out;
}

static std::vector<std::string> split_comma(const std::string& text)
{
    std::vector<std::string> out;
    std::string field;
    std::istringstream in(text);
    while (std::getline(in, field, ','))
        out.push_back(field);
    return out;
}

static bool sokuonize(const std::string& base, std::string& out)
{
    std::vector<uint32_t> cps = to_cps(base);
    if (cps.empty())
        return false;

    uint32_t& last = cps.back();
    if (last == U'つ' || last == U'ち' || last == U'き' || last == U'く') {
        last = U'っ';
        out = cps_to_string(cps);
        return true;
    }
    return false;
}

static bool dakuonize_head(const std::string& base, std::string& out)
{
    static const std::map<uint32_t, uint32_t> dakuon = {
        {U'か', U'が'}, {U'き', U'ぎ'}, {U'く', U'ぐ'}, {U'け', U'げ'}, {U'こ', U'ご'},
        {U'さ', U'ざ'}, {U'し', U'じ'}, {U'す', U'ず'}, {U'せ', U'ぜ'}, {U'そ', U'ぞ'},
        {U'た', U'だ'}, {U'ち', U'ぢ'}, {U'つ', U'づ'}, {U'て', U'で'}, {U'と', U'ど'},
        {U'は', U'ば'}, {U'ひ', U'び'}, {U'ふ', U'ぶ'}, {U'へ', U'べ'}, {U'ほ', U'ぼ'},
    };

    std::vector<uint32_t> cps = to_cps(base);
    if (cps.empty())
        return false;

    auto it = dakuon.find(cps.front());
    if (it == dakuon.end())
        return false;

    cps.front() = it->second;
    out = cps_to_string(cps);
    return true;
}

static bool handakuonize_head(const std::string& base, std::string& out)
{
    static const std::map<uint32_t, uint32_t> handakuon = {
        {U'は', U'ぱ'}, {U'ひ', U'ぴ'}, {U'ふ', U'ぷ'}, {U'へ', U'ぺ'}, {U'ほ', U'ぽ'},
    };

    std::vector<uint32_t> cps = to_cps(base);
    if (cps.empty())
        return false;

    auto it = handakuon.find(cps.front());
    if (it == handakuon.end())
        return false;

    cps.front() = it->second;
    out = cps_to_string(cps);
    return true;
}

static bool renyo_candidate(const std::string& yomi, const std::string& okuri, std::string& out)
{
    static const std::map<uint32_t, uint32_t> renyo = {
        {U'う', U'い'},
        {U'く', U'き'},
        {U'ぐ', U'ぎ'},
        {U'す', U'し'},
        {U'つ', U'ち'},
        {U'ぬ', U'に'},
        {U'ぶ', U'び'},
        {U'む', U'み'},
    };

    std::vector<uint32_t> okuri_cps = to_cps(kata_to_hira(okuri));
    if (okuri_cps.size() != 1)
        return false;

    auto it = renyo.find(okuri_cps.front());
    if (it == renyo.end())
        return false;

    out = kata_to_hira(yomi) + cp_to_utf8(it->second);
    return true;
}

static bool renyo_ru_candidate(const std::string& yomi, const std::string& okuri, std::string& out)
{
    std::vector<uint32_t> okuri_cps = to_cps(kata_to_hira(okuri));
    if (okuri_cps.size() != 1 || okuri_cps.front() != U'る')
        return false;

    out = kata_to_hira(yomi) + cp_to_utf8(U'り');
    return true;
}

static bool is_e_dan(uint32_t cp)
{
    return cp == U'え' || cp == U'け' || cp == U'げ' || cp == U'せ' || cp == U'ぜ' ||
           cp == U'て' || cp == U'で' || cp == U'ね' || cp == U'へ' || cp == U'べ' ||
           cp == U'ぺ' || cp == U'め' || cp == U'れ';
}

static bool renyo_ichidan_e_candidate(const std::string& yomi, const std::string& okuri, std::string& out)
{
    std::vector<uint32_t> okuri_cps = to_cps(kata_to_hira(okuri));
    if (okuri_cps.size() < 2 || okuri_cps.back() != U'る')
        return false;
    if (!is_e_dan(okuri_cps[okuri_cps.size() - 2]))
        return false;

    okuri_cps.pop_back();
    out = kata_to_hira(yomi) + cps_to_string(okuri_cps);
    return true;
}

static bool okuri_candidate(const std::string& yomi, const std::string& okuri, std::string& out)
{
    if (okuri.empty())
        return false;

    out = kata_to_hira(yomi) + kata_to_hira(okuri);
    return true;
}

static std::set<std::string> load_readings(sqlite3* db, const std::string& letter)
{
    std::set<std::string> out;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "select distinct yomi from y where letter=?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return out;

    sqlite3_bind_text(stmt, 1, letter.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW)
        out.insert(kata_to_hira((const char*)sqlite3_column_text(stmt, 0)));

    sqlite3_finalize(stmt);
    return out;
}

static std::set<std::string> load_renyo_readings(
    sqlite3* db,
    const std::string& letter,
    bool include_regular,
    bool include_ru,
    bool include_ichidan_e,
    bool include_rendaku)
{
    std::set<std::string> out;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "select distinct yomi, okuri from y_base where okuri is not null and okuri != '' and cp = (select cp from k where letter=?)";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return out;

    sqlite3_bind_text(stmt, 1, letter.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string yomi = (const char*)sqlite3_column_text(stmt, 0);
        std::string okuri = (const char*)sqlite3_column_text(stmt, 1);
        std::string derived;
        if (include_regular && renyo_candidate(yomi, okuri, derived))
            out.insert(derived);
        if (include_rendaku && renyo_candidate(yomi, okuri, derived)) {
            std::string voiced;
            if (dakuonize_head(derived, voiced))
                out.insert(voiced);
            if (handakuonize_head(derived, voiced))
                out.insert(voiced);
        }
        if (include_ru && renyo_ru_candidate(yomi, okuri, derived))
            out.insert(derived);
        if (include_rendaku && renyo_ru_candidate(yomi, okuri, derived)) {
            std::string voiced;
            if (dakuonize_head(derived, voiced))
                out.insert(voiced);
            if (handakuonize_head(derived, voiced))
                out.insert(voiced);
        }
        if (include_ichidan_e && renyo_ichidan_e_candidate(yomi, okuri, derived))
            out.insert(derived);
        if (include_rendaku && renyo_ichidan_e_candidate(yomi, okuri, derived)) {
            std::string voiced;
            if (dakuonize_head(derived, voiced))
                out.insert(voiced);
            if (handakuonize_head(derived, voiced))
                out.insert(voiced);
        }
    }

    sqlite3_finalize(stmt);
    return out;
}

static std::set<std::string> load_okuri_readings(sqlite3* db, const std::string& letter)
{
    std::set<std::string> out;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "select distinct yomi, okuri from y_base where okuri is not null and okuri != '' and cp = (select cp from k where letter=?)";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return out;

    sqlite3_bind_text(stmt, 1, letter.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string yomi = (const char*)sqlite3_column_text(stmt, 0);
        std::string okuri = (const char*)sqlite3_column_text(stmt, 1);
        std::string derived;
        if (okuri_candidate(yomi, okuri, derived))
            out.insert(derived);
    }

    sqlite3_finalize(stmt);
    return out;
}

static bool accepted_by_onbin_rule(const std::set<std::string>& readings, const std::string& candidate)
{
    std::string derived;
    for (const auto& base : readings) {
        if (sokuonize(base, derived) && derived == candidate)
            return true;
        if (dakuonize_head(base, derived) && derived == candidate)
            return true;
        if (handakuonize_head(base, derived) && derived == candidate)
            return true;
    }
    return false;
}

static bool parse_rules(const std::string& text, RuleOptions& options)
{
    options.onbin = false;
    options.okuri = false;
    options.renyo = false;
    options.renyo_ru = false;
    options.renyo_ichidan_e = false;
    options.renyo_rendaku = false;

    for (const auto& rule : split_comma(text)) {
        if (rule == "onbin") {
            options.onbin = true;
        } else if (rule == "okuri") {
            options.okuri = true;
        } else if (rule == "renyo") {
            options.renyo = true;
        } else if (rule == "renyo-ru") {
            options.renyo_ru = true;
        } else if (rule == "renyo-ichidan-e") {
            options.renyo_ichidan_e = true;
        } else if (rule == "renyo-rendaku") {
            options.renyo_rendaku = true;
        } else if (rule == "all") {
            options.onbin = true;
            options.okuri = true;
            options.renyo = true;
            options.renyo_ru = true;
            options.renyo_ichidan_e = true;
            options.renyo_rendaku = true;
        } else {
            return false;
        }
    }
    return options.onbin || options.okuri || options.renyo || options.renyo_ru ||
           options.renyo_ichidan_e || options.renyo_rendaku;
}

int main(int argc, char** argv)
{
    std::string db_path = "dict-jmdict-p1.db";
    RuleOptions rules;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-d" && i + 1 < argc)
            db_path = argv[++i];
        else if (arg == "-r" && i + 1 < argc) {
            if (!parse_rules(argv[++i], rules)) {
                std::cerr << "ERROR\tunknown p1filter rule\n";
                std::cerr << "usage: p1filter [-d DB] [-r onbin|okuri|renyo|renyo-ru|renyo-ichidan-e|renyo-rendaku|all] < raw.tsv > accept.tsv\n";
                return 2;
            }
        }
        else {
            std::cerr << "usage: p1filter [-d DB] [-r onbin|okuri|renyo|renyo-ru|renyo-ichidan-e|renyo-rendaku|all] < raw.tsv > accept.tsv\n";
            return 2;
        }
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "ERROR\tcannot open db\t" << db_path << "\n";
        return 2;
    }

    std::string line;
    std::map<std::string, std::set<std::string>> reading_cache;
    std::map<std::string, std::set<std::string>> okuri_cache;
    std::map<std::string, std::set<std::string>> renyo_cache;
    while (std::getline(std::cin, line)) {
        std::vector<std::string> fields = split_tab(line);
        if (fields.size() < 4 || fields[0] != "MISSING_CANDIDATE")
            continue;

        const std::string& letter = fields[1];
        std::string candidate = kata_to_hira(fields[2]);

        bool accepted = false;
        if (rules.onbin) {
            auto it = reading_cache.find(letter);
            if (it == reading_cache.end())
                it = reading_cache.insert({letter, load_readings(db, letter)}).first;

            accepted = accepted_by_onbin_rule(it->second, candidate);
        }

        if (!accepted && (rules.renyo || rules.renyo_ru || rules.renyo_ichidan_e || rules.renyo_rendaku)) {
            auto it = renyo_cache.find(letter);
            if (it == renyo_cache.end())
                it = renyo_cache.insert({
                    letter,
                    load_renyo_readings(db, letter, rules.renyo, rules.renyo_ru,
                                        rules.renyo_ichidan_e, rules.renyo_rendaku)
                }).first;

            accepted = it->second.find(candidate) != it->second.end();
        }

        if (!accepted && rules.okuri) {
            auto it = okuri_cache.find(letter);
            if (it == okuri_cache.end())
                it = okuri_cache.insert({letter, load_okuri_readings(db, letter)}).first;

            accepted = it->second.find(candidate) != it->second.end();
        }

        if (accepted)
            std::cout << line << "\n";
    }

    sqlite3_close(db);
    return 0;
}
