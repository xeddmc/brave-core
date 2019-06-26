/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */


// #include "base/path_service.h"
// #include "base/task/post_task.h"
// #include "base/test/thread_test_helper.h"
// #include "brave/browser/brave_browser_process_impl.h"
// #include "brave/common/brave_paths.h"
// #include "brave/components/brave_shields/browser/https_everywhere_service.h"
// #include "brave/components/brave_shields/common/brave_shield_constants.h"
// #include "chrome/browser/content_settings/host_content_settings_map_factory.h"
// #include "chrome/browser/extensions/extension_browsertest.h"
// #include "chrome/browser/net/url_request_mock_util.h"
// #include "chrome/browser/profiles/profile.h"
// #include "chrome/browser/ui/browser.h"
// #include "chrome/test/base/in_process_browser_test.h"
// #include "chrome/test/base/ui_test_utils.h"
// #include "components/content_settings/core/browser/host_content_settings_map.h"
// #include "components/content_settings/core/common/content_settings_pattern.h"
// #include "content/public/browser/browser_task_traits.h"
// #include "content/public/test/browser_test_utils.h"
// #include "net/dns/mock_host_resolver.h"

#include "base/path_service.h"
#include "base/strings/utf_string_conversions.h"
#include "base/test/thread_test_helper.h"
#include "brave/browser/brave_browser_process_impl.h"
#include "brave/browser/extensions/brave_extension_functional_test.h"
#include "brave/common/brave_paths.h"
#include "brave/common/pref_names.h"
#include "brave/common/url_constants.h"
#include "brave/components/brave_shields/browser/https_everywhere_service.h"
#include "chrome/browser/extensions/crx_installer.h"
#include "chrome/browser/extensions/extension_browsertest.h"
#include "chrome/browser/net/url_request_mock_util.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/test/browser_test_utils.h"

class BraveExtensionBrowserTest : public InProcessBrowserTest {
 public:
  using InProcessBrowserTest::InProcessBrowserTest;

  void SetUp() override {
    InitEmbeddedTestServer();
    InProcessBrowserTest::SetUp();
  }

  void InitEmbeddedTestServer() {
    brave::RegisterPathProvider();
    base::FilePath test_data_dir;
    base::PathService::Get(brave::DIR_TEST_DATA, &test_data_dir);
    embedded_test_server()->ServeFilesFromDirectory(test_data_dir);
    ASSERT_TRUE(embedded_test_server()->Start());
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(BraveExtensionBrowserTest);
};

IN_PROC_BROWSER_TEST_F(BraveExtensionBrowserTest, MutationObserverTriggeredWhenDOMChanged) {
  ASSERT_TRUE(true);
  GURL url = embedded_test_server()->GetURL("reddit.com", "/cosmetic-filter/mutation_observer.html");
  ui_test_utils::NavigateToURL(browser(), url);

  // GURL iframe_url = embedded_test_server()->GetURL("www.digg.com", "/");
  // const char kIframeID[] = "test";
  // content::WebContents* contents =  browser()->tab_strip_model()->GetActiveWebContents();
  // EXPECT_TRUE(NavigateIframeToURL(contents, kIframeID, iframe_url));
  // content::RenderFrameHost* iframe_contents = ChildFrameAt(contents->GetMainFrame(), 0);
  // WaitForLoadStop(contents);
  // EXPECT_EQ(GURL("https://www.digg.com/"), iframe_contents->GetLastCommittedURL());

}
