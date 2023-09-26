/****************************************************************************
 * vendor/sim/boards/miwear/src/ap.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/board.h>
#include <nuttx/clock.h>
#include <nuttx/kmalloc.h>
#include <nuttx/mtd/mtd.h>
#include <nuttx/fs/fs.h>
#include <nuttx/drivers/ramdisk.h>
#include <nuttx/sensors/fakesensor.h>
#include <nuttx/timers/oneshot.h>
#include <nuttx/video/fb.h>
#include <nuttx/video/video.h>
#include <nuttx/timers/oneshot.h>
#include <nuttx/wireless/pktradio.h>
#include <nuttx/wireless/bluetooth/bt_null.h>
#include <nuttx/wireless/bluetooth/bt_uart_shim.h>
#include <nuttx/wireless/ieee802154/ieee802154_loopback.h>

#include <syslog.h>

#ifdef CONFIG_LCD_DEV
#include <nuttx/lcd/lcd_dev.h>
#endif

#if defined(CONFIG_INPUT_BUTTONS_LOWER) && defined(CONFIG_SIM_BUTTONS)
#include <nuttx/input/buttons.h>
#endif

#include "sim_internal.h"

/* procfs File System */

#ifdef CONFIG_FS_PROCFS
#  ifdef CONFIG_NSH_PROC_MOUNTPOINT
#    define SIM_PROCFS_MOUNTPOINT CONFIG_NSH_PROC_MOUNTPOINT
#  else
#    define SIM_PROCFS_MOUNTPOINT "/proc"
#  endif
#endif

/****************************************************************************
 * Name: sim_bringup
 *
 * Description:
 *   Bring up simulated board features
 *
 ****************************************************************************/

static int sim_bringup(void)
{
#ifdef CONFIG_ONESHOT
  struct oneshot_lowerhalf_s *oneshot;
#endif
#ifdef CONFIG_RAMMTD
  uint8_t *ramstart;
#endif
#if !defined(CONFIG_DISABLE_MOUNTPOINT)
  uint8_t *ramdiskstart;
#endif
  int ret = OK;

#ifdef CONFIG_RAMMTD
  /* Create a RAM MTD device if configured */

  ramstart = (uint8_t *)kmm_malloc(128 * 1024);
  if (ramstart == NULL)
    {
      syslog(LOG_ERR, "ERROR: Allocation for RAM MTD failed\n");
    }
  else
    {
      /* Initialized the RAM MTD */

      struct mtd_dev_s *mtd = rammtd_initialize(ramstart, 128 * 1024);
      if (mtd == NULL)
        {
          syslog(LOG_ERR, "ERROR: rammtd_initialize failed\n");
          kmm_free(ramstart);
        }
      else
        {
          /* Erase the RAM MTD */

          ret = mtd->ioctl(mtd, MTDIOC_BULKERASE, 0);
          if (ret < 0)
            {
              syslog(LOG_ERR, "ERROR: IOCTL MTDIOC_BULKERASE failed\n");
            }

          /* Register the MTD driver so that it can be accessed from the
           * VFS.
           */

          ret = register_mtddriver("/dev/rammtd", mtd, 0755, NULL);
          if (ret < 0)
            {
              syslog(LOG_ERR, "ERROR: Failed to register MTD driver: %d\n",
                     ret);
            }
        }
    }
#endif

#if !defined(CONFIG_DISABLE_MOUNTPOINT)
  ramdiskstart = (uint8_t *)kmm_malloc(512 * 2048);
  ret = ramdisk_register(1, ramdiskstart, 2048, 512,
                         RDFLAG_WRENABLED | RDFLAG_FUNLINK);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: Failed to register ramdisk: %d\n", ret);
    }
#endif

#ifdef CONFIG_ONESHOT
  /* Get an instance of the simulated oneshot timer */

  oneshot = oneshot_initialize(0, 0);
  if (oneshot == NULL)
    {
      syslog(LOG_ERR, "ERROR: oneshot_initialize failed\n");
    }
  else
    {
#ifdef CONFIG_CPULOAD_ONESHOT
      /* Configure the oneshot timer to support CPU load measurement */

      nxsched_oneshot_extclk(oneshot);

#else
      /* Initialize the simulated oneshot driver */

      ret = oneshot_register("/dev/oneshot", oneshot);
      if (ret < 0)
        {
          syslog(LOG_ERR,
                 "ERROR: Failed to register oneshot at /dev/oneshot: %d\n",
                 ret);
        }
#endif
    }
#endif

#ifdef CONFIG_INPUT_AJOYSTICK
  /* Initialize the simulated analog joystick input device */

  sim_ajoy_initialize();
#endif

#ifdef CONFIG_VIDEO_FB
  /* Initialize and register the simulated framebuffer driver */

  ret = fb_register(0, 0);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: fb_register() failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_SIM_VIDEO
  /* Initialize and register the simulated video driver */

  ret = video_initialize(CONFIG_SIM_VIDEO_DEV_PATH);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: video_initialize() failed: %d\n", ret);
    }

  sim_video_initialize();
#endif

#ifdef CONFIG_LCD

  ret = board_lcd_initialize();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: board_lcd_initialize() failed: %d\n", ret);
    }

#  ifdef CONFIG_LCD_DEV

  ret = lcddev_register(0);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: lcddev_register() failed: %d\n", ret);
    }

#  endif

#endif

#ifdef CONFIG_SIM_TOUCHSCREEN
  /* Initialize the touchscreen */

  ret = sim_tsc_initialize(0);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: sim_tsc_initialize failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_SIM_KEYBOARD
  /* Initialize the keyboard */

  ret = sim_kbd_initialize();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: sim_kbd_initialize failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_IEEE802154_LOOPBACK
  /* Initialize and register the IEEE802.15.4 MAC network loop device */

  ret = ieee8021514_loopback();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: ieee8021514_loopback() failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_PKTRADIO_LOOPBACK
  /* Initialize and register the IEEE802.15.4 MAC network loop device */

  ret = pktradio_loopback();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: pktradio_loopback() failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_BLUETOOTH_NULL
  /* Register the NULL Bluetooth network device */

  ret = btnull_register();
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: btnull_register() failed: %d\n", ret);
    }
#endif

#ifdef CONFIG_SIM_HCISOCKET
  /* Register the Host Bluetooth network device via HCI socket */

  ret = sim_bthcisock_register(CONFIG_SIM_HCISOCKET_DEVID);
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: sim_bthcisock_register() failed: %d\n", ret);
    }
#endif

#if defined(CONFIG_INPUT_BUTTONS_LOWER) && defined(CONFIG_SIM_BUTTONS)
  ret = btn_lower_initialize("/dev/buttons");
  if (ret < 0)
    {
      syslog(LOG_ERR, "ERROR: btn_lower_initialize() failed: %d\n", ret);
    }
#endif

  return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

void board_early_initialize(void)
{
  return;
}

void board_late_initialize(void)
{
  sim_bringup();
  return;
}

int board_app_initialize(uintptr_t arg)
{
  return 0;
}
