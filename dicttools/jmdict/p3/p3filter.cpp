// p3filter.cpp
// p3/p4 人手レビュー用候補から、直前文字の読み漏れとして説明できる候補を除外する。

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sqlite3.h>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

struct Split {
    std::string prefix;
    std::string base;
};

struct RuleOptions {
    bool prev_reading_leak = true;
    bool n_prefix = true;
    bool small_prefix = true;
    bool obsolete_kana = true;
    bool long_mark = true;
};

struct Example {
    std::string line;
    std::string candidate;
    std::string word;
    std::string reading;
};

struct Explain {
    bool ok = false;
    std::string reason;
    std::string prefix;
    std::string base;
    std::string prev_letter;
    std::string prev_source;
    std::string word;
    std::string reading;
};

struct Block {
    std::string header;
    std::string letter;
    std::string candidate;
    std::string count;
    std::vector<std::string> lines;
    std::vector<Example> examples;
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

static bool starts_with_cps(const std::vector<uint32_t>& text, const std::vector<uint32_t>& prefix)
{
    if (text.size() < prefix.size())
        return false;
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (text[i] != prefix[i])
            return false;
    }
    return true;
}

static bool ends_with_cps(const std::vector<uint32_t>& text, const std::vector<uint32_t>& suffix)
{
    if (text.size() < suffix.size())
        return false;
    size_t off = text.size() - suffix.size();
    for (size_t i = 0; i < suffix.size(); ++i) {
        if (text[off + i] != suffix[i])
            return false;
    }
    return true;
}

static bool starts_with_n(const std::string& text)
{
    std::vector<uint32_t> cps = to_cps(kata_to_hira(text));
    return !cps.empty() && cps.front() == U'ん';
}

static bool starts_with_small_kana(const std::string& text)
{
    std::vector<uint32_t> cps = to_cps(kata_to_hira(text));
    if (cps.empty())
        return false;
    return cps.front() == U'っ' ||
           cps.front() == U'ゃ' || cps.front() == U'ゅ' || cps.front() == U'ょ' ||
           cps.front() == U'ぁ' || cps.front() == U'ぃ' || cps.front() == U'ぅ' ||
           cps.front() == U'ぇ' || cps.front() == U'ぉ';
}

static bool contains_obsolete_kana(const std::string& text)
{
    std::vector<uint32_t> cps = to_cps(kata_to_hira(text));
    for (uint32_t cp : cps) {
        if (cp == U'ゐ' || cp == U'ゑ' || cp == U'を')
            return true;
    }
    return false;
}

static bool contains_long_mark(const std::string& text)
{
    std::vector<uint32_t> cps = to_cps(text);
    for (uint32_t cp : cps) {
        if (cp == U'ー')
            return true;
    }
    return false;
}

static std::string suffix_after_prefix(const std::string& text, const std::string& prefix)
{
    std::vector<uint32_t> tcps = to_cps(text);
    std::vector<uint32_t> pcps = to_cps(prefix);
    if (!starts_with_cps(tcps, pcps) || tcps.size() == pcps.size())
        return "";

    std::vector<uint32_t> rest(tcps.begin() + pcps.size(), tcps.end());
    return cps_to_string(rest);
}

static std::string prefix_before_suffix(const std::string& text, const std::string& suffix)
{
    std::vector<uint32_t> tcps = to_cps(text);
    std::vector<uint32_t> scps = to_cps(suffix);
    if (!ends_with_cps(tcps, scps) || tcps.size() == scps.size())
        return "";

    std::vector<uint32_t> rest(tcps.begin(), tcps.end() - scps.size());
    return cps_to_string(rest);
}

static std::vector<size_t> positions_of(const std::vector<uint32_t>& text, uint32_t target)
{
    std::vector<size_t> out;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == target)
            out.push_back(i);
    }
    return out;
}

static bool renyo_from_okuri(const std::string& okuri, std::string& out)
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
        {U'る', U'り'},
    };

    std::vector<uint32_t> cps = to_cps(kata_to_hira(okuri));
    if (cps.size() != 1)
        return false;

    auto it = renyo.find(cps.front());
    if (it == renyo.end())
        return false;

    out = cp_to_utf8(it->second);
    return true;
}

