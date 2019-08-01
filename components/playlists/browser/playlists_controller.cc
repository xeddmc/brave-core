/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/playlists/browser/playlists_controller.h"

#include "brave/components/playlists/browser/playlists_controller_observer.h"

PlaylistsController::PlaylistsController() {
}

PlaylistsController::~PlaylistsController() {
}

void PlaylistsController::Init() {
}

void PlaylistsController::CreatePlaylist(const CreatePlaylistParams& params) {
}

std::vector<PlaylistInfo> PlaylistsController::GetAllPlaylists() {
  return std::vector<PlaylistInfo>();
}

PlaylistInfo PlaylistsController::GetPlaylist(const std::string& id) {
  return PlaylistInfo();
}

void PlaylistsController::DeletePlaylist(const std::string& id) {
}

void PlaylistsController::DeleteAllPlaylists() {
}

void PlaylistsController::AddObserver(PlaylistsControllerObserver* observer) {
  observers_.AddObserver(observer);
}

void PlaylistsController::RemoveObserver(
    PlaylistsControllerObserver* observer) {
  observers_.RemoveObserver(observer);
}
