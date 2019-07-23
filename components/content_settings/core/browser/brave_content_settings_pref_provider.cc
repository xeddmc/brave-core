/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/content_settings/core/browser/brave_content_settings_pref_provider.h"

#include <memory>
#include <utility>

#include "base/bind.h"
#include "brave/components/brave_shields/common/brave_shield_constants.h"
#include "components/content_settings/core/browser/content_settings_pref.h"
#include "components/content_settings/core/browser/website_settings_registry.h"

namespace content_settings {

namespace {

class BraveShieldsRuleIterator : public RuleIterator {
 public:
  BraveShieldsRuleIterator(RuleIterator* brave_shields_iterator,
                           RuleIterator* brave_cookies_iterator,
                           RuleIterator* chromium_cookies_iterator)
      : brave_shields_iterator_(brave_shields_iterator),
        brave_cookies_iterator_(brave_cookies_iterator),
        chromium_cookies_iterator_(chromium_cookies_iterator) {}

  bool HasNext() const override {
    return chromium_cookies_iterator_->HasNext() ||
           brave_cookies_iterator_->HasNext();
  }

  Rule Next() override {
    if (chromium_cookies_iterator_->HasNext())
      return chromium_cookies_iterator_->Next();

    // TODO(bridiver) - check through brave_shields and ignore cookie setting
    // if brave_shields is disabled
    return brave_cookies_iterator_->Next();
  }

 private:
  RuleIterator* brave_shields_iterator_;
  RuleIterator* brave_cookies_iterator_;
  RuleIterator* chromium_cookies_iterator_;

  DISALLOW_COPY_AND_ASSIGN(BraveShieldsRuleIterator);
};

}

BravePrefProvider::BravePrefProvider(PrefService* prefs,
                                     bool incognito,
                                     bool store_last_modified)
    : PrefProvider(prefs, incognito, store_last_modified) {
  brave_pref_change_registrar_.Init(prefs_);

  WebsiteSettingsRegistry* website_settings =
      WebsiteSettingsRegistry::GetInstance();
  // Makes BravePrefProvder handle plugin type.
  for (const WebsiteSettingsInfo* info : *website_settings) {
    if (info->type() == CONTENT_SETTINGS_TYPE_PLUGINS) {
      content_settings_prefs_.insert(std::make_pair(
          info->type(),
          std::make_unique<ContentSettingsPref>(
              info->type(), prefs_, &brave_pref_change_registrar_,
              info->pref_name(),
              is_incognito_,
              base::Bind(&PrefProvider::Notify, base::Unretained(this)))));
      return;
    }
  }
}

void BravePrefProvider::ShutdownOnUIThread() {
  brave_pref_change_registrar_.RemoveAll();
  PrefProvider::ShutdownOnUIThread();
}

bool BravePrefProvider::SetWebsiteSetting(
    const ContentSettingsPattern& primary_pattern,
    const ContentSettingsPattern& secondary_pattern,
    ContentSettingsType content_type,
    const ResourceIdentifier& resource_identifier,
    std::unique_ptr<base::Value>&& in_value) {
  // Flash's setting shouldn't be reached here.
  // Its content type is plugin and id is empty string.
  // One excpetion is default setting. It can be persisted.
  if (content_type == CONTENT_SETTINGS_TYPE_PLUGINS &&
      resource_identifier.empty()) {
    DCHECK(primary_pattern == ContentSettingsPattern::Wildcard() &&
           secondary_pattern == ContentSettingsPattern::Wildcard());
  }

  return PrefProvider::SetWebsiteSetting(primary_pattern, secondary_pattern,
                                         content_type, resource_identifier,
                                         std::move(in_value));
}

std::unique_ptr<RuleIterator> BravePrefProvider::GetRuleIterator(
      ContentSettingsType content_type,
      const ResourceIdentifier& resource_identifier,
      bool incognito) const {
  if (content_type == CONTENT_SETTINGS_TYPE_COOKIES) {
    return std::make_unique<BraveShieldsRuleIterator>(
        GetRuleIterator(CONTENT_SETTINGS_TYPE_PLUGINS,
                        brave_shields::kBraveShields,
                        incognito),
        GetRuleIterator(CONTENT_SETTINGS_TYPE_PLUGINS,
                        brave_shields::kCookies,
                        incognito),
        PrefProvider::GetRuleIterator(content_type,
                                      resource_identifier,
                                      incognito));
  }
}

}  // namespace content_settings
