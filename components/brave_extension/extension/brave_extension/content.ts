const unique = require('unique-selector').default
// import { debounce } from '../../../common/debounce'

let target: EventTarget | null
let siteFilters = {}

if (process.env.NODE_ENV === 'development') {
  console.info('development content script here')
}

// const applyDOMCosmeticFilterDebounce = debounce((data: any) => {
//   // applyDOMCosmeticFilters()
//   // sendMessage
//   console.log(Date.now(), 'debounced call')
// }, 1000 / 60) // 60 fps, 16.67ms minimum time between calls

function getCurrentURL () {
  return window.location.hostname
}

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

chrome.storage.local.get('cosmeticFilterList', (storeData = {}) => { // fetch filter list
  if (!storeData.cosmeticFilterList) {
    if (process.env.NODE_ENV === 'development') {
      console.info('applySiteFilters: no cosmetic filter store yet')
    }
    return
  }
  Object.assign(siteFilters, storeData.cosmeticFilterList)
  console.log(siteFilters)
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
    // applyDOMCosmeticFilterDebounce()
  } else {
    console.log('ON COMMITTED MUTATION OBSERVER NOT APPLIED')
  }
})

// const applyDOMCosmeticFilters = function () {
//   let hostname = getCurrentURL()
//   storeData.cosmeticFilterList[hostname].map((filter: string) => { // if the filter hasn't been applied once before, apply it and set the corresponding filter to true
//     let addedNodeList: NodeList
//     addedNodeList = document.querySelectorAll(filter)
//     console.log('${filter} exists:', addedNodeList.length > 0)
//     if (addedNodeList.length > 0) {
//       addedNodeList.forEach((node, currentIndex = 0) => {
//         // node.remove()
//         console.log('mutation observer: ${filter} removed')
//       })
//     }
//   })
// }

// let observer = new MutationObserver(function (mutations) {
//   mutations.forEach(function (mutation) {
//     console.log(mutation.type)
//   })
// })

// let observerConfig = {
//   attributes: true,
//   childList: true,
//   characterData: true
// }

// let targetNode = document.body
// observer.observe(targetNode, observerConfig)

// MutationObserver(applyDOMCosmeticFilters())

// export const applyDOMCosmeticFilters = (tabData: Tab, tabId: number) => {
//   let hostname = tabData.hostname
//   // let updatedFilterList = Object.assign(tabData.appliedFilterList)
//   chrome.storage.local.get('cosmeticFilterList', (storeData = {}) => { // fetch filter list
//     if (!storeData.cosmeticFilterList) {
//       console.info('applySiteFilters: no cosmetic filter store yet')
//       return
//     }

//   })
// }
