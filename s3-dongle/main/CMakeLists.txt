# SPDX-License-Identifier: Apache-2.0
idf_component_register(SRCS "main.c" "network.c" "usb_xbox.c" INCLUDE_DIRS "." "../../components/kvm_hid/s3_protocol" REQUIRES esp_tinyusb esp_driver_gpio esp_timer esp_wifi nvs_flash esp_http_server esp_netif mbedtls)
target_compile_definitions(${COMPONENT_LIB} PRIVATE CONFIG_KVM_USB_XBOX=1)
