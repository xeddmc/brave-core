/* Copyright 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../../../extensions/browser/extensions_browser_client.cc"  // NOLINT

namespace extensions {

bool ExtensionsBrowserClient::HasTorContext(content::BrowserContext* context) {
  return false;
}

content::BrowserContext* ExtensionsBrowserClient::GetTorContext(content::BrowserContext* context) {
  return nullptr;
}

}  // namespace extensions