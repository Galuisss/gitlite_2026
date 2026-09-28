#ifndef COMMIT_H
#define COMMIT_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct Commit {
    std::string message;
    std::int64_t timestamp = 0;
    std::vector<std::string> parents;
    std::map<std::string, std::string> files;

    std::string serialize() const;
    static Commit deserialize(const std::string& data);
};

#endif
