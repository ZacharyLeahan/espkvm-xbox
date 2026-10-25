/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
esp_err_t xbox_s3_route_init(void);
void xbox_s3_route_tick(const uint8_t reports[4][20]);
bool xbox_s3_route_ready(void);
bool xbox_s3_route_alive(void);
