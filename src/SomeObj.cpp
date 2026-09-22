#include "../include/SomeObj.h"

SomeObj::SomeObj() {}

// ---------------- Subtask 1：init / add / commit / rm ----------------

void SomeObj::init() {
    // TODO: 创建 .gitlite 目录、initial commit 与 master 分支
}

void SomeObj::add(const std::string&) {
    // TODO: 将文件的当前内容加入暂存区
}

void SomeObj::commit(const std::string&) {
    // TODO: 将暂存区保存为新提交，并推进当前分支
}

void SomeObj::rm(const std::string&) {
    // TODO: 将文件取消暂存，或标记为待删除
}

// ---------------- Subtask 2：log / global-log / find / checkout（文件） ----------------

void SomeObj::log() {
    // TODO: 从当前提交沿第一个父提交打印日志
}

void SomeObj::globalLog() {
    // TODO: 打印所有提交，顺序不限
}

void SomeObj::find(const std::string&) {
    // TODO: 打印提交信息与给定信息完全相同的提交 id
}

void SomeObj::checkoutFile(const std::string&) {
    // TODO: 用当前提交中的版本覆盖工作目录中的文件
}

void SomeObj::checkoutFileInCommit(const std::string&, const std::string&) {
    // TODO: 用指定提交中的版本覆盖工作目录中的文件，支持 id 缩写
}

// ---------------- Subtask 3：status / checkout（分支） ----------------

void SomeObj::status() {
    // TODO: 显示分支、暂存区；Bonus 再补全未暂存修改与未跟踪文件
}

void SomeObj::checkoutBranch(const std::string&) {
    // TODO: 切换到指定分支
}

// ---------------- Subtask 4：branch / rm-branch / reset ----------------

void SomeObj::branch(const std::string&) {
    // TODO: 新建指向当前提交的分支，不切换
}

void SomeObj::rmBranch(const std::string&) {
    // TODO: 删除分支指针，不删除其提交
}

void SomeObj::reset(const std::string&) {
    // TODO: 检出指定提交，并将当前分支头移动到该提交
}

// ---------------- Subtask 5：merge ----------------

void SomeObj::merge(const std::string&) {
    // TODO: 将指定分支合并到当前分支
}

// ---------------- Subtask 6：add-remote / rm-remote / push / fetch / pull ----------------

void SomeObj::addRemote(const std::string&, const std::string&) {
    // TODO: 保存远程仓库地址
}

void SomeObj::rmRemote(const std::string&) {
    // TODO: 删除远程仓库信息
}

void SomeObj::push(const std::string&, const std::string&) {
    // TODO: 将当前分支的提交推送到远程分支
}

void SomeObj::fetch(const std::string&, const std::string&) {
    // TODO: 将远程分支的提交与 blob 复制到本地
}

void SomeObj::pull(const std::string&, const std::string&) {
    // TODO: fetch 后将取回的分支合并到当前分支
}

// ---------------- Subtask 6：status Bonus / diff ----------------

void SomeObj::diff() {
    // TODO: 比较当前提交与工作目录并输出差异，无差异时不输出
}

void SomeObj::diffWithCommit(const std::string&) {
    // TODO: 比较指定提交与工作目录并输出差异，无差异时不输出
}

void SomeObj::diffBetween(const std::string&, const std::string&) {
    // TODO: 比较两个指定提交并输出差异，无差异时不输出
}
