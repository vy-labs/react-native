#include "JSDeltaBundleClient.h"

#include <sstream>

#include <folly/Memory.h>

namespace facebook {
namespace react {

namespace {
  std::string startupCode(const folly::dynamic *pre, const folly::dynamic *post) {
    std::ostringstream startupCode;

    for (auto section : {pre, post}) {
      if (section != nullptr) {
        startupCode << section->getString() << '\n';
      }
    }

    return startupCode.str();
  }
} // namespace

void JSDeltaBundleClient::patch(const folly::dynamic& delta) {
  auto const base = delta.get_ptr("base");

  if (base != nullptr && base->asBool()) {
    clear();

    auto const pre = delta.get_ptr("pre");
    auto const post = delta.get_ptr("post");

    startupCode_ = startupCode(pre, post);
  } else {
    const folly::dynamic *deleted = delta.get_ptr("deleted");
    if (deleted != nullptr) {
      for (const folly::dynamic id : *deleted) {
        modules_.erase(id.getInt());
      }
    }
  }

  const folly::dynamic *modules = delta.get_ptr("modules");
  if (modules != nullptr) {
    for (const folly::dynamic pair : *modules) {
      auto id = pair[0].getInt();
      auto module = pair[1];
      modules_.emplace(id, module.getString());
    }
  }
}

JSModulesUnbundle::Module JSDeltaBundleClient::getModule(uint32_t moduleId) const {
  auto search = modules_.find(moduleId);
  if (search != modules_.end()) {
    return {folly::to<std::string>(search->first, ".js"), search->second};
  }

  throw JSModulesUnbundle::ModuleNotFound(moduleId);
}

std::unique_ptr<const JSBigString> JSDeltaBundleClient::getStartupCode() const {
  return folly::make_unique<JSBigStdString>(startupCode_);
}

void JSDeltaBundleClient::clear() {
  modules_.clear();
  startupCode_.clear();
}

} // namespace react
} // namespace facebook
