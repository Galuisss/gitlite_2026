#include "../include/Repository.h"
#include "../include/Utils.h"

namespace {
const char* kUntrackedError = "There is an untracked file in the way; delete it, or add and commit it first.";
}

void Repository::checkoutBranch(const std::string& name) {
    auto bs = branches();
    auto it = bs.find(name);
    if (it == bs.end()) Utils::exitWithMessage("No such branch exists.");
    if (name == currentBranch()) Utils::exitWithMessage("No need to checkout the current branch.");

    const Commit current = headCommit();
    const Commit target = loadCommit(it->second);
    if (hasBlockingUntracked(current, target)) Utils::exitWithMessage(kUntrackedError);
    checkoutSnapshot(current, target);
    Utils::writeFile(headFile().string(), name + "\n");
    clearStage();
}

void Repository::checkoutFile(const std::string& file) {
    const Commit current = headCommit();
    auto it = current.files.find(file);
    if (it == current.files.end()) Utils::exitWithMessage("File does not exist in that commit.");
    writeWorkingFile(file, it->second);
}

void Repository::checkoutFileInCommit(const std::string& revision, const std::string& file) {
    const std::string id = resolveRevision(revision);
    if (id.empty()) Utils::exitWithMessage("No commit with that id exists.");
    const Commit c = loadCommit(id);
    auto it = c.files.find(file);
    if (it == c.files.end()) Utils::exitWithMessage("File does not exist in that commit.");
    writeWorkingFile(file, it->second);
}

void Repository::reset(const std::string& revision) {
    const std::string id = resolveRevision(revision);
    if (id.empty()) Utils::exitWithMessage("No commit with that id exists.");
    const Commit current = headCommit();
    const Commit target = loadCommit(id);
    if (hasBlockingUntracked(current, target)) Utils::exitWithMessage(kUntrackedError);
    checkoutSnapshot(current, target);
    auto bs = branches();
    bs[currentBranch()] = id;
    saveBranches(bs);
    clearStage();
}
