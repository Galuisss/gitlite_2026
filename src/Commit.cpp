#include "../include/Commit.h"

#include <stdexcept>

namespace {
void appendField(std::string& out, const std::string& value) {
    out += std::to_string(value.size());
    out.push_back('\n');
    out += value;
    out.push_back('\n');
}

std::string readLine(const std::string& data, std::size_t& pos) {
    const std::size_t end = data.find('\n', pos);
    if (end == std::string::npos) throw std::runtime_error("corrupt commit");
    std::string line = data.substr(pos, end - pos);
    pos = end + 1;
    return line;
}

std::string readField(const std::string& data, std::size_t& pos) {
    std::size_t len = static_cast<std::size_t>(std::stoull(readLine(data, pos)));
    if (pos + len > data.size()) throw std::runtime_error("corrupt commit");
    std::string value = data.substr(pos, len);
    pos += len;
    if (pos >= data.size() || data[pos] != '\n') throw std::runtime_error("corrupt commit");
    ++pos;
    return value;
}
} // namespace

std::string Commit::serialize() const {
    std::string out = "gitlite-commit-v1\n";
    appendField(out, message);
    out += std::to_string(timestamp) + "\n";
    out += std::to_string(parents.size()) + "\n";
    for (const auto& parent : parents) appendField(out, parent);
    out += std::to_string(files.size()) + "\n";
    for (const auto& [name, blob] : files) {
        appendField(out, name);
        appendField(out, blob);
    }
    return out;
}

Commit Commit::deserialize(const std::string& data) {
    std::size_t pos = 0;
    if (readLine(data, pos) != "gitlite-commit-v1") throw std::runtime_error("corrupt commit");
    Commit c;
    c.message = readField(data, pos);
    c.timestamp = std::stoll(readLine(data, pos));
    const std::size_t parentCount = static_cast<std::size_t>(std::stoull(readLine(data, pos)));
    for (std::size_t i = 0; i < parentCount; ++i) c.parents.push_back(readField(data, pos));
    const std::size_t fileCount = static_cast<std::size_t>(std::stoull(readLine(data, pos)));
    for (std::size_t i = 0; i < fileCount; ++i) {
        std::string name = readField(data, pos);
        std::string blob = readField(data, pos);
        c.files[name] = blob;
    }
    return c;
}
