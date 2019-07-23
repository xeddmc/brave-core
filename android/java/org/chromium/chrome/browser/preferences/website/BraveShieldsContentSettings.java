/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.preferences.website;

import org.chromium.base.annotations.CalledByNative;
import org.chromium.base.annotations.JCaller;
import org.chromium.base.annotations.JNINamespace;
import org.chromium.base.annotations.NativeMethods;
import org.chromium.base.Log;
import org.chromium.chrome.browser.ContentSettingsType;
import org.chromium.chrome.browser.preferences.PrefServiceBridge;
import org.chromium.chrome.browser.preferences.website.BraveShieldsContentSettingsObserver;
import org.chromium.chrome.browser.preferences.website.WebsitePreferenceBridge;

import java.util.List;

@JNINamespace("chrome::android")
public class BraveShieldsContentSettings {
    static public final String RESOURCE_IDENTIFIER_ADS = "ads";
    static public final String RESOURCE_IDENTIFIER_TRACKERS = "trackers";
    static public final String RESOURCE_IDENTIFIER_HTTP_UPGRADABLE_RESOURCES = "httpUpgradableResources";
    static public final String RESOURCE_IDENTIFIER_BRAVE_SHIELDS = "braveShields";
    static public final String RESOURCE_IDENTIFIER_FINGERPRINTING = "fingerprinting";
    static public final String RESOURCE_IDENTIFIER_COOKIES = "cookies";
    static public final String RESOURCE_IDENTIFIER_REFERRERS = "referrers";
    static public final String RESOURCE_IDENTIFIER_JAVASCRIPTS = "javascript";

    static private final String blockResource = "block";
    static private final String allowResource = "allow";

    private long mNativeBraveShieldsContentSettings;
    private BraveShieldsContentSettingsObserver mBraveShieldsContentSettingsObserver;

    public BraveShieldsContentSettings(BraveShieldsContentSettingsObserver braveShieldsContentSettingsObserver) {
        mBraveShieldsContentSettingsObserver = braveShieldsContentSettingsObserver;
        mNativeBraveShieldsContentSettings = 0;
        init();
    }

    private void init() {
        if (mNativeBraveShieldsContentSettings == 0) {
            BraveShieldsContentSettingsJni.get().init(this);
        }
    }

    public void destroy() {
        if (mNativeBraveShieldsContentSettings == 0) {
            return;
        }
        BraveShieldsContentSettingsJni.get().destroy(mNativeBraveShieldsContentSettings);
    }

