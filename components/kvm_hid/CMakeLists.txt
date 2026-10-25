idf_component_register(
    SRCS "usb_hid.c"
        "usb_xbox.c"
        "xbox_s3_route.c"
        "mmc.c"
        "jiggler.c"
    INCLUDE_DIRS "include"
    PRIV_INCLUDE_DIRS "s3_protocol"
    PRIV_REQUIRES
        kvm_board
        kvm_config
        kvm_storage
        esp_driver_gpio
        esp_tinyusb
        esp_http_client
        esp_timer
        lwip
        mbedtls
)