static bool ichidan_e_from_okuri(const std::string& okuri, std::string& out)
{
    static const std::set<uint32_t> e_dan = {
        U'え', U'け', U'げ', U'せ', U'ぜ', U'て', U'で',
        U'ね', U'へ', U'べ', U'ぺ', U'め', U'れ',
    };

    std::vector<uint32_t> cps = to_cps(kata_to_hira(okuri));
    if (cps.size() < 2 || cps.back() != U'る')
        return false;
    if (e_dan.find(cps[cps.size() - 2]) == e_dan.end())
        return false;

    cps.pop_back();
    out = cps_to_string(cps);
    return true;
}

static bool ichidan_i_from_okuri(const std::string& okuri, std::string& out)
{
    static const std::set<uint32_t> i_dan = {
        U'い', U'き', U'ぎ', U'し', U'じ', U'ち', U'ぢ',
        U'に', U'ひ', U'び', U'ぴ', U'み', U'り',
    };

    std::vector<uint32_t> cps = to_cps(kata_to_hira(okuri));
    if (cps.size() < 2 || cps.back() != U'る')
        return false;
    if (i_dan.find(cps[cps.size() - 2]) == i_dan.end())
        return false;

    cps.pop_back();
    out = cps_to_string(cps);
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

static std::set<std::string> load_leak_prefixes(sqlite3* db, const std::string& letter)
{
    std::set<std::string> out;

    std::set<std::string> readings = load_readings(db, letter);
    std::set<std::string> full_readings = readings;

    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "select distinct yomi, okuri from y_base "
        "where okuri is not null and okuri != '' "
        "and cp = (select cp from k where letter=?)";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return out;

    sqlite3_bind_text(stmt, 1, letter.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string yomi = kata_to_hira((const char*)sqlite3_column_text(stmt, 0));
        std::string okuri = kata_to_hira((const char*)sqlite3_column_text(stmt, 1));
        out.insert(okuri);
        full_readings.insert(yomi + okuri);

        std::string derived;
        if (renyo_from_okuri(okuri, derived)) {
            out.insert(derived);
            full_readings.insert(yomi + derived);
        }
        if (ichidan_e_from_okuri(okuri, derived)) {
            out.insert(derived);
            full_readings.insert(yomi + derived);
        }
        if (ichidan_i_from_okuri(okuri, derived)) {
            out.insert(derived);
            full_readings.insert(yomi + derived);
        }
    }

    sqlite3_finalize(stmt);

    for (const auto& full : full_readings) {
        for (const auto& base : readings) {
            std::string suffix = suffix_after_prefix(full, base);
            if (!suffix.empty())
                out.insert(suffix);
        }
    }

    return out;
}

static std::set<std::string> load_candidate_bases(sqlite3* db, const std::string& letter)
{
    std::set<std::string> out = load_readings(db, letter);
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "select distinct yomi, okuri from y_base "
        "where okuri is not null and okuri != '' "
        "and cp = (select cp from k where letter=?)";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return out;

    sqlite3_bind_text(stmt, 1, letter.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string yomi = kata_to_hira((const char*)sqlite3_column_text(stmt, 0));
        std::string okuri = kata_to_hira((const char*)sqlite3_column_text(stmt, 1));
        std::string derived;

        out.insert(yomi + okuri);

        if (renyo_from_okuri(okuri, derived))
            out.insert(yomi + derived);
        if (ichidan_e_from_okuri(okuri, derived))
            out.insert(yomi + derived);
        if (ichidan_i_from_okuri(okuri, derived))
            out.insert(yomi + derived);
    }

    sqlite3_finalize(stmt);
    return out;
}

static std::vector<Split> candidate_splits(sqlite3* db, const std::string& letter, const std::string& candidate)
{
    std::vector<Split> out;
    for (const auto& base : load_candidate_bases(db, letter)) {
        std::string prefix = prefix_before_suffix(candidate, base);
        if (!prefix.empty())
            out.push_back({prefix, base});
    }
    return out;
}

static bool explain_example(sqlite3* db,
                            const Block& block,
                            const Example& example,
                            const std::vector<Split>& splits,
                            std::map<std::string, std::set<std::string>>& leak_cache,
                            Explain& explain)
{
    std::vector<uint32_t> wcps = to_cps(example.word);
    std::vector<uint32_t> target = to_cps(block.letter);
    if (target.size() != 1)
        return false;

    for (size_t pos : positions_of(wcps, target.front())) {
        if (pos == 0)
            continue;

        std::string prev = cp_to_utf8(wcps[pos - 1]);
        auto it = leak_cache.find(prev);
        if (it == leak_cache.end())
            it = leak_cache.insert({prev, load_leak_prefixes(db, prev)}).first;

        for (const auto& split : splits) {
            if (it->second.find(split.prefix) == it->second.end())
                continue;

            explain.ok = true;
            explain.reason = "prev-reading-leak";
            explain.prefix = split.prefix;
            explain.base = split.base;
            explain.prev_letter = prev;
            explain.prev_source = split.prefix;
            explain.word = example.word;
            explain.reading = example.reading;
            return true;
        }
    }

    return false;
}

static bool should_reject(sqlite3* db,
                          const Block& block,
                          const RuleOptions& rules,
                          std::map<std::string, std::set<std::string>>& leak_cache,
                          std::vector<Explain>& explains)
{
    if (rules.n_prefix && starts_with_n(block.candidate)) {
        Explain explain;
        explain.ok = true;
        explain.reason = "n-prefix";
        explains.push_back(explain);
        return true;
    }

    if (rules.small_prefix && starts_with_small_kana(block.candidate)) {
        Explain explain;
        explain.ok = true;
        explain.reason = "small-prefix";
        explains.push_back(explain);
        return true;
    }

    if (rules.obsolete_kana && contains_obsolete_kana(block.candidate)) {
        Explain explain;
        explain.ok = true;
        explain.reason = "obsolete-kana";
        explains.push_back(explain);
        return true;
    }

    if (rules.long_mark && contains_long_mark(block.candidate)) {
        Explain explain;
        explain.ok = true;
        explain.reason = "long-mark";
        explains.push_back(explain);
        return true;
    }

    if (!rules.prev_reading_leak)
        return false;

    if (block.examples.empty())
        return false;

    std::vector<Split> splits = candidate_splits(db, block.letter, block.candidate);
    if (splits.empty())
        return false;

    for (const auto& example : block.examples) {
        Explain explain;
        if (!explain_example(db, block, example, splits, leak_cache, explain))
            return false;
        explains.push_back(explain);
    }

    return true;
}

static void output_block(std::ostream& out, const Block& block)
{
    out << block.header << "\n";
    for (const auto& line : block.lines)
        out << line << "\n";
}

static void reject_block(std::ostream& out, const Block& block, const std::vector<Explain>& explains)
{
    std::string reason = "PREV_READING_LEAK";
    if (!explains.empty() && explains.front().reason == "n-prefix")
        reason = "N_PREFIX";
    if (!explains.empty() && explains.front().reason == "small-prefix")
        reason = "SMALL_PREFIX";
    if (!explains.empty() && explains.front().reason == "obsolete-kana")
        reason = "OBSOLETE_KANA";
    if (!explains.empty() && explains.front().reason == "long-mark")
        reason = "LONG_MARK";
    size_t shown_examples = reason == "N_PREFIX" || reason == "SMALL_PREFIX" ||
                                    reason == "OBSOLETE_KANA" || reason == "LONG_MARK"
                                ? block.examples.size()
                                : explains.size();

    out << "REJECT_" << reason << "\t"
        << block.letter << "\t"
        << block.candidate << "\t"
        << block.count << "\t"
        << "shown_examples=" << shown_examples << "\n";

    for (const auto& explain : explains) {
        if (explain.reason == "n-prefix") {
            out << "\tR\t"
                << "n-prefix\t"
                << block.candidate << "\n";
            continue;
        }
        if (explain.reason == "small-prefix") {
            out << "\tR\t"
                << "small-prefix\t"
                << block.candidate << "\n";
            continue;
        }
        if (explain.reason == "obsolete-kana") {
            out << "\tR\t"
                << "obsolete-kana\t"
                << block.candidate << "\n";
            continue;
        }
        if (explain.reason == "long-mark") {
            out << "\tR\t"
                << "long-mark\t"
                << block.candidate << "\n";
            continue;
        }

        out << "\tR\t"
            << explain.prefix << "\t"
            << explain.base << "\t"
            << explain.prev_letter << "\t"
            << explain.word << "\t"
            << explain.reading << "\n";
    }

    output_block(out, block);
}

static bool parse_header(const std::string& line, Block& block)
{
    std::vector<std::string> fields = split_tab(line);
    if (fields.size() < 4 || fields[0] != "MISSING_CANDIDATE")
        return false;

    block.header = line;
    block.letter = fields[1];
    block.candidate = kata_to_hira(fields[2]);
    block.count = fields[3];
    block.lines.clear();
    block.examples.clear();
    return true;
}

static void add_line(Block& block, const std::string& line)
{
    block.lines.push_back(line);
    std::vector<std::string> fields = split_tab(line);
    if (fields.size() >= 5 && fields[0].empty() && fields[1] == "T") {
        Example ex;
        ex.line = line;
        ex.candidate = kata_to_hira(fields[2]);
        ex.word = fields[3];
        ex.reading = kata_to_hira(fields[4]);
        block.examples.push_back(ex);
    }
}

static void usage()
{
    std::cerr << "usage: p3filter [-d DB] [-r prev-reading-leak|n-prefix|small-prefix|obsolete-kana|long-mark|all] [--reject FILE] < raw.tsv > filtered.tsv\n";
}

static bool parse_rules(const std::string& text, RuleOptions& options)
{
    options.prev_reading_leak = false;
    options.n_prefix = false;
    options.small_prefix = false;
    options.obsolete_kana = false;
    options.long_mark = false;

    for (const auto& rule : split_comma(text)) {
        if (rule == "prev-reading-leak") {
            options.prev_reading_leak = true;
        } else if (rule == "n-prefix") {
            options.n_prefix = true;
        } else if (rule == "small-prefix") {
            options.small_prefix = true;
        } else if (rule == "obsolete-kana") {
            options.obsolete_kana = true;
        } else if (rule == "long-mark") {
            options.long_mark = true;
        } else if (rule == "all") {
            options.prev_reading_leak = true;
            options.n_prefix = true;
            options.small_prefix = true;
            options.obsolete_kana = true;
            options.long_mark = true;
        } else {
            return false;
        }
    }

    return options.prev_reading_leak || options.n_prefix || options.small_prefix ||
           options.obsolete_kana || options.long_mark;
}

int main(int argc, char** argv)
{
    std::string db_path = "../p2/dict-jmdict-p2.db";
    std::string reject_path;
    RuleOptions rules;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-d" && i + 1 < argc) {
            db_path = argv[++i];
        } else if (arg == "-r" && i + 1 < argc) {
            if (!parse_rules(argv[++i], rules)) {
                std::cerr << "ERROR\tunknown p3filter rule\n";
                usage();
                return 2;
            }
        } else if (arg == "--reject" && i + 1 < argc) {
            reject_path = argv[++i];
        } else {
            usage();
            return 2;
        }
    }

    sqlite3* db = nullptr;
    if (sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        std::cerr << "ERROR\tcannot open db\t" << db_path << "\n";
        return 2;
    }

    std::ofstream reject_file;
    std::ostream* reject_out = nullptr;
    if (!reject_path.empty()) {
        reject_file.open(reject_path.c_str());
        if (!reject_file) {
            std::cerr << "ERROR\tcannot open reject file\t" << reject_path << "\n";
            sqlite3_close(db);
            return 2;
        }
        reject_out = &reject_file;
    }

    std::string line;
    Block block;
    bool have_block = false;
    std::map<std::string, std::set<std::string>> leak_cache;

    auto flush = [&]() {
        if (!have_block)
            return;

        std::vector<Explain> explains;
        if (should_reject(db, block, rules, leak_cache, explains)) {
            if (reject_out)
                reject_block(*reject_out, block, explains);
        } else {
            output_block(std::cout, block);
        }
    };

    while (std::getline(std::cin, line)) {
        Block next;
        if (parse_header(line, next)) {
            flush();
            block = next;
            have_block = true;
        } else if (have_block) {
            add_line(block, line);
        } else {
            std::cout << line << "\n";
        }
    }
    flush();

    sqlite3_close(db);
    return 0;
}
