/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/views/translate/brave_translate_icon_view.h"

#include "brave/browser/translate/buildflags/buildflags.h"

#if BUILDFLAG(ENABLE_BRAVE_TRANSLATE_EXTENSION)
#define TranslateIconView BraveTranslateIconView
#endif
#include "../../../../../../../chrome/browser/ui/views/page_action/omnibox_page_action_icon_container_view.cc"
#if BUILDFLAG(ENABLE_BRAVE_TRANSLATE_EXTENSION)
#undef TranslateIconView
#endif
