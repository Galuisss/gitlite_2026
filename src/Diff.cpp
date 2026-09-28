#include "../include/Repository.h"
#include "../include/Utils.h"

#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

std::vector<Repository::DiffLine> Repository::splitLines(const std::string& content) {
    std::vector<DiffLine> lines;
    std::size_t start = 0;
    while (start < content.size()) {
        const std::size_t nl = content.find('\n', start);
        if (nl == std::string::npos) {
            lines.push_back({content.substr(start), false});
            break;
        }
        lines.push_back({content.substr(start, nl - start), true});
        start = nl + 1;
    }
    return lines;
}

std::vector<Repository::DiffOp> Repository::buildDiffOps(const std::vector<DiffLine>& oldLines,
                                                         const std::vector<DiffLine>& newLines) {
    const int n = static_cast<int>(oldLines.size());
    const int m = static_cast<int>(newLines.size());
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));
    auto equal = [](const DiffLine& a, const DiffLine& b) {
        return a.text == b.text && a.newline == b.newline;
    };
    for (int i = n - 1; i >= 0; --i) {
        for (int j = m - 1; j >= 0; --j) {
            if (equal(oldLines[i], newLines[j])) dp[i][j] = 1 + dp[i + 1][j + 1];
            else dp[i][j] = std::max(dp[i + 1][j], dp[i][j + 1]);
        }
    }

    std::vector<DiffOp> ops;
    int i = 0, j = 0;
    while (i < n || j < m) {
        if (i < n && j < m && equal(oldLines[i], newLines[j])) {
            ops.push_back({' ', oldLines[i], i + 1, j + 1});
            ++i; ++j;
        } else if (i < n && (j == m || dp[i + 1][j] >= dp[i][j + 1])) {
            ops.push_back({'-', oldLines[i], i + 1, 0});
            ++i;
        } else {
            ops.push_back({'+', newLines[j], 0, j + 1});
            ++j;
        }
    }
    return ops;
}

void Repository::printFileDiff(const std::string& file,
                               bool oldPresent, const std::string& oldContent,
                               bool newPresent, const std::string& newContent) {
    if (oldPresent == newPresent && oldContent == newContent) return;

    std::cout << "diff --gitlite a/" << file << " b/" << file << "\n";
    std::cout << "--- " << (oldPresent ? "a/" + file : std::string("/dev/null")) << "\n";
    std::cout << "+++ " << (newPresent ? "b/" + file : std::string("/dev/null")) << "\n";

    const auto oldLines = splitLines(oldContent);
    const auto newLines = splitLines(newContent);
    const auto ops = buildDiffOps(oldLines, newLines);
    std::vector<int> changes;
    for (int idx = 0; idx < static_cast<int>(ops.size()); ++idx) {
        if (ops[idx].type != ' ') changes.push_back(idx);
    }
    if (changes.empty()) return;

    std::vector<std::pair<int, int>> ranges;
    for (int change : changes) {
        int left = change;
        int contexts = 0;
        for (int k = change - 1; k >= 0 && contexts < 3; --k) {
            left = k;
            if (ops[k].type == ' ') ++contexts;
        }
        int right = change;
        contexts = 0;
        for (int k = change + 1; k < static_cast<int>(ops.size()) && contexts < 3; ++k) {
            right = k;
            if (ops[k].type == ' ') ++contexts;
        }
        if (ranges.empty() || left > ranges.back().second + 1) ranges.push_back({left, right});
        else ranges.back().second = std::max(ranges.back().second, right);
    }

    for (const auto& [l, r] : ranges) {
        int oldCount = 0, newCount = 0, oldStart = 0, newStart = 0;
        for (int k = l; k <= r; ++k) {
            if (ops[k].type != '+') {
                ++oldCount;
                if (oldStart == 0) oldStart = ops[k].oldNo;
            }
            if (ops[k].type != '-') {
                ++newCount;
                if (newStart == 0) newStart = ops[k].newNo;
            }
        }
        if (oldCount == 0) oldStart = 0;
        if (newCount == 0) newStart = 0;
        std::cout << "@@ -" << oldStart << ',' << oldCount
                  << " +" << newStart << ',' << newCount << " @@\n";
        for (int k = l; k <= r; ++k) {
            std::cout << ops[k].type << ops[k].line.text << '\n';
            if (!ops[k].line.newline) std::cout << "\\ No newline at end of file\n";
        }
    }
}

void Repository::diffCommitToWorking(const Commit& oldCommit) const {
    for (const auto& [name, blob] : oldCommit.files) {
        const std::string oldContent = loadBlob(blob);
        const fs::path work = root_ / name;
        const bool newPresent = fs::is_regular_file(work);
        const std::string newContent = newPresent ? Utils::readFile(work.string()) : std::string();
        printFileDiff(name, true, oldContent, newPresent, newContent);
    }
}

void Repository::diffCommits(const Commit& oldCommit, const Commit& newCommit) const {
    std::set<std::string> names;
    for (const auto& [n, _] : oldCommit.files) names.insert(n);
    for (const auto& [n, _] : newCommit.files) names.insert(n);
    for (const auto& name : names) {
        const auto oi = oldCommit.files.find(name);
        const auto ni = newCommit.files.find(name);
        const bool oldPresent = oi != oldCommit.files.end();
        const bool newPresent = ni != newCommit.files.end();
        const std::string oldContent = oldPresent ? loadBlob(oi->second) : std::string();
        const std::string newContent = newPresent ? loadBlob(ni->second) : std::string();
        printFileDiff(name, oldPresent, oldContent, newPresent, newContent);
    }
}

void Repository::diff() const { diffCommitToWorking(headCommit()); }

void Repository::diffWithCommit(const std::string& revision) const {
    const std::string id = resolveRevision(revision);
    if (id.empty()) Utils::exitWithMessage("No commit with that id exists.");
    diffCommitToWorking(loadCommit(id));
}

void Repository::diffBetween(const std::string& rev1, const std::string& rev2) const {
    const std::string id1 = resolveRevision(rev1);
    if (id1.empty()) Utils::exitWithMessage("No commit with that id exists.");
    const std::string id2 = resolveRevision(rev2);
    if (id2.empty()) Utils::exitWithMessage("No commit with that id exists.");
    diffCommits(loadCommit(id1), loadCommit(id2));
}

void Repository::show() const {
    const std::string id = headId();
    const Commit c = loadCommit(id);
    printCommit(id, c);
    Commit parent;
    if (!c.parents.empty()) parent = loadCommit(c.parents[0]);
    diffCommits(parent, c);
}

void Repository::show(const std::string& revision) const {
    const std::string id = resolveRevision(revision);
    if (id.empty()) Utils::exitWithMessage("No commit with that id exists.");
    const Commit c = loadCommit(id);
    printCommit(id, c);
    Commit parent;
    if (!c.parents.empty()) parent = loadCommit(c.parents[0]);
    diffCommits(parent, c);
}
