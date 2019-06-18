/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

// Types
import * as shieldsPanelTypes from '../../constants/shieldsPanelTypes'
import * as windowTypes from '../../constants/windowTypes'
import * as tabTypes from '../../constants/tabTypes'
import * as webNavigationTypes from '../../constants/webNavigationTypes'
import * as cosmeticFilterTypes from '../../constants/cosmeticFilterTypes'
import { State } from '../../types/state/shieldsPannelState'
import { Actions } from '../../types/actions/index'

// APIs
import { setAllowBraveShields, requestShieldPanelData } from '../api/shieldsAPI'
import { reloadTab } from '../api/tabsAPI'
import {
  removeSiteFilter,
  addSiteCosmeticFilter,
  applyDOMCosmeticFilters,
  applyCSSCosmeticFilters,
  removeAllFilters
} from '../api/cosmeticFilterAPI'
import { debounce } from '../../../../../common/debounce'

// State helpers
import * as shieldsPanelState from '../../state/shieldsPanelState'
import * as noScriptState from '../../state/noScriptState'
import { getOrigin } from '../../helpers/urlUtils'

// export const debounce = function<T>(fn: (data: T) => void, bufferInterval: number, ...args: Array<any>) {
const applyDOMCosmeticFilterDebounce = debounce((data: any) => {
  // chrome.send('brave_adblock.updateCustomFilters', [customFilters])
  applyDOMCosmeticFilters(data.tabData, data.tabId)
}, 1000 / 60) // 60 fps

const focusedWindowChanged = (state: State, windowId: number): State => {
  if (windowId !== -1) {
    state = shieldsPanelState.updateFocusedWindow(state, windowId)
    if (shieldsPanelState.getActiveTabId(state)) {
      requestShieldPanelData(shieldsPanelState.getActiveTabId(state))
    } else {
      console.warn('no tab id so cannot request shield data from window focus change!')
    }
  }
  return state
}

const updateActiveTab = (state: State, windowId: number, tabId: number): State => {
  requestShieldPanelData(tabId)
  return shieldsPanelState.updateActiveTab(state, windowId, tabId)
}

