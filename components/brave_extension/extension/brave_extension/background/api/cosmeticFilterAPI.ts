/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/. */
import { Tab } from '../../types/state/shieldsPannelState'
// import * as shieldsPanelState from '../../state/shieldsPanelState'

export const addSiteCosmeticFilter = async (origin: string, cssfilter: string) => {
  chrome.storage.local.get('cosmeticFilterList', (storeData = {}) => {
    let storeList = Object.assign({}, storeData.cosmeticFilterList)
    if (storeList[origin] === undefined || storeList[origin].length === 0) { // nothing in filter list for origin
      storeList[origin] = [cssfilter]
    } else { // add entry
      storeList[origin].push(cssfilter)
    }
    chrome.storage.local.set({ 'cosmeticFilterList': storeList })
  })
}

export const removeSiteFilter = (origin: string) => {
  chrome.storage.local.get('cosmeticFilterList', (storeData = {}) => {
    let storeList = Object.assign({}, storeData.cosmeticFilterList)
    delete storeList[origin]
    chrome.storage.local.set({ 'cosmeticFilterList': storeList })
  })
}

export const removeAllFilters = () => {
  chrome.storage.local.set({ 'cosmeticFilterList': {} })
}

export const applyCSSCosmeticFilters = (tabData: Tab, tabId: number) => {
  chrome.storage.local.get('cosmeticFilterList', (storeData = {}) => { // fetch filter list
    if (!storeData.cosmeticFilterList) {
      if (process.env.NODE_ENV === 'shields_development') {
        console.log('applySiteFilters: no cosmetic filter store yet')
      }
      return
    }
    let hostname = tabData.hostname
    if (storeData.cosmeticFilterList[hostname] !== undefined) {
      storeData.cosmeticFilterList[hostname].map((filter: string) => { // if the filter hasn't been applied once before, apply it and set the corresponding filter to true
        if (process.env.NODE_ENV === 'shields_development') {
          console.log('applying filter', filter)
        }
        chrome.tabs.insertCSS({
          code: `${filter} {display: none;}`,
          runAt: 'document_start'
        })
      })
    }
  })
}

chrome.runtime.onConnect.addListener(function (port) {
  console.assert(port.name === 'knockknock')
  port.onMessage.addListener(function (msg) {
    if (msg.joke === 'Knock knock') {
      port.postMessage({ question: 'Who\'s there?' })
    } else if (msg.answer === 'Madame') {
      port.postMessage({ question: 'Madame who?' })
    } else if (msg.answer === 'Madame... Bovary') {
      port.postMessage({ question: "I don't get it." })
    }
  })
})
