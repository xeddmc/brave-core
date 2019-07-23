/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "services/network/network_context.h"

// TODO(bridiver) - use BraveCookieSettings because we need to check firstPartyUrl
#define BRAVE_ON_CAN_GET_COOKIES_INTERNAL \
network_context_->cookie_manager() \
                ->cookie_settings() \
                .IsCookieAccessAllowed( \
                    request.url(), \
                    GURL(request.network_isolation_key().ToString())) &&

#define BRAVE_ON_CAN_SET_COOKIES_INTERNAL BRAVE_ON_CAN_GET_COOKIES_INTERNAL

#include "../../../../services/network/network_context.cc"  // NOLINT
#undef BRAVE_ON_CAN_GET_COOKIES_INTERNAL
#undef BRAVE_ON_CAN_SET_COOKIES_INTERNAL