export default function cosmeticFilterReducer (state: State = {
  tabs: {},
  windows: {},
  currentWindowId: -1 },
  action: Actions) {
  switch (action.type) {
    case webNavigationTypes.ON_COMMITTED: {
      const tabData = shieldsPanelState.getActiveTabData(state)
      if (!tabData) {
        console.error('Active tab not found')
        break
      }
      if (action.isMainFrame) {
        state = shieldsPanelState.resetBlockingStats(state, action.tabId)
        state = shieldsPanelState.resetBlockingResources(state, action.tabId)
        state = noScriptState.resetNoScriptInfo(state, action.tabId, getOrigin(action.url))
      }
      // applySiteFilters(tabData.hostname)
      // applyCSSCosmeticFilters(tabData)
      // check if storage list is > 0
      // if it is, call applyCOMCosmeticFilterDebounce
      // updateCustomFilters(state.settings.customFilters)

      chrome.storage.local.get('cosmeticFilterList', (storeData = {}) => { // fetch filter list
        let notToBeApplied: Boolean
        // !storeData.cosmeticFilterList || storeData.cosmeticFilterList.length === 0 // if it doesn't exist, don't apply mutation observer
        if (!storeData.cosmeticFilterList) {
          notToBeApplied = true
          console.log('storeData.cosmeticFilterList does not exist')
        } else if (Object.keys(storeData.cosmeticFilterList).length === 0) {
          notToBeApplied = true
          console.log('storeData.cosmeticFilterList length === 0')
        } else {
          notToBeApplied = false
        }
        if (!notToBeApplied) { // to be applied
          console.log('ON COMMITTED MUTATION OBSERVER BEING APPLIED:')
          chrome.storage.local.get('cosmeticFilterList', (storeData = {}) => { // fetch filter list
            console.log('cosmeticFilterList.length:', Object.keys(storeData.cosmeticFilterList).length)
          })
          applyDOMCosmeticFilterDebounce({
            tabData: tabData,
            tabId: action.tabId
          })
        } else {
          console.log('ON COMMITTED MUTATION OBSERVER NOT APPLIED')
        }
      })
      console.log('applying CSS filters')
      applyCSSCosmeticFilters(tabData, action.tabId)
      break
    }
    case windowTypes.WINDOW_REMOVED: {
      state = shieldsPanelState.removeWindowInfo(state, action.windowId)
      break
    }
    case windowTypes.WINDOW_CREATED: {
      if (action.window.focused || Object.keys(state.windows).length === 0) {
        state = focusedWindowChanged(state, action.window.id)
      }
      break
    }
    case windowTypes.WINDOW_FOCUS_CHANGED: {
      state = focusedWindowChanged(state, action.windowId)
      break
    }
    case tabTypes.ACTIVE_TAB_CHANGED: {
      const windowId: number = action.windowId
      const tabId: number = action.tabId
      state = updateActiveTab(state, windowId, tabId)
      break
    }
    case tabTypes.TAB_DATA_CHANGED: {
      const tab: chrome.tabs.Tab = action.tab
      if (tab.active && tab.id) {
        state = updateActiveTab(state, tab.windowId, tab.id)
      }
      break
    }
    case tabTypes.TAB_CREATED: {
      const tab: chrome.tabs.Tab = action.tab
      if (!tab) {
        break
      }

      if (tab.active && tab.id) {
        state = updateActiveTab(state, tab.windowId, tab.id)
      }
      break
    }
    case shieldsPanelTypes.SHIELDS_PANEL_DATA_UPDATED: {
      state = shieldsPanelState.updateTabShieldsData(state, action.details.id, action.details)
      break
    }
    case shieldsPanelTypes.SHIELDS_TOGGLED: {
      const tabId: number = shieldsPanelState.getActiveTabId(state)
      const tabData = shieldsPanelState.getActiveTabData(state)
      if (!tabData) {
        console.error('Active tab not found')
        break
      }
      setAllowBraveShields(tabData.origin, action.setting)
        .then(() => {
          reloadTab(tabId, true).catch((e) => {
            console.error('Tab reload was not successful', e)
          })
          requestShieldPanelData(shieldsPanelState.getActiveTabId(state))
        })
        .catch((e: any) => {
          console.error('Could not set shields', e)
        })
      state = shieldsPanelState
        .updateTabShieldsData(state, tabId, { braveShields: action.setting })
      break
    }
    case cosmeticFilterTypes.SITE_COSMETIC_FILTER_REMOVED: {
      let url = action.origin
      removeSiteFilter(url)
      break
    }
    case cosmeticFilterTypes.ALL_COSMETIC_FILTERS_REMOVED: {
      removeAllFilters()
      break
    }
    case cosmeticFilterTypes.SITE_COSMETIC_FILTER_ADDED: {
      const tabData = shieldsPanelState.getActiveTabData(state)
      const tabId: number = shieldsPanelState.getActiveTabId(state)
      if (!tabData) {
        console.error('Active tab not found')
        break
      }
      addSiteCosmeticFilter(tabData.hostname, action.cssfilter)
        .then(() => {
          console.log(`added: ${tabData.hostname} | ${action.cssfilter}`)
        })
        .catch(e => {
          console.error('Could not add filter:', e)
        })
      applyCSSCosmeticFilters(tabData, tabId)
      break

    }
    // case cosmeticFilterTypes.SITE_DOM_COSMETIC_FILTER_APPLIED: {
    //   const tabData = shieldsPanelState.getActiveTabData(state)
    //   const tabId: number = shieldsPanelState.getActiveTabId(state)
    //   if (!tabData) {
    //     console.error('Active tab not found')
    //     break
    //   }
    //   // let updatedFilterList = applySiteFilters(tabData, tabId)
    //   applyDOMCosmeticFilters(tabData, tabId)
    //   // shieldsPanelState.updateTabShieldsData(state, tabId, { appliedFilterList: updatedFilterList })
    //   break
    // }
    // case cosmeticFilterTypes.SITE_CSS_COSMETIC_FILTER_APPLIED: {
    //   const tabData = shieldsPanelState.getActiveTabData(state)
    //   const tabId: number = shieldsPanelState.getActiveTabId(state)
    //   if (!tabData) {
    //     console.error('Active tab not found')
    //     break
    //   }
    //   // let updatedFilterList = applySiteFilters(tabData, tabId)
    //   applyCSSCosmeticFilters(tabData, tabId)
    //   // shieldsPanelState.updateTabShieldsData(state, tabId, { appliedFilterList: updatedFilterList })
    //   break
    // }

  }
  return state
}