    static public void setShields(boolean incognito, String host, String resourceIndentifier, boolean value,
            boolean fromTopShields) {
        if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_JAVASCRIPTS)) {
            BraveShieldsContentSettings.setJavaScriptBlock(incognito, host, value, fromTopShields, resourceIndentifier);
        }
        String setting_string = (value ? blockResource : allowResource);
        BraveShieldsContentSettingsJni.get().setShields(incognito, host, resourceIndentifier, setting_string);
    }

    public static boolean getShields(boolean incognito, String host, String resourceIndentifier) {
        if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_JAVASCRIPTS)) {
            return !BraveShieldsContentSettings.isJavaScriptEnabled(incognito, host);
        }
        String settings = BraveShieldsContentSettingsJni.get().getShields(incognito, host, resourceIndentifier);
        if (resourceIndentifier.equals(RESOURCE_IDENTIFIER_FINGERPRINTING) &&
                !settings.equals(allowResource) && !settings.equals(blockResource)) {
            return false;
        }

        return !settings.equals(allowResource); 
    }

    private static boolean isJavaScriptEnabled(boolean incognitoTab, String host) {
        host = CutWwwPrefix(host);
        // TODO JavaScript for incognito profiles. Check that
        // https://github.com/brave/browser-android-tabs/commit/e1dd6f7797398155d640303e05241f9fb2b433f9#diff-e1d5c8c446116e371020d44baa09d09bR189
        WebsitePreferenceBridge websitePreferenceBridge = new WebsitePreferenceBridge();
        List<ContentSettingException> exceptions = (incognitoTab) ?
            websitePreferenceBridge.getContentSettingsExceptions(ContentSettingsType.CONTENT_SETTINGS_TYPE_JAVASCRIPT) :
            websitePreferenceBridge.getContentSettingsExceptions(ContentSettingsType.CONTENT_SETTINGS_TYPE_JAVASCRIPT);

        for (ContentSettingException exception : exceptions) {
            String pattern = exception.getPattern();

            pattern = CutWwwPrefix(pattern);
            if (!pattern.equals(host)) {
                continue;
            }

            if (ContentSettingValues.ALLOW == exception.getContentSetting()) {
                return true;
            } else {
                return false;
            }
        }

        if (incognitoTab) {
          //for incognito tab inherit settings from normal tab
          return isJavaScriptEnabled(false, host);
        }

        if (!PrefServiceBridge.getInstance().isCategoryEnabled(ContentSettingsType.CONTENT_SETTINGS_TYPE_JAVASCRIPT)) {
            return false;
        }

        return true;
    }

    private static void setJavaScriptBlock(boolean incognitoTab, String host, boolean block, 
            boolean fromTopShields, String resourceIndentifier) {
        String hostForAccessMap = CutWwwPrefix(host);

        int setting = ContentSettingValues.ALLOW;
        if (block && !fromTopShields) {
            setting = ContentSettingValues.BLOCK;
        }

        if (fromTopShields) {
          // when change comes from Top Shields:
          // in anyway do not touch hostSettings string
          // block is false => we should allow js
          // block is true => we should switch js back according to hostSettings string
          if (!block) {
            setting = ContentSettingValues.ALLOW;
          } else {
            setting = (!BraveShieldsContentSettingsJni.get().getShields(incognitoTab, host, resourceIndentifier).equals(allowResource)) ? 
                ContentSettingValues.ALLOW : ContentSettingValues.BLOCK;
          }
        }

        if (incognitoTab) {
            // TODO JavaScript for incognito profiles. Check that
            // https://github.com/brave/browser-android-tabs/commit/e1dd6f7797398155d640303e05241f9fb2b433f9#diff-e1d5c8c446116e371020d44baa09d09bR189
            // PrefServiceBridge.getInstance().nativeSetContentSettingForPatternIncognito(
            //           ContentSettingsType.CONTENT_SETTINGS_TYPE_JAVASCRIPT, host,
            //           setting);
            PrefServiceBridge.getInstance().nativeSetContentSettingForPattern(
                        ContentSettingsType.CONTENT_SETTINGS_TYPE_JAVASCRIPT, host,
                        setting);
        } else {
            PrefServiceBridge.getInstance().nativeSetContentSettingForPattern(
                      ContentSettingsType.CONTENT_SETTINGS_TYPE_JAVASCRIPT, host,
                      setting);
        }
    }

    private static String CutWwwPrefix(String host) {
        if (null != host && host.startsWith("www.")) {
            host = host.substring("www.".length());
        }
        return host;
    }

    @CalledByNative
    private void setNativePtr(long nativePtr) {
        assert mNativeBraveShieldsContentSettings == 0;
        mNativeBraveShieldsContentSettings = nativePtr;
    }

    @CalledByNative
    private void blockedEvent(int tabId, String block_type, String subresource) {
        assert mBraveShieldsContentSettingsObserver != null;
        if (mBraveShieldsContentSettingsObserver == null) {
            return;
        }
        mBraveShieldsContentSettingsObserver.blockEvent(tabId, block_type, subresource);
    }

    @NativeMethods
    interface Natives {
        void init(@JCaller BraveShieldsContentSettings self);
        void destroy(long nativeBraveShieldsContentSettings);
        void setShields(boolean incognito, String host, String resourceIndentifier, String value);
        String getShields(boolean incognito, String host, String resourceIndentifier);
    }
}
