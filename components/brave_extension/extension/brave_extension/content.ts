const unique = require('unique-selector').default
import { debounce } from '../../../common/debounce'
import { query } from './background/events/cosmeticFilterEvents'

let target: EventTarget | null
let contentSiteFilters: any
// let contentSiteFilters: Array<object>

if (process.env.NODE_ENV === 'development') {
  console.info('development content script here')
}

function getCurrentURL () {
  return window.location.hostname
}

// when page loads, grab filter list and only activate if there are rules
chrome.storage.local.get('cosmeticFilterList', (storeData = {}) => {
  if (!storeData.cosmeticFilterList) {
    if (process.env.NODE_ENV === 'development') {
      console.info('applySiteFilters: no cosmetic filter store yet')
    }
    return
  }
  Object.assign(contentSiteFilters, storeData.cosmeticFilterList[getCurrentURL()])
  console.log('current site list in content script', contentSiteFilters)
})

// on load retrieve each website's filter list
chrome.storage.local.get('cosmeticFilterList', (storeData = {}) => { // fetch filter list
  let notToBeApplied: Boolean
  // !storeData.cosmeticFilterList || storeData.cosmeticFilterList.length === 0 // if no rules, don't apply mutation observer

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
    applyDOMCosmeticFilterDebounce()
  } else {
    console.log('ON COMMITTED MUTATION OBSERVER NOT APPLIED')
  }
})

const applyDOMCosmeticFilterDebounce = function () {
  console.log(Date.now(), 'debounced call')
  let targetNode = document.body
  let observer = new MutationObserver(function (mutations) {
    let debouncedRemove = debounce((contentSiteFilters: Array<object>) => {
      // console.log('data')
      removeAll(contentSiteFilters)
    }, 1000 / 60)

  })
  let observerConfig = {
    childList: true,
    subtree: true
    // characterData: true
  }
  observer.observe(targetNode, observerConfig)
}

function removeAll (siteFilters: any) {
  /*
    let contentSiteFilters = [{
    'filter': 'filter1',
    'isIdempotent': true,
    'applied': false
  }, {
    'filter': 'filter2',
    'isIdempotent': false,
    'applied': false
  }, {
    'filter': 'filter3',
    'isIdempotent': false,
    'applied': false
  }]
  */
  // array of site filters, go through each one and check if idempotent/already applied
  siteFilters.map((filter) => {
    if (!filter.isIdempotent || !filter.applied) { // don't apply if filter is idempotent AND was already applied
      if (document.querySelector(siteFilters.length) > 0) { // attempt filter application
        document.querySelectorAll(siteFilters).forEach(e => {
          e.remove()
        })
      }
      filter.applied = true
    }
  })
}

// MutationObserver(applyDOMCosmeticFilters())

document.addEventListener('contextmenu', (event) => {
  // send host and store target
  // `target` needed for when background page handles `addBlockElement`
  target = event.target
  chrome.runtime.sendMessage({
    type: 'contextMenuOpened',
    baseURI: getCurrentURL()
  })
}, true)

chrome.runtime.onMessage.addListener((msg, sender, sendResponse) => {
  const action = typeof msg === 'string' ? msg : msg.type
  switch (action) {
    case 'getTargetSelector': {
      sendResponse(unique(target))
    }
  }
})
