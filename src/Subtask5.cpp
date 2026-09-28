#include "../include/Repository.h"
#include "../include/Utils.h"

#include <queue>
#include <unordered_set>

namespace fs = std::filesystem;

namespace {
const char* kUntrackedError = "There is an untracked file in the way; delete it, or add and commit it first.";

void ensureEndsWithNewline(std::string& s) {
    if (!s.empty() && s.back() != '\n') s.push_back('\n');
}

bool sameOptional(const std::optional<std::string>& a, const std::optional<std::string>& b) {
    return a == b;
}

std::optional<std::string> lookupBlob(const Commit& c, const std::string& name) {
    auto it = c.files.find(name);
    if (it == c.files.end()) return std::nullopt;
    return it->second;
}
}

std::string Repository::findSplitPoint(const std::string& currentId, const std::string& givenId) const {
    std::unordered_set<std::string> currentAncestors;
    std::vector<std::string> stack{currentId};
    while (!stack.empty()) {
        std::string id = stack.back();
        stack.pop_back();
        if (!currentAncestors.insert(id).second) continue;
        Commit c = loadCommit(id);
        for (const auto& parent : c.parents) stack.push_back(parent);
    }

    std::queue<std::string> q;
    std::unordered_set<std::string> seen;
    q.push(givenId);
    seen.insert(givenId);
    while (!q.empty()) {
        std::string id = q.front();
        q.pop();
        if (currentAncestors.count(id)) return id;
        Commit c = loadCommit(id);
        for (const auto& parent : c.parents) {
            if (seen.insert(parent).second) q.push(parent);
        }
    }
    return std::string();
}

bool Repository::isAncestor(const std::string& ancestor, const std::string& descendant) const {
    if (ancestor == descendant) return true;
    std::vector<std::string> stack{descendant};
    std::unordered_set<std::string> seen;
    while (!stack.empty()) {
        std::string id = stack.back();
        stack.pop_back();
        if (!seen.insert(id).second) continue;
        if (id == ancestor) return true;
        fs::path p = commitsDir() / id;
        if (!fs::is_regular_file(p)) continue;
        Commit c = loadCommit(id);
        for (const auto& parent : c.parents) stack.push_back(parent);
    }
    return false;
}

void Repository::merge(const std::string& branchName) {
    if (!stageAdd().empty() || !stageRm().empty()) {
        Utils::exitWithMessage("You have uncommitted changes.");
    }
    auto bs = branches();
    auto givenIt = bs.find(branchName);
    if (givenIt == bs.end()) Utils::exitWithMessage("A branch with that name does not exist.");
    if (branchName == currentBranch()) Utils::exitWithMessage("Cannot merge a branch with itself.");

    const std::string currentId = headId();
    const std::string givenId = givenIt->second;
    const std::string splitId = findSplitPoint(currentId, givenId);
    const Commit current = loadCommit(currentId);
    const Commit given = loadCommit(givenId);
    const Commit split = loadCommit(splitId);

    if (splitId == givenId) {
        Utils::message("Given branch is an ancestor of the current branch.");
        return;
    }
    if (splitId == currentId) {
        if (hasBlockingUntracked(current, given)) Utils::exitWithMessage(kUntrackedError);
        checkoutSnapshot(current, given);
        bs[currentBranch()] = givenId;
        saveBranches(bs);
        clearStage();
        Utils::message("Current branch fast-forwarded.");
        return;
    }

    enum class ActionKind { None, TakeGiven, Remove, Conflict };
    struct Action { ActionKind kind = ActionKind::None; std::optional<std::string> givenBlob; };
    std::map<std::string, Action> actions;
    std::set<std::string> names;
    for (const auto& [n, _] : split.files) names.insert(n);
    for (const auto& [n, _] : current.files) names.insert(n);
    for (const auto& [n, _] : given.files) names.insert(n);

    std::set<std::string> affected;
    for (const auto& name : names) {
        const auto s = lookupBlob(split, name);
        const auto c = lookupBlob(current, name);
        const auto g = lookupBlob(given, name);
        if (sameOptional(c, g)) continue;
        if (sameOptional(s, c)) {
            if (g) actions[name] = {ActionKind::TakeGiven, g};
            else actions[name] = {ActionKind::Remove, std::nullopt};
            affected.insert(name);
        } else if (sameOptional(s, g)) {
            continue;
        } else {
            actions[name] = {ActionKind::Conflict, g};
            affected.insert(name);
        }
    }

    for (const auto& name : affected) {
        if (!fs::is_regular_file(root_ / name)) continue;
        if (current.files.count(name)) continue;
        if (isIgnored(name)) continue;
        Utils::exitWithMessage(kUntrackedError);
    }

    auto adds = stageAdd();
    auto rms = stageRm();
    bool conflict = false;
    for (const auto& [name, action] : actions) {
        if (action.kind == ActionKind::TakeGiven) {
            writeWorkingFile(name, *action.givenBlob);
            adds[name] = *action.givenBlob;
            rms.erase(name);
        } else if (action.kind == ActionKind::Remove) {
            std::error_code ec;
            fs::remove(root_ / name, ec);
            adds.erase(name);
            rms.insert(name);
        } else if (action.kind == ActionKind::Conflict) {
            conflict = true;
            std::string currentContent;
            std::string givenContent;
            if (auto c = lookupBlob(current, name)) currentContent = loadBlob(*c);
            if (auto g = lookupBlob(given, name)) givenContent = loadBlob(*g);
            ensureEndsWithNewline(currentContent);
            ensureEndsWithNewline(givenContent);
            std::string merged = "<<<<<<< HEAD\n" + currentContent
                               + "=======\n" + givenContent
                               + ">>>>>>>\n";
            Utils::writeFile((root_ / name).string(), merged);
            const std::string blob = storeBlob(merged);
            adds[name] = blob;
            rms.erase(name);
        }
    }
    saveStage(adds, rms);

    const std::string message = "Merged " + branchName + " into " + currentBranch() + ".";
    createCommit(message, givenId, true);
    if (conflict) Utils::message("Encountered a merge conflict.");
}
