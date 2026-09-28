#include "../include/SomeObj.h"
#include "../include/Repository.h"

SomeObj::SomeObj() = default;

void SomeObj::init() { Repository().init(); }
void SomeObj::add(const std::string& f) { Repository().add(f); }
void SomeObj::commit(const std::string& m) { Repository().commit(m); }
void SomeObj::rm(const std::string& f) { Repository().rm(f); }
void SomeObj::log() { Repository().log(); }
void SomeObj::globalLog() { Repository().globalLog(); }
void SomeObj::listTags() { Repository().listTags(); }
void SomeObj::tag(const std::string& n) { Repository().tag(n); }
void SomeObj::tag(const std::string& n, const std::string& r) { Repository().tag(n, r); }
void SomeObj::status() { Repository().status(); }
void SomeObj::checkoutBranch(const std::string& b) { Repository().checkoutBranch(b); }
void SomeObj::checkoutFile(const std::string& f) { Repository().checkoutFile(f); }
void SomeObj::checkoutFileInCommit(const std::string& r, const std::string& f) { Repository().checkoutFileInCommit(r, f); }
void SomeObj::branch(const std::string& b) { Repository().branch(b); }
void SomeObj::rmBranch(const std::string& b) { Repository().rmBranch(b); }
void SomeObj::reset(const std::string& r) { Repository().reset(r); }
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
