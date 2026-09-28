#ifndef UTILS_H
#define UTILS_H

#include <map>
#include <set>
#include <string>
#include <vector>

class Utils {
public:
    static std::string sha1(const std::string& data);
    static std::string readFile(const std::string& path);
    static void writeFile(const std::string& path, const std::string& data);
    static bool exists(const std::string& path);
    static bool isFile(const std::string& path);
    static bool isDirectory(const std::string& path);
    static bool createDirectories(const std::string& path);
    static bool restrictedDelete(const std::string& path);
    static std::vector<std::string> plainFilenamesIn(const std::string& dirPath);
    static void message(const std::string& msg);
    [[noreturn]] static void exitWithMessage(const std::string& msg);

    static std::map<std::string, std::string> readMap(const std::string& path);
    static void writeMap(const std::string& path,
                         const std::map<std::string, std::string>& values);
    static std::set<std::string> readSet(const std::string& path);
    static void writeSet(const std::string& path, const std::set<std::string>& values);
};

#endif
