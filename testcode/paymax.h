/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef ANDROID_PAYMAX_INTERFACE_H
#define ANDROID_PAYMAX_INTERFACE_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <hardware/hardware.h>

__BEGIN_DECLS

/**
 * The id of this module
 */
#define PAYMAX_HARDWARE_MODULE_ID "paymax"

struct paymax_device_t {
    struct hw_device_t common;
	void (*writeCmdToMax)(struct paymax_device_t *dev, const char* cmd_buf,unsigned int cmd_len);
    unsigned int (*readDataFromMax)(struct paymax_device_t *dev, char* read_buf, unsigned int read_len, unsigned int timeout);
};


__END_DECLS

#endif  // ANDROID_PAYMAX_INTERFACE_H

