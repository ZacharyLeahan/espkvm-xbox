# SPDX-License-Identifier: Apache-2.0
# Reuse the existing fork; the S3 defaults disable its KVM storage class.
set(KVM_TINYUSB_EXTRA_REQUIRES espressif__tinyusb)
include("${CMAKE_CURRENT_LIST_DIR}/../../../components/esp_tinyusb/CMakeLists.txt")
