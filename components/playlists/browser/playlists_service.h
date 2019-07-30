/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_PLAYLISTS_BROWSER_PLAYLISTS_SERVICE_H_
#define BRAVE_COMPONENTS_PLAYLISTS_BROWSER_PLAYLISTS_SERVICE_H_

#include <memory>

#include "base/macros.h"
#include "base/scoped_observer.h"
#include "brave/components/playlists/browser/playlists_controller_observer.h"
#include "components/keyed_service/core/keyed_service.h"

class PlaylistsController;
class Profile;

class PlaylistsService : public KeyedService,
                         public PlaylistsControllerObserver {
 public:
  explicit PlaylistsService(Profile* profile);
  ~PlaylistsService() override;

  PlaylistsController* controller() const { return controller_.get(); }

 private:
  // PlaylistsControllerObserver overrides:
  void OnPlaylistsInitialized() override;
  void OnPlaylistsChanged(const PlaylistsChangeParams& params) override;

  std::unique_ptr<PlaylistsController> controller_;
  ScopedObserver<PlaylistsController, PlaylistsControllerObserver> observer_;
  Profile* profile_;

  DISALLOW_COPY_AND_ASSIGN(PlaylistsService);
};

#endif  // BRAVE_COMPONENTS_PLAYLISTS_BROWSER_PLAYLISTS_SERVICE_H_
