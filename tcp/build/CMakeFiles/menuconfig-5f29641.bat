cd /D C:\Users\16339\Downloads\esp-idf-v3.3.1\tcp\build || exit /b
C:\Users\16339\.espressif\python_env\idf3.3_py3.7_env\Scripts\python.exe C:/Users/16339/Downloads/esp-idf-v3.3.1/tools/kconfig_new/confgen.py --kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/Kconfig --config C:/Users/16339/Downloads/esp-idf-v3.3.1/tcp/sdkconfig --env "COMPONENT_KCONFIGS= C:/Users/16339/Downloads/esp-idf-v3.3.1/components/app_trace/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/aws_iot/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/bt/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/driver/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/efuse/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esp32/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esp_adc_cal/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esp_event/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esp_http_client/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esp_http_server/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esp_https_ota/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/espcoredump/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/ethernet/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/fatfs/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/freemodbus/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/freertos/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/heap/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/libsodium/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/log/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/lwip/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/mbedtls/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/mdns/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/mqtt/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/nvs_flash/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/openssl/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/pthread/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/spi_flash/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/spiffs/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/tcpip_adapter/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/unity/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/vfs/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/wear_levelling/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/wifi_provisioning/Kconfig" --env "COMPONENT_KCONFIGS_PROJBUILD= C:/Users/16339/Downloads/esp-idf-v3.3.1/components/app_update/Kconfig.projbuild C:/Users/16339/Downloads/esp-idf-v3.3.1/components/bootloader/Kconfig.projbuild C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esptool_py/Kconfig.projbuild C:/Users/16339/Downloads/esp-idf-v3.3.1/tcp/main/Kconfig.projbuild C:/Users/16339/Downloads/esp-idf-v3.3.1/components/partition_table/Kconfig.projbuild" --env IDF_CMAKE=y --env IDF_TARGET=esp32 --output config C:/Users/16339/Downloads/esp-idf-v3.3.1/tcp/sdkconfig || exit /b
C:\Users\16339\.espressif\tools\cmake\3.13.4\bin\cmake.exe -E env "COMPONENT_KCONFIGS= C:/Users/16339/Downloads/esp-idf-v3.3.1/components/app_trace/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/aws_iot/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/bt/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/driver/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/efuse/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esp32/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esp_adc_cal/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esp_event/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esp_http_client/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esp_http_server/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esp_https_ota/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/espcoredump/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/ethernet/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/fatfs/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/freemodbus/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/freertos/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/heap/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/libsodium/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/log/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/lwip/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/mbedtls/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/mdns/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/mqtt/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/nvs_flash/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/openssl/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/pthread/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/spi_flash/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/spiffs/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/tcpip_adapter/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/unity/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/vfs/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/wear_levelling/Kconfig C:/Users/16339/Downloads/esp-idf-v3.3.1/components/wifi_provisioning/Kconfig" "COMPONENT_KCONFIGS_PROJBUILD= C:/Users/16339/Downloads/esp-idf-v3.3.1/components/app_update/Kconfig.projbuild C:/Users/16339/Downloads/esp-idf-v3.3.1/components/bootloader/Kconfig.projbuild C:/Users/16339/Downloads/esp-idf-v3.3.1/components/esptool_py/Kconfig.projbuild C:/Users/16339/Downloads/esp-idf-v3.3.1/tcp/main/Kconfig.projbuild C:/Users/16339/Downloads/esp-idf-v3.3.1/components/partition_table/Kconfig.projbuild" IDF_CMAKE=y KCONFIG_CONFIG=C:/Users/16339/Downloads/esp-idf-v3.3.1/tcp/sdkconfig IDF_TARGET=esp32 C:/Users/16339/.espressif/tools/mconf/v4.6.0.0-idf-20190628/mconf-idf.exe C:/Users/16339/Downloads/esp-idf-v3.3.1/Kconfig || exit /b