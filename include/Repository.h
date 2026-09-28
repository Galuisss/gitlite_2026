#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "Commit.h"

#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

class Repository {
public:
    Repository();
    static std::string getGitliteDir();

    void init();
    void add(const std::string& file);
    void commit(const std::string& message);
    void rm(const std::string& file);

    void log() const;
    void globalLog() const;
    void listTags() const;
    void tag(const std::string& name);
    void tag(const std::string& name, const std::string& revision);

    void status() const;
    void branch(const std::string& name);
    void rmBranch(const std::string& name);

    void checkoutBranch(const std::string& name);
    void checkoutFile(const std::string& file);
    void checkoutFileInCommit(const std::string& revision, const std::string& file);
    void reset(const std::string& revision);

    void merge(const std::string& branchName);

    void addRemote(const std::string& name, const std::string& path);
    void rmRemote(const std::string& name);
    void push(const std::string& remote, const std::string& branchName);
    void fetch(const std::string& remote, const std::string& branchName);
    void pull(const std::string& remote, const std::string& branchName);

    void diff() const;
    void diffWithCommit(const std::string& revision) const;
    void diffBetween(const std::string& rev1, const std::string& rev2) const;
    void show() const;
    void show(const std::string& revision) const;

private:
    std::filesystem::path root_;
    std::filesystem::path gitDir_;

    std::filesystem::path commitsDir() const;
    std::filesystem::path blobsDir() const;
    std::filesystem::path branchesFile() const;
    std::filesystem::path tagsFile() const;
    std::filesystem::path remotesFile() const;
    std::filesystem::path headFile() const;
    std::filesystem::path stageAddFile() const;
    std::filesystem::path stageRmFile() const;

    std::map<std::string, std::string> branches() const;
    std::map<std::string, std::string> tags() const;
    std::map<std::string, std::string> remotes() const;
    std::map<std::string, std::string> stageAdd() const;
    std::set<std::string> stageRm() const;

    void saveBranches(const std::map<std::string, std::string>& values) const;
    void saveTags(const std::map<std::string, std::string>& values) const;
    void saveRemotes(const std::map<std::string, std::string>& values) const;
    void saveStage(const std::map<std::string, std::string>& adds,
                   const std::set<std::string>& rms) const;
    void clearStage() const;

    std::string currentBranch() const;
    std::string headId() const;
    Commit headCommit() const;
    Commit loadCommit(const std::string& id) const;
    std::string storeCommit(const Commit& commit) const;
    std::string storeBlob(const std::string& data) const;
    std::string loadBlob(const std::string& id) const;

    std::string resolveRevision(const std::string& revision) const;
    void printCommit(const std::string& id, const Commit& commit) const;
    static std::string formatTimestamp(std::int64_t timestamp);

    bool isIgnored(const std::string& filename) const;
    static bool globMatch(const std::string& pattern, const std::string& text);

    bool hasBlockingUntracked(const Commit& current, const Commit& target,
                              const std::set<std::string>* onlyPaths = nullptr) const;
    void checkoutSnapshot(const Commit& current, const Commit& target) const;
    void writeWorkingFile(const std::string& file, const std::string& blobId) const;

    std::string createCommit(const std::string& message,
                             const std::optional<std::string>& secondParent,
                             bool allowEmpty);
    std::string findSplitPoint(const std::string& currentId,
                               const std::string& givenId) const;
    bool isAncestor(const std::string& ancestor, const std::string& descendant) const;

    static std::map<std::string, std::string> readMapAt(const std::filesystem::path& path);
    static void writeMapAt(const std::filesystem::path& path,
                           const std::map<std::string, std::string>& values);
    static void copyObjectDirectory(const std::filesystem::path& src,
                                    const std::filesystem::path& dst);
    void copyAllObjects(const std::filesystem::path& srcGit,
                        const std::filesystem::path& dstGit) const;

    struct DiffLine {
        std::string text;
        bool newline = false;
    };
    struct DiffOp {
        char type = ' ';
        DiffLine line;
        int oldNo = 0;
        int newNo = 0;
    };

    static std::vector<DiffLine> splitLines(const std::string& content);
    static std::vector<DiffOp> buildDiffOps(const std::vector<DiffLine>& oldLines,
                                            const std::vector<DiffLine>& newLines);
    static void printFileDiff(const std::string& file,
                              bool oldPresent, const std::string& oldContent,
                              bool newPresent, const std::string& newContent);
    void diffCommitToWorking(const Commit& oldCommit) const;
    void diffCommits(const Commit& oldCommit, const Commit& newCommit) const;
};

#endif
