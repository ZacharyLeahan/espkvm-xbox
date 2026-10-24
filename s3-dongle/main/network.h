/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdint.h>
#include <stdbool.h>
void s3_network_start(void);
void s3_network_status(char *out,unsigned size);
bool s3_network_owns_input(void);
