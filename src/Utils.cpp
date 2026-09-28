#include "../include/Utils.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

namespace {
std::uint32_t rol(std::uint32_t value, int bits) {
    return (value << bits) | (value >> (32 - bits));
}
}

std::string Utils::sha1(const std::string& data) {
    std::vector<std::uint8_t> msg(data.begin(), data.end());
    const std::uint64_t bitLen = static_cast<std::uint64_t>(msg.size()) * 8ULL;
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) msg.push_back(0);
    for (int i = 7; i >= 0; --i) msg.push_back(static_cast<std::uint8_t>((bitLen >> (i * 8)) & 0xff));

    std::uint32_t h0 = 0x67452301;
    std::uint32_t h1 = 0xEFCDAB89;
    std::uint32_t h2 = 0x98BADCFE;
    std::uint32_t h3 = 0x10325476;
    std::uint32_t h4 = 0xC3D2E1F0;

    for (std::size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        std::array<std::uint32_t, 80> w{};
        for (int i = 0; i < 16; ++i) {
            const std::size_t j = chunk + static_cast<std::size_t>(i) * 4;
            w[i] = (static_cast<std::uint32_t>(msg[j]) << 24)
                 | (static_cast<std::uint32_t>(msg[j + 1]) << 16)
                 | (static_cast<std::uint32_t>(msg[j + 2]) << 8)
                 | static_cast<std::uint32_t>(msg[j + 3]);
        }
        for (int i = 16; i < 80; ++i) w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

        std::uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
        for (int i = 0; i < 80; ++i) {
            std::uint32_t f = 0, k = 0;
            if (i < 20) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            std::uint32_t temp = rol(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = rol(b, 30);
            b = a;
            a = temp;
        }
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }

    std::ostringstream out;
    out << std::hex << std::setfill('0')
        << std::setw(8) << h0 << std::setw(8) << h1 << std::setw(8) << h2
        << std::setw(8) << h3 << std::setw(8) << h4;
    return out.str();
}

std::string Utils::readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void Utils::writeFile(const std::string& path, const std::string& data) {
    fs::path p(path);
    if (p.has_parent_path()) fs::create_directories(p.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(data.data(), static_cast<std::streamsize>(data.size()));
}

bool Utils::exists(const std::string& path) { return fs::exists(fs::path(path)); }
bool Utils::isFile(const std::string& path) { return fs::is_regular_file(fs::path(path)); }
bool Utils::isDirectory(const std::string& path) { return fs::is_directory(fs::path(path)); }
bool Utils::createDirectories(const std::string& path) { return fs::create_directories(fs::path(path)); }

bool Utils::restrictedDelete(const std::string& path) {
    std::error_code ec;
    return fs::remove(fs::path(path), ec);
}

std::vector<std::string> Utils::plainFilenamesIn(const std::string& dirPath) {
    std::vector<std::string> result;
    std::error_code ec;
    if (!fs::is_directory(fs::path(dirPath), ec)) return result;
    for (const auto& entry : fs::directory_iterator(fs::path(dirPath))) {
        if (entry.is_regular_file()) result.push_back(entry.path().filename().string());
    }
    std::sort(result.begin(), result.end());
    return result;
}

void Utils::message(const std::string& msg) { std::cout << msg << '\n'; }
[[noreturn]] void Utils::exitWithMessage(const std::string& msg) {
    std::cout << msg << '\n';
    std::exit(0);
}

std::map<std::string, std::string> Utils::readMap(const std::string& path) {
    std::map<std::string, std::string> result;
    if (!isFile(path)) return result;
    std::ifstream in(path, std::ios::binary);
    std::size_t count = 0;
    if (!(in >> count)) return result;
    for (std::size_t i = 0; i < count; ++i) {
        std::string k, v;
        in >> std::quoted(k) >> std::quoted(v);
        result[k] = v;
    }
    return result;
}

void Utils::writeMap(const std::string& path, const std::map<std::string, std::string>& values) {
    fs::path p(path);
    if (p.has_parent_path()) fs::create_directories(p.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << values.size() << '\n';
    for (const auto& [k, v] : values) out << std::quoted(k) << ' ' << std::quoted(v) << '\n';
}

std::set<std::string> Utils::readSet(const std::string& path) {
    std::set<std::string> result;
    if (!isFile(path)) return result;
    std::ifstream in(path, std::ios::binary);
    std::size_t count = 0;
    if (!(in >> count)) return result;
    for (std::size_t i = 0; i < count; ++i) {
        std::string v;
        in >> std::quoted(v);
        result.insert(v);
    }
    return result;
}

void Utils::writeSet(const std::string& path, const std::set<std::string>& values) {
    fs::path p(path);
    if (p.has_parent_path()) fs::create_directories(p.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << values.size() << '\n';
    for (const auto& v : values) out << std::quoted(v) << '\n';
}
