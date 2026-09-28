#include "../include/Repository.h"
#include "../include/Utils.h"

namespace fs = std::filesystem;

std::map<std::string, std::string> Repository::readMapAt(const fs::path& path) {
    return Utils::readMap(path.string());
}

void Repository::writeMapAt(const fs::path& path, const std::map<std::string, std::string>& values) {
    Utils::writeMap(path.string(), values);
}

void Repository::copyObjectDirectory(const fs::path& src, const fs::path& dst) {
    fs::create_directories(dst);
    if (!fs::is_directory(src)) return;
    for (const auto& entry : fs::directory_iterator(src)) {
        if (!entry.is_regular_file()) continue;
        const fs::path target = dst / entry.path().filename();
        if (!fs::exists(target)) fs::copy_file(entry.path(), target);
    }
}

void Repository::copyAllObjects(const fs::path& srcGit, const fs::path& dstGit) const {
    copyObjectDirectory(srcGit / "objects" / "commits", dstGit / "objects" / "commits");
    copyObjectDirectory(srcGit / "objects" / "blobs", dstGit / "objects" / "blobs");
}

void Repository::addRemote(const std::string& name, const std::string& path) {
    auto rs = remotes();
    if (rs.count(name) || branches().count(name)) {
        Utils::exitWithMessage("A remote with that name already exists.");
    }
    rs[name] = path;
    saveRemotes(rs);
}

void Repository::rmRemote(const std::string& name) {
    auto rs = remotes();
    if (!rs.count(name)) Utils::exitWithMessage("A remote with that name does not exist.");
    rs.erase(name);
    saveRemotes(rs);
}

void Repository::push(const std::string& remote, const std::string& branchName) {
    const auto rs = remotes();
    auto it = rs.find(remote);
    if (it == rs.end()) Utils::exitWithMessage("Remote directory not found.");
    const fs::path remoteGit(it->second);
    if (!fs::is_directory(remoteGit)) Utils::exitWithMessage("Remote directory not found.");

    auto remoteBranches = readMapAt(remoteGit / "branches");
    auto rb = remoteBranches.find(branchName);
    if (rb != remoteBranches.end() && !isAncestor(rb->second, headId())) {
        Utils::exitWithMessage("Please pull down remote changes before pushing.");
    }
    copyAllObjects(gitDir_, remoteGit);
    remoteBranches[branchName] = headId();
    writeMapAt(remoteGit / "branches", remoteBranches);
}

void Repository::fetch(const std::string& remote, const std::string& branchName) {
    const auto rs = remotes();
    auto it = rs.find(remote);
    if (it == rs.end()) Utils::exitWithMessage("Remote directory not found.");
    const fs::path remoteGit(it->second);
    if (!fs::is_directory(remoteGit)) Utils::exitWithMessage("Remote directory not found.");

    const auto remoteBranches = readMapAt(remoteGit / "branches");
    auto rb = remoteBranches.find(branchName);
    if (rb == remoteBranches.end()) Utils::exitWithMessage("That remote does not have that branch.");
    copyAllObjects(remoteGit, gitDir_);
    auto bs = branches();
    bs[remote + "/" + branchName] = rb->second;
    saveBranches(bs);
}

void Repository::pull(const std::string& remote, const std::string& branchName) {
    fetch(remote, branchName);
    merge(remote + "/" + branchName);
}

