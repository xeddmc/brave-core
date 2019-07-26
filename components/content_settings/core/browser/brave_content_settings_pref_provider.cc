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
#include "components/content_settings/core/common/content_settings_utils.h"

namespace content_settings {

namespace {

Rule invalid(ContentSettingsPattern::FromString("https://no-thanks.invalid"),
             ContentSettingsPattern::FromString("https://no-thanks.invalid"),
             base::Value(CONTENT_SETTING_BLOCK));

class BraveShieldsRuleIterator : public RuleIterator {
 public:
  BraveShieldsRuleIterator(
      const base::RepeatingCallback<std::unique_ptr<RuleIterator>(ContentSettingsType content_type,
      const ResourceIdentifier& resource_identifier,
      bool incognito)>& callback,
      ContentSettingsType content_type,
      const ResourceIdentifier& resource_identifier,
      bool incognito)
      : callback_(callback),
        content_type_(content_type),
        resource_identifier_(resource_identifier),
        incognito_(incognito),
        has_next_(true),
        is_first_(true) {}

  Rule GetNext() {
    if (chromium_cookies_iterator_ && chromium_cookies_iterator_->HasNext()) {
      return chromium_cookies_iterator_->Next();
    }

    LOG(ERROR) << "brave cookies start";

    while (brave_cookies_iterator_ && brave_cookies_iterator_->HasNext()) {
      LOG(ERROR) << "cookies has next";
      // TODO(bridiver) - handle 1st vs 3rd party cookies
      Rule rule = brave_cookies_iterator_->Next();
      if (IsActive(rule)) {
        LOG(ERROR) << "rule is active";
        return rule;
      }
    }

    LOG(ERROR) << "brave cookies end";

    return Rule(invalid.primary_pattern,
                invalid.secondary_pattern,
                invalid.value.Clone());
  }

  bool HasNext() const override {
    return has_next_;
  }

  bool IsActive(const Rule& cookie_rule) const {
    for (const auto& shields_rule : shield_rules_) {
      auto primary_compare =
          shields_rule.primary_pattern.Compare(cookie_rule.primary_pattern);
      auto secondary_compare =
          shields_rule.secondary_pattern.Compare(cookie_rule.secondary_pattern);
      // TODO(bridiver) - verify that SUCCESSOR is correct and not PREDECESSOR
      if ((primary_compare == ContentSettingsPattern::IDENTITY ||
           primary_compare == ContentSettingsPattern::SUCCESSOR) &&
          (secondary_compare == ContentSettingsPattern::IDENTITY ||
           secondary_compare == ContentSettingsPattern::SUCCESSOR)) {
        return ValueToContentSetting(&shields_rule.value) !=
               CONTENT_SETTING_BLOCK;
      }
    }

    return true;
  }

  Rule Next() override {
    LOG(ERROR) << "Next " << this;
    if (is_first_) {
      chromium_cookies_iterator_ = callback_.Run(content_type_,
                                                          resource_identifier_,
                                                          incognito_);
      brave_cookies_iterator_ =
          callback_.Run(CONTENT_SETTINGS_TYPE_PLUGINS,
                                 brave_shields::kCookies,
                                 incognito_);

      auto shields_iterator = callback_.Run(CONTENT_SETTINGS_TYPE_PLUGINS,
                                          brave_shields::kBraveShields,
                                          incognito_);

      while (shields_iterator->HasNext())
        shield_rules_.push_back(shields_iterator->Next());

      is_first_ = false;
    }

    Rule rule = GetNext();

    if (rule.primary_pattern == invalid.primary_pattern)
      has_next_ = false;

    Rule to_return(rule.primary_pattern,
                   rule.secondary_pattern,
                   rule.value.Clone());

    return to_return;
  }

 private:
  base::RepeatingCallback<std::unique_ptr<RuleIterator>(ContentSettingsType content_type,
      const ResourceIdentifier& resource_identifier,
      bool incognito)> callback_;
  ContentSettingsType content_type_;
  const ResourceIdentifier resource_identifier_;
  bool incognito_;
  bool has_next_;
  bool is_first_;

  std::unique_ptr<RuleIterator> chromium_cookies_iterator_;
  std::unique_ptr<RuleIterator> brave_cookies_iterator_;
  std::vector<Rule> shield_rules_;

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

BravePrefProvider::~BravePrefProvider() {
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

std::unique_ptr<RuleIterator> BravePrefProvider::GetRuleIteratorInternal(
      ContentSettingsType content_type,
      const ResourceIdentifier& resource_identifier,
      bool incognito) const {
  return GetPref(content_type)->GetRuleIterator(resource_identifier, incognito);
}

std::unique_ptr<RuleIterator> BravePrefProvider::GetRuleIterator(
      ContentSettingsType content_type,
      const ResourceIdentifier& resource_identifier,
      bool incognito) const {
  if (content_type == CONTENT_SETTINGS_TYPE_COOKIES) {
    auto callback = base::BindRepeating(&BravePrefProvider::GetRuleIteratorInternal,
                                        // TODO(bridiver) - convert to weak ptr
                                        base::Unretained(this));

    // TODO(bridiver) - get the actual default setting for brave shields and cookies
    return std::make_unique<BraveShieldsRuleIterator>(callback,
                                                      content_type,
                                                      resource_identifier,
                                                      incognito);
  }

  return PrefProvider::GetRuleIterator(content_type,
                                       resource_identifier,
                                       incognito);
}

}  // namespace content_settings
