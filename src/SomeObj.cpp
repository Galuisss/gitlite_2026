#include "../include/SomeObj.h"
#include "../include/Repository.h"

SomeObj::SomeObj() = default;

void SomeObj::init() { Repository().init(); }
void SomeObj::add(const std::string& f) { Repository().add(f); }
void SomeObj::commit(const std::string& m) { Repository().commit(m); }
void SomeObj::rm(const std::string& f) { Repository().rm(f); }
void SomeObj::log() {}
void SomeObj::globalLog() {}
void SomeObj::listTags() {}
void SomeObj::tag(const std::string&) {}
void SomeObj::tag(const std::string&, const std::string&) {}
void SomeObj::status() {}
void SomeObj::checkoutBranch(const std::string&) {}
void SomeObj::checkoutFile(const std::string&) {}
void SomeObj::checkoutFileInCommit(const std::string&, const std::string&) {}
void SomeObj::branch(const std::string&) {}
void SomeObj::rmBranch(const std::string&) {}
void SomeObj::reset(const std::string&) {}
void SomeObj::merge(const std::string&) {}
void SomeObj::addRemote(const std::string&, const std::string&) {}
void SomeObj::rmRemote(const std::string&) {}
void SomeObj::push(const std::string&, const std::string&) {}
void SomeObj::fetch(const std::string&, const std::string&) {}
void SomeObj::pull(const std::string&, const std::string&) {}
void SomeObj::diff() {}
void SomeObj::diffWithCommit(const std::string&) {}
void SomeObj::diffBetween(const std::string&, const std::string&) {}
void SomeObj::show() {}
void SomeObj::show(const std::string&) {}
