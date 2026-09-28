#include "../include/Repository.h"
#include "../include/Utils.h"

#include <iostream>

namespace fs = std::filesystem;

void Repository::status() const {
    const auto bs = branches();
    const auto adds = stageAdd();
    const auto rms = stageRm();
    const Commit current = headCommit();

    std::cout << "=== Branches ===\n";
    for (const auto& [name, _] : bs) {
        if (name == currentBranch()) std::cout << '*';
        std::cout << name << '\n';
    }
    std::cout << "\n=== Staged Files ===\n";
    for (const auto& [name, _] : adds) std::cout << name << '\n';
    std::cout << "\n=== Removed Files ===\n";
    for (const auto& name : rms) std::cout << name << '\n';

    std::map<std::string, std::string> modifications;
    std::set<std::string> candidates;
    for (const auto& [name, _] : current.files) candidates.insert(name);
    for (const auto& [name, _] : adds) candidates.insert(name);

    for (const auto& name : candidates) {
        const fs::path work = root_ / name;
        if (adds.count(name)) {
            if (!fs::is_regular_file(work)) {
                modifications[name] = "deleted";
            } else {
                const std::string blob = Utils::sha1(Utils::readFile(work.string()));
                if (blob != adds.at(name)) modifications[name] = "modified";
            }
        } else if (current.files.count(name) && !rms.count(name)) {
            if (!fs::is_regular_file(work)) {
                modifications[name] = "deleted";
            } else {
                const std::string blob = Utils::sha1(Utils::readFile(work.string()));
                if (blob != current.files.at(name)) modifications[name] = "modified";
            }
        }
    }

    std::cout << "\n=== Modifications Not Staged For Commit ===\n";
    for (const auto& [name, kind] : modifications) std::cout << name << " (" << kind << ")\n";

    std::cout << "\n=== Untracked Files ===\n";
    for (const auto& name : Utils::plainFilenamesIn(root_.string())) {
        if (adds.count(name)) continue;
        const bool tracked = current.files.count(name) != 0;
        if (tracked && !rms.count(name)) continue;
        if (tracked && rms.count(name)) {
            std::cout << name << '\n';
        } else if (!isIgnored(name)) {
            std::cout << name << '\n';
        }
    }
}

void Repository::branch(const std::string& name) {
    auto bs = branches();
    if (bs.count(name) || remotes().count(name)) {
        Utils::exitWithMessage("A branch with that name already exists.");
    }
    bs[name] = headId();
    saveBranches(bs);
}

void Repository::rmBranch(const std::string& name) {
    auto bs = branches();
    if (!bs.count(name)) Utils::exitWithMessage("A branch with that name does not exist.");
    if (name == currentBranch()) Utils::exitWithMessage("Cannot remove the current branch.");
    bs.erase(name);
    saveBranches(bs);
}
