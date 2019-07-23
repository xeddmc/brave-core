/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/network/services/brave_cookie_settings.h"

#include "brave/components/content_settings/core/common/content_settings_util.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"
#include "url/gurl.h"

using namespace net::registry_controlled_domains;  // NOLINT

namespace network {

namespace {

bool ShouldBlockCookie(bool allow_brave_shields,
                       bool allow_1p_cookies,
                       bool allow_3p_cookies,
                       const GURL& main_frame_url,
                       const GURL& url,
                       bool allow_google_auth) {
  // shields settings only apply to http/https
  if (!url.SchemeIsHTTPOrHTTPS()) {
    return false;
  }

  if (!allow_brave_shields) {
    return false;
  }

  // If 1p cookies are not allowed, then we just want to block everything.
  if (!allow_1p_cookies) {
    return true;
  }

  // If 3p is allowed, we have nothing extra to block
  if (allow_3p_cookies) {
    return false;
  }

  // If it is whitelisted, we shouldn't block
  if (content_settings::IsWhitelistedCookieException(main_frame_url,
                                                     url,
                                                     allow_google_auth))
    return false;

  // Same TLD+1 whouldn't set the referrer
  return !SameDomainOrHost(url, main_frame_url, INCLUDE_PRIVATE_REGISTRIES);
}

}  // namespace

BraveCookieSettings::BraveCookieSettings() : allow_google_auth_(true) {}

BraveCookieSettings::~BraveCookieSettings() {}

void BraveCookieSettings::GetCookieSetting(
    const GURL& url,
    const GURL& first_party_url,
    content_settings::SettingSource* source,
    ContentSetting* cookie_setting) const {
  GetCookieSetting(url, first_party_url, first_party_url, source,
                   cookie_setting);
}

void BraveCookieSettings::GetCookieSetting(
    const GURL& url,
    const GURL& first_party_url,
    const GURL& tab_url,
    content_settings::SettingSource* source,
    ContentSetting* cookie_setting) const {
  DCHECK(cookie_setting);

  GURL main_frame_url = first_party_url.is_empty() ? tab_url : first_party_url;

  if (main_frame_url.is_empty() || main_frame_url == GURL("about:blank"))
    main_frame_url = url;

  // TODO(bridiver) - update brave_shields content settings
  bool allow_brave_shields =
      IsAllowContentSetting(brave_shields_content_settings_,
                            main_frame_url,
                            main_frame_url,
                            CONTENT_SETTINGS_TYPE_PLUGINS,
                            brave_shields::kBraveShields);

  bool allow_1p_cookies =
      IsAllowContentSetting(content_settings_,
                            main_frame_url,
                            GURL("https://firstParty/"),
                            CONTENT_SETTINGS_TYPE_PLUGINS,
                            brave_shields::kCookies);

  bool allow_3p_cookies =
      IsAllowContentSetting(content_settings_,
                            main_frame_url,
                            GURL(),
                            CONTENT_SETTINGS_TYPE_PLUGINS,
                            brave_shields::kCookies);

  if (ShouldBlockCookie(allow_brave_shields,
                        allow_1p_cookies,
                        allow_3p_cookies,
                        main_frame_url,
                        url,
                        allow_google_auth_)) {
    *cookie_setting = CONTENT_SETTING_BLOCK;
  } else {
    return CookieSettings::GetCookieSetting(url,
                                            first_party_url,
                                            source,
                                            cookie_setting);
  }
}

}  // namespace content_settings
