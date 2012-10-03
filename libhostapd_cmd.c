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

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <pwd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <linux/prctl.h>
#include <utils/Log.h>
#include <cutils/sockets.h>

int main(int argc, char **argv) {
  int ret = 0;
  char cmd[128];
  char reply[2048];
  size_t len = sizeof(reply) - 1;

  if (argc <= 1) {
    LOGE("Command syntax is => hostapdcmd [cmd]");
    return 0;
  }

  ret = wifiConnectToHostapd();
  if (ret < 0) {
    LOGE("Connecting to hostapd failed: %d", ret);
    return 0;
  }

  strcpy(cmd, argv[1]);
  LOGE("Command: %s", cmd);

  ret = wifiHostapdCommand(cmd, reply, &len);
  if (ret < 0) {
    LOGE("Send hostapd command failed: %d", ret);
    return 0;
  }

  reply[len] = 0;
  LOGE("Lenght: %d Reply: %s", len, reply);

  wifiCloseHostapdConnection();

  return 0;
}
