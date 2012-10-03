/*
 * Copyright (C) 2012 Mozilla Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include "libwpa_client/wpa_ctrl.h"

#define LOG_TAG "HOSTAPD"
#include "cutils/log.h"

/* socket pair used to exit from a blocking read */
static struct wpa_ctrl *ctrl_conn;
static struct wpa_ctrl *monitor_conn;
static const char *ctrl_iface_dir = CONFIG_CTRL_HOSTAPD_IFACE_DIR;
static char *ctrl_ifname = NULL;

static struct wpa_ctrl *
wifiOpenConnection(const char *ifname)
{
  char *cfile;
  int flen;

  if (ifname == NULL) {
    return NULL;
  }

  flen = strlen(ctrl_iface_dir) + strlen(ifname) + 2;
  cfile = malloc(flen);
  if (cfile == NULL) {
    return NULL;
  }
  snprintf(cfile, flen, "%s/%s", ctrl_iface_dir, ifname);

  ctrl_conn = wpa_ctrl_open(cfile);
  free(cfile);
  return ctrl_conn;
}

static int
wifiSendCommand(struct wpa_ctrl *ctrl, const char *cmd, char *reply, size_t *reply_len)
{
  int ret;

  if (ctrl_conn == NULL) {
    LOGV("Not connected to hostapd - \"%s\" command dropped.\n", cmd);
    return -1;
  }
  ret = wpa_ctrl_request(ctrl, cmd, strlen(cmd), reply, reply_len, NULL);
  if (ret == -2) {
    LOGD("'%s' command timed out.\n", cmd);
    return -2;
  } else if (ret < 0 || strncmp(reply, "FAIL", 4) == 0) {
    return -1;
  }
  if (strncmp(cmd, "PING", 4) == 0) {
    reply[*reply_len] = '\0';
  }
  return 0;
}

int
wifiConnectToHostapd()
{
  struct dirent *dent;

  DIR *dir = opendir(ctrl_iface_dir);
  if (dir) {
    while ((dent = readdir(dir))) {
      if (strcmp(dent->d_name, ".") == 0 ||
        strcmp(dent->d_name, "..") == 0) {
        continue;
      }
      LOGD("Selected interface '%s'", dent->d_name);
      ctrl_ifname = strdup(dent->d_name);
      break;
    }
    closedir(dir);
  }

  ctrl_conn = wifiOpenConnection(ctrl_ifname);
  if (ctrl_conn == NULL) {
    LOGE("Unable to open connection to hostapd on \"%s\": %s",
         ctrl_ifname, strerror(errno));
    return -1;
  }

  if (wpa_ctrl_attach(ctrl_conn) != 0) {
    wpa_ctrl_close(ctrl_conn);
    ctrl_conn = NULL;
    return -1;
  }

  return 0;
}

int
wifiCloseHostapdConnection()
{
  int ret;

  if (ctrl_conn == NULL) {
    LOGE("Invalid ctrl_conn.");
    return -1;
  }

  if (wpa_ctrl_detach(ctrl_conn) < 0) {
    LOGE("Failed to detach wpa_ctrl.");
  }

  wpa_ctrl_close(ctrl_conn);
  ctrl_conn = NULL;

  return 0;
}

int
wifiHostapdCommand(const char *command, char *reply, size_t *reply_len)
{
  return wifiSendCommand(ctrl_conn, command, reply, reply_len);
}

int
wifiAllStaCommand()
{
  char addr[32];
  char cmd[64];
  int sta_number = 0;
  size_t addr_len = sizeof(addr) - 1;
  size_t cmd_len = sizeof(cmd) - 1;

  if (wifiHostapdCommand("STA-FIRST", addr, &addr_len)) {
      return 0;
  }

  do {
      sta_number++;
      sprintf(cmd, "STA-NEXT %s", addr);
  } while (wifiHostapdCommand(cmd, addr, &cmd_len) == 0);

  return sta_number++;
}
