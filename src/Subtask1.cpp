#include "../include/Repository.h"
#include "../include/Utils.h"

#include <ctime>

namespace fs = std::filesystem;

void Repository::init() {
    if (fs::exists(gitDir_)) {
        Utils::exitWithMessage("A Gitlite version-control system already exists in the current directory.");
    }
    fs::create_directories(commitsDir());
    fs::create_directories(blobsDir());
    Utils::writeMap(tagsFile().string(), {});
    Utils::writeMap(remotesFile().string(), {});
    clearStage();

    Commit initial;
    initial.message = "initial commit";
    initial.timestamp = 0;
    const std::string initialId = storeCommit(initial);
    saveBranches({{"master", initialId}});
    Utils::writeFile(headFile().string(), "master\n");
}


void Repository::add(const std::string& file) {
    const fs::path work = root_ / file;
    if (!fs::is_regular_file(work)) Utils::exitWithMessage("File does not exist.");

    auto adds = stageAdd();
    auto rms = stageRm();
    const Commit current = headCommit();
    const bool tracked = current.files.count(file) != 0;
    const bool alreadyStaged = adds.count(file) != 0;
    if (!tracked && !alreadyStaged && isIgnored(file)) return;

    const std::string data = Utils::readFile(work.string());
    const std::string blob = storeBlob(data);
    auto curIt = current.files.find(file);
    if (curIt != current.files.end() && curIt->second == blob) {
        adds.erase(file);
        rms.erase(file);
    } else {
        adds[file] = blob;
        rms.erase(file);
    }
    saveStage(adds, rms);
}

std::string Repository::createCommit(const std::string& message,
                                     const std::optional<std::string>& secondParent,
                                     bool allowEmpty) {
    if (message.empty()) Utils::exitWithMessage("Please enter a commit message.");
    auto adds = stageAdd();
    auto rms = stageRm();
    if (!allowEmpty && adds.empty() && rms.empty()) {
        Utils::exitWithMessage("No changes added to the commit.");
    }

    const std::string parentId = headId();
    Commit next = loadCommit(parentId);
    next.message = message;
    next.timestamp = static_cast<std::int64_t>(std::time(nullptr));
    next.parents.clear();
    next.parents.push_back(parentId);
    if (secondParent) next.parents.push_back(*secondParent);
    for (const auto& [name, blob] : adds) next.files[name] = blob;
    for (const auto& name : rms) next.files.erase(name);

    const std::string id = storeCommit(next);
    auto bs = branches();
    bs[currentBranch()] = id;
    saveBranches(bs);
    clearStage();
    return id;
}

void Repository::commit(const std::string& message) { createCommit(message, std::nullopt, false); }

void Repository::rm(const std::string& file) {
    auto adds = stageAdd();
    auto rms = stageRm();
    const Commit current = headCommit();
    const bool staged = adds.count(file) != 0;
    const bool tracked = current.files.count(file) != 0;
    if (!staged && !tracked) Utils::exitWithMessage("No reason to remove the file.");

    if (staged) adds.erase(file);
    if (tracked) {
        rms.insert(file);
        std::error_code ec;
        fs::remove(root_ / file, ec);
    }
    saveStage(adds, rms);
}
