/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/playlists/browser/playlists_service.h"

#include <string>

#include "brave/common/extensions/api/brave_playlists.h"
#include "brave/components/playlists/browser/playlists_controller.h"
#include "chrome/browser/profiles/profile.h"
#include "extensions/browser/event_router.h"

namespace {
extensions::EventRouter* GetEventRouter(Profile* profile) {
  return extensions::EventRouter::Get(profile);
}

std::string ConvertPlaylistsChangeType(PlaylistsChangeParams::ChangeType type) {
  switch (type) {
    case PlaylistsChangeParams::ChangeType::CHANGE_TYPE_ADD:
      return "add";
    case PlaylistsChangeParams::ChangeType::CHANGE_TYPE_DELETE:
      return "delete";
    case PlaylistsChangeParams::ChangeType::CHANGE_TYPE_ABORT:
      return "abort";
    case PlaylistsChangeParams::ChangeType::CHANGE_TYPE_READY:
      return "ready";
    default:
      NOTREACHED();
      return "unknown";
  }
}
}  // namespace

PlaylistsService::PlaylistsService(Profile* profile)
    : controller_(new PlaylistsController),
      observer_(this),
      profile_(profile) {
  observer_.Add(controller_.get());
  controller_->Init();
}

PlaylistsService::~PlaylistsService() {
  DCHECK(!IsInObserverList());
}

void PlaylistsService::OnPlaylistsInitialized() {
  auto event = std::make_unique<extensions::Event>(
      extensions::events::BRAVE_PLAYLISTS_ON_INITIALIZED,
      extensions::api::brave_playlists::OnInitialized::kEventName,
      nullptr,
      profile_);

  GetEventRouter(profile_)->BroadcastEvent(std::move(event));
}

void PlaylistsService::OnPlaylistsChanged(
    const PlaylistsChangeParams& params) {
  auto event = std::make_unique<extensions::Event>(
      extensions::events::BRAVE_PLAYLISTS_ON_PLAYLISTS_CHANGED,
      extensions::api::brave_playlists::OnPlaylistsChanged::kEventName,
      extensions::api::brave_playlists::OnPlaylistsChanged::Create(
          ConvertPlaylistsChangeType(params.type), params.playlist_id),
      profile_);

  GetEventRouter(profile_)->BroadcastEvent(std::move(event));
}
