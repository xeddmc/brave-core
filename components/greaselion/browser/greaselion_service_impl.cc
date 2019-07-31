/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/greaselion/browser/greaselion_service_impl.h"

#include <memory>

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/json/json_writer.h"  // TODO remove this, for debugging only
#include "base/sequenced_task_runner.h"
#include "base/task/post_task.h"
#include "base/task_runner_util.h"
#include "brave/components/greaselion/browser/greaselion_download_service.h"
#include "brave/components/greaselion/browser/greaselion_extension_converter.h"
#include "extensions/common/extension.h"
#include "extensions/common/file_util.h"

namespace greaselion {

GreaselionServiceImpl::GreaselionServiceImpl(
    GreaselionDownloadService* download_service,
    const base::FilePath& install_directory,
    scoped_refptr<base::SequencedTaskRunner> task_runner)
    : download_service_(download_service),
      install_directory_(install_directory),
      all_rules_installed_successfully_(true),
      pending_installs_(0),
      task_runner_(std::move(task_runner)),
      weak_factory_(this) {
  for (int i = FIRST_FEATURE; i != LAST_FEATURE; i++)
    state_[static_cast<GreaselionFeature>(i)] = false;
}

GreaselionServiceImpl::~GreaselionServiceImpl() = default;

void GreaselionServiceImpl::UpdateInstalledExtensions() {
  std::vector<std::unique_ptr<GreaselionRule>>* rules =
      download_service_->rules();
  LOG(INFO) << "entering UpdateInstalledExtensions";
  all_rules_installed_successfully_ = true;
  pending_installs_ = 0;
  for (const std::unique_ptr<GreaselionRule>& rule : *rules) {
    if (rule->Matches(state_)) {
      pending_installs_ += 1;
    }
  }
  if (!pending_installs_) {
    MaybeNotifyObservers();
    // no rules match, nothing else to do
    LOG(INFO) << "No Greaselion rules, exiting";
    return;
  }
  LOG(INFO) << "found " << pending_installs_ << " matching rules";
  for (const std::unique_ptr<GreaselionRule>& rule : *rules) {
    if (rule->Matches(state_)) {
      // Convert script file to component extension. This must run on extension
      // file task runner, which was passed in.
      base::PostTaskAndReplyWithResult(
          task_runner_.get(), FROM_HERE,
          base::BindOnce(
              &greaselion::ConvertGreaselionRuleToExtensionOnTaskRunner,
              rule.get(), install_directory_),
          base::BindOnce(&GreaselionServiceImpl::Install,
                         weak_factory_.GetWeakPtr()));
    }
  }
}

void GreaselionServiceImpl::Install(
    scoped_refptr<extensions::Extension> extension) {
  if (!extension.get()) {
    all_rules_installed_successfully_ = false;
    pending_installs_ -= 1;
    MaybeNotifyObservers();
    LOG(ERROR) << "Could not load Greaselion script";
  } else {
    LOG(INFO) << "Extension ID: " << extension->id();
    std::string json;
    base::JSONWriter::Write(*extension->manifest()->value(), &json);
    LOG(INFO) << "Extension manifest: " << json;

    installed_[extension->id()] = extension->path();
    base::PostTaskAndReplyWithResult(
        task_runner_.get(), FROM_HERE,
        base::BindOnce(&extensions::file_util::InstallExtension,
                       extension->path(), extension->id(),
                       extension->VersionString(), install_directory_),
        base::BindOnce(&GreaselionServiceImpl::PostInstall,
                       weak_factory_.GetWeakPtr()));
  }
}

void GreaselionServiceImpl::PostInstall(const base::FilePath& extension_path) {
  LOG(INFO) << "Greaselion dynamic extension successfully installed into "
            << extension_path;
  pending_installs_ -= 1;
  MaybeNotifyObservers();
}

void GreaselionServiceImpl::MaybeNotifyObservers() {
  if (!pending_installs_)
    for (Observer& observer : observers_)
      observer.OnExtensionsReady(this, all_rules_installed_successfully_);
}

void GreaselionServiceImpl::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void GreaselionServiceImpl::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

#if 0
bool GreaselionServiceImpl::ScriptsFor(const GURL& primary_url,
                                       std::vector<base::FilePath>* scripts) {
  bool any = false;
  std::vector<std::unique_ptr<GreaselionRule>>* rules =
      download_service_->rules();
  scripts->clear();
  for (const std::unique_ptr<GreaselionRule>& rule : *rules) {
    if (rule->Matches(primary_url, state_)) {
      rule->Populate(scripts);
      any = true;
    }
  }
  return any;
}
#endif

void GreaselionServiceImpl::SetFeatureEnabled(GreaselionFeature feature,
                                              bool enabled) {
  DCHECK(feature >= 0 && feature < LAST_FEATURE);
  state_[feature] = enabled;
  UpdateInstalledExtensions();
}

}  // namespace greaselion
