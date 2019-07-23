/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_SERVICES_NETWORK_BRAVE_COOKIE_SETTINGS_H_
#define BRAVE_SERVICES_NETWORK_BRAVE_COOKIE_SETTINGS_H_

#include "components/content_settings/core/common/content_settings.h"
#include "services/network/cookie_settings.h"

namespace network {

class BraveCookieSettings : public network::CookieSettings {
 public:
  BraveCookieSettings();
  ~BraveCookieSettings() override;

  void GetCookieSetting(const GURL& url,
                        const GURL& first_party_url,
                        content_settings::SettingSource* source,
                        ContentSetting* cookie_setting) const override;

 private:
  bool allow_google_auth_;
  ContentSettingsForOneType brave_shields_content_settings_;

  DISALLOW_COPY_AND_ASSIGN(BraveCookieSettings);
};

}  // namespace content_settings

#endif  // BRAVE_SERVICES_NETWORK_BRAVE_COOKIE_SETTINGS_H_
