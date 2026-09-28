#include "../include/Repository.h"
#include "../include/Utils.h"

#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>

namespace fs = std::filesystem;

namespace {
bool startsWith(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

std::string readTextTrimNewline(const fs::path& p) {
    std::string s = Utils::readFile(p.string());
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
    return s;
}
}

Repository::Repository() : root_(fs::current_path()), gitDir_(root_ / ".gitlite") {}

std::string Repository::getGitliteDir() { return ".gitlite"; }

fs::path Repository::commitsDir() const { return gitDir_ / "objects" / "commits"; }
fs::path Repository::blobsDir() const { return gitDir_ / "objects" / "blobs"; }
fs::path Repository::branchesFile() const { return gitDir_ / "branches"; }
fs::path Repository::tagsFile() const { return gitDir_ / "tags"; }
fs::path Repository::remotesFile() const { return gitDir_ / "remotes"; }
fs::path Repository::headFile() const { return gitDir_ / "HEAD"; }
fs::path Repository::stageAddFile() const { return gitDir_ / "stage_add"; }
fs::path Repository::stageRmFile() const { return gitDir_ / "stage_rm"; }

std::map<std::string, std::string> Repository::branches() const { return Utils::readMap(branchesFile().string()); }
std::map<std::string, std::string> Repository::tags() const { return Utils::readMap(tagsFile().string()); }
std::map<std::string, std::string> Repository::remotes() const { return Utils::readMap(remotesFile().string()); }
std::map<std::string, std::string> Repository::stageAdd() const { return Utils::readMap(stageAddFile().string()); }
std::set<std::string> Repository::stageRm() const { return Utils::readSet(stageRmFile().string()); }

void Repository::saveBranches(const std::map<std::string, std::string>& values) const { Utils::writeMap(branchesFile().string(), values); }
void Repository::saveTags(const std::map<std::string, std::string>& values) const { Utils::writeMap(tagsFile().string(), values); }
void Repository::saveRemotes(const std::map<std::string, std::string>& values) const { Utils::writeMap(remotesFile().string(), values); }
void Repository::saveStage(const std::map<std::string, std::string>& adds, const std::set<std::string>& rms) const {
    Utils::writeMap(stageAddFile().string(), adds);
    Utils::writeSet(stageRmFile().string(), rms);
}
void Repository::clearStage() const { saveStage({}, {}); }

std::string Repository::currentBranch() const { return readTextTrimNewline(headFile()); }

std::string Repository::headId() const {
    const auto bs = branches();
    const auto it = bs.find(currentBranch());
    return it == bs.end() ? std::string() : it->second;
}

Commit Repository::headCommit() const { return loadCommit(headId()); }

Commit Repository::loadCommit(const std::string& id) const {
    return Commit::deserialize(Utils::readFile((commitsDir() / id).string()));
}

std::string Repository::storeCommit(const Commit& commit) const {
    const std::string data = commit.serialize();
    const std::string id = Utils::sha1(data);
    const fs::path path = commitsDir() / id;
    if (!fs::exists(path)) Utils::writeFile(path.string(), data);
    return id;
}

std::string Repository::storeBlob(const std::string& data) const {
    const std::string id = Utils::sha1(data);
    const fs::path path = blobsDir() / id;
    if (!fs::exists(path)) Utils::writeFile(path.string(), data);
    return id;
}

std::string Repository::loadBlob(const std::string& id) const {
    return Utils::readFile((blobsDir() / id).string());
}

bool Repository::globMatch(const std::string& pattern, const std::string& text) {
    struct Token { char kind; char ch; };
    std::vector<Token> tokens;
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        char c = pattern[i];
        if (c == '\\') {
            if (i + 1 < pattern.size()) tokens.push_back({'L', pattern[++i]});
            else tokens.push_back({'L', '\\'});
        } else if (c == '*') {
            if (tokens.empty() || tokens.back().kind != '*') tokens.push_back({'*', 0});
        } else if (c == '?') {
            tokens.push_back({'?', 0});
        } else {
            tokens.push_back({'L', c});
        }
    }

    std::vector<std::vector<int>> memo(tokens.size() + 1, std::vector<int>(text.size() + 1, -1));
    std::function<bool(std::size_t, std::size_t)> solve = [&](std::size_t i, std::size_t j) -> bool {
        int& cell = memo[i][j];
        if (cell != -1) return cell != 0;
        bool ans = false;
        if (i == tokens.size()) {
            ans = (j == text.size());
        } else if (tokens[i].kind == '*') {
            ans = solve(i + 1, j) || (j < text.size() && solve(i, j + 1));
        } else if (j < text.size()) {
            if (tokens[i].kind == '?') ans = solve(i + 1, j + 1);
            else ans = (tokens[i].ch == text[j]) && solve(i + 1, j + 1);
        }
        cell = ans ? 1 : 0;
        return ans;
    };
    return solve(0, 0);
}

bool Repository::isIgnored(const std::string& filename) const {
    const fs::path ignorePath = root_ / ".gitliteignore";
    if (!fs::is_regular_file(ignorePath)) return false;
    std::ifstream in(ignorePath, std::ios::binary);
    std::string line;
    bool ignored = false;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        if (line[0] == '#') continue;
        bool negated = false;
        if (!line.empty() && line[0] == '!') {
            negated = true;
            line.erase(line.begin());
        }
        if (!line.empty() && line[0] == '/') line.erase(line.begin());
        if (globMatch(line, filename)) ignored = !negated;
    }
    return ignored;
}

std::string Repository::formatTimestamp(std::int64_t timestamp) {
    std::time_t t = static_cast<std::time_t>(timestamp);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buffer[128];
    std::strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y %z", &tm);
    return buffer;
}

void Repository::printCommit(const std::string& id, const Commit& commitObj) const {
    std::cout << "===\n";
    std::cout << "commit " << id << "\n";
    if (commitObj.parents.size() == 2) {
        std::cout << "Merge: " << commitObj.parents[0].substr(0, 7)
                  << ' ' << commitObj.parents[1].substr(0, 7) << "\n";
    }
    std::cout << "Date: " << formatTimestamp(commitObj.timestamp) << "\n";
    std::cout << commitObj.message << "\n\n";
}


std::string Repository::resolveRevision(const std::string& revision) const {
    if (revision.empty()) return {};

    const auto ts = tags();
    auto tagIt = ts.find(revision);
    if (tagIt != ts.end()) return tagIt->second;

    std::string best;
    for (const auto& id : Utils::plainFilenamesIn(commitsDir().string())) {
        if (startsWith(id, revision) && (best.empty() || id < best)) best = id;
    }
    return best;
}

bool Repository::hasBlockingUntracked(const Commit& current, const Commit& target,
                                      const std::set<std::string>* onlyPaths) const {
    const auto adds = stageAdd();
    for (const auto& [name, _] : target.files) {
        if (onlyPaths && !onlyPaths->count(name)) continue;
        const fs::path work = root_ / name;
        if (!fs::is_regular_file(work)) continue;
        if (current.files.count(name)) continue;
        if (adds.count(name)) continue;
        if (isIgnored(name)) continue;
        return true;
    }
    return false;
}

void Repository::writeWorkingFile(const std::string& file, const std::string& blobId) const {
    Utils::writeFile((root_ / file).string(), loadBlob(blobId));
}

void Repository::checkoutSnapshot(const Commit& current, const Commit& target) const {
    for (const auto& [name, blob] : target.files) writeWorkingFile(name, blob);
    for (const auto& [name, _] : current.files) {
        if (!target.files.count(name)) {
            std::error_code ec;
            fs::remove(root_ / name, ec);
        }
    }
}
