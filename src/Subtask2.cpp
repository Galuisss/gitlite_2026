#include "../include/Repository.h"
#include "../include/Utils.h"

#include <iostream>

void Repository::log() const {
    std::string id = headId();
    while (!id.empty()) {
        Commit c = loadCommit(id);
        printCommit(id, c);
        if (c.parents.empty()) break;
        id = c.parents[0];
    }
}

void Repository::globalLog() const {
    for (const auto& name : Utils::plainFilenamesIn(commitsDir().string())) {
        printCommit(name, loadCommit(name));
    }
}


void Repository::listTags() const {
    for (const auto& [name, id] : tags()) std::cout << name << ' ' << id << '\n';
}

void Repository::tag(const std::string& name) {
    auto ts = tags();
    if (ts.count(name)) Utils::exitWithMessage("A tag with that name already exists.");
    ts[name] = headId();
    saveTags(ts);
}

void Repository::tag(const std::string& name, const std::string& revision) {
    auto ts = tags();
    if (ts.count(name)) Utils::exitWithMessage("A tag with that name already exists.");
    const std::string id = resolveRevision(revision);
    if (id.empty()) Utils::exitWithMessage("No commit with that id exists.");
    ts[name] = id;
    saveTags(ts);
}
