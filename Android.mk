# Copyright (C) 2012 Mozilla Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libhostapd
LOCAL_SRC_FILES := libhostapd.c
LOCAL_SHARED_LIBRARIES := libcutils libwpa_client
LOCAL_CFLAGS := -DCONFIG_CTRL_HOSTAPD_IFACE_DIR=\"/data/misc/wifi/hostapd\"
LOCAL_MODULE_TAGS := optional eng
include $(BUILD_SHARED_LIBRARY)

# libhostapd command line tool 
include $(CLEAR_VARS)
LOCAL_MODULE := hostapdcmd
LOCAL_SRC_FILES := libhostapd_cmd.c
LOCAL_SHARED_LIBRARIES := libhostapd libcutils
LOCAL_CFLAGS := $(LIBRECOVERY_CFLAGS)
LOCAL_MODULE_TAGS := tests
include $(BUILD_EXECUTABLE)

