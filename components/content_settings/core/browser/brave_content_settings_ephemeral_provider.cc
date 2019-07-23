/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/content_settings/core/browser/brave_content_settings_ephemeral_provider.h"

#include <memory>
#include <utility>

#include "brave/components/brave_shields/common/brave_shield_constants.h"

namespace {

bool IsShieldsResourceID(
    const content_settings::ResourceIdentifier& resource_identifier) {
  if (resource_identifier == brave_shields::kAds ||
      resource_identifier == brave_shields::kTrackers ||
      resource_identifier == brave_shields::kHTTPUpgradableResources ||
      resource_identifier == brave_shields::kJavaScript ||
      resource_identifier == brave_shields::kFingerprinting ||
      resource_identifier == brave_shields::kBraveShields ||
      resource_identifier == brave_shields::kReferrers ||
      resource_identifier == brave_shields::kCookies) {
    return true;
  } else {
    return false;
  }
}

class CookieRuleIterator : public RuleIterator {
 public:
  CookieRuleIterator(ContentSetting setting) : setting_(setting) {}

  bool HasNext() const override { return !is_done_; }

  Rule Next() override {
    DCHECK(HasNext());
    is_done_ = true;
    return Rule(ContentSettingsPattern::Wildcard(),
                ContentSettingsPattern::Wildcard(), base::Value(setting_));
  }

 private:
  const ContentSetting setting_;
  bool is_done_ = false;

  DISALLOW_COPY_AND_ASSIGN(RuleIteratorSimple);
};

}  // namespace

namespace content_settings {

std::unique_ptr<RuleIterator> BraveEphemeralProvider::GetRuleIterator(
      ContentSettingsType content_type,
      const ResourceIdentifier& resource_identifier,
      bool incognito) const {
  if (content_type == CONTENT_SETTINGS_TYPE_COOKIES) {

  }
}

bool BraveEphemeralProvider::SetWebsiteSetting(
    const ContentSettingsPattern& primary_pattern,
    const ContentSettingsPattern& secondary_pattern,
    ContentSettingsType content_type,
    const ResourceIdentifier& resource_identifier,
    std::unique_ptr<base::Value>&& in_value) {
  // Prevent this handle shields configuration.
  if (content_type == CONTENT_SETTINGS_TYPE_PLUGINS &&
      IsShieldsResourceID(resource_identifier)) {
    return false;
  }

  // Only flash plugin setting can be reached here.
  DCHECK(resource_identifier.empty());

  return EphemeralProvider::SetWebsiteSetting(
      primary_pattern, secondary_pattern, content_type, resource_identifier,
      std::move(in_value));
}

}  // namespace content_settings
