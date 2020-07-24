/* SD card and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

static const char *TAG = "example";

// This example can use SDMMC and SPI peripherals to communicate with SD card.
// By default, SDMMC peripheral is used.
// To enable SPI mode, uncomment the following line:

// #define USE_SPI_MODE

// When testing SD and SPI modes, keep in mind that once the card has been
// initialized in SPI mode, it can not be reinitialized in SD mode without
// toggling power to the card.

#ifdef USE_SPI_MODE
// Pin mapping when using SPI mode.
// With this mapping, SD card can be used both in SPI and 1-line SD mode.
// Note that a pull-up on CS line is required in SD mode.
#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 15
#define PIN_NUM_CLK 14
#define PIN_NUM_CS 13
#endif //USE_SPI_MODE

void app_main(void)
{
  ESP_LOGI(TAG, "Initializing SD card");
  gpio_config_t io_conf;
  io_conf.pin_bit_mask = 1ULL << 32;
  io_conf.mode = GPIO_MODE_OUTPUT;
  gpio_config(&io_conf);
  gpio_set_level(32, 0);
  printf("TF_Pw:On\n");

  vTaskDelay(100 / portTICK_RATE_MS);
#ifndef USE_SPI_MODE
  ESP_LOGI(TAG, "Using SDMMC peripheral");
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();

  // This initializes the slot without card detect (CD) and write protect (WP) signals.
  // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

  // To use 1-line SD mode, uncomment the following line:
  // slot_config.width = 1;

#else
  ESP_LOGI(TAG, "Using SPI peripheral");

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
  slot_config.gpio_miso = PIN_NUM_MISO;
  slot_config.gpio_mosi = PIN_NUM_MOSI;
  slot_config.gpio_sck = PIN_NUM_CLK;
  slot_config.gpio_cs = PIN_NUM_CS;
  // This initializes the slot without card detect (CD) and write protect (WP) signals.
  // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
#endif //USE_SPI_MODE

  // Options for mounting the filesystem.
  // If format_if_mount_failed is set to true, SD card will be partitioned and
  // formatted in case when mounting fails.
  //安装文件系统的选项。
  //如果format_if_mount_failed设置为true, SD卡将被分区
  //格式化，以防挂载失败。
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = true,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024};

  // Use settings defined above to initialize SD card and mount FAT filesystem.
  // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
  // Please check its source code and implement error recovery when developing
  // production applications.
  //使用上面定义的设置初始化SD卡和挂载FAT文件系统。
  //注意:esp_vfs_fat_sdmmc_mount是一个集功能于一身的便利函数。
  //开发时请检查其源代码并实现错误恢复
  // 生产应用程序。
  sdmmc_card_t *card;
  esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

  if (ret != ESP_OK)
  {
    if (ret == ESP_FAIL)
    {
      ESP_LOGE(TAG, "Failed to mount filesystem. "
                    "If you want the card to be formatted, set format_if_mount_failed = true.");
    }
    else
    {
      ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                    "Make sure SD card lines have pull-up resistors in place.",
               esp_err_to_name(ret));
    }
    return;
  }

  // Card has been initialized, print its properties
  //SD已初始化，打印其属性
  sdmmc_card_print_info(stdout, card);

  // Use POSIX and C standard library functions to work with files.
  // First create a file.
  //使用POSIX和C标准库函数处理文件。
  //首先创建一个文件。
  ESP_LOGI(TAG, "Opening file");
  FILE *f = fopen("/sdcard/hello.txt", "w");
  if (f == NULL)
  {
    ESP_LOGE(TAG, "Failed to open file for writing");
    return;
  }
  fprintf(f, "Hello %s!\n", card->cid.name);
  fclose(f);
  ESP_LOGI(TAG, "File written");

  // Check if destination file exists before renaming
  //重命名前检查目标文件是否存在
  struct stat st;
  if (stat("/sdcard/foo.txt", &st) == 0)
  {
    // Delete it if it exists
    unlink("/sdcard/foo.txt");
  }

  // Rename original file
  //重命名原始文件
  ESP_LOGI(TAG, "Renaming file");
  if (rename("/sdcard/hello.txt", "/sdcard/foo.txt") != 0)
  {
    ESP_LOGE(TAG, "Rename failed");
    return;
  }

  // Open renamed file for reading
  //打开重命名文件进行读取
  ESP_LOGI(TAG, "Reading file");
  f = fopen("/sdcard/foo.txt", "r");
  if (f == NULL)
  {
    ESP_LOGE(TAG, "Failed to open file for reading");
    return;
  }
  char line[64];
  fgets(line, sizeof(line), f);
  fclose(f);
  // strip newline
  //带换行符
  char *pos = strchr(line, '\n');
  if (pos)
  {
    *pos = '\0';
  }
  ESP_LOGI(TAG, "Read from file: '%s'", line);

  // All done, unmount partition and disable SDMMC or SPI peripheral
  //全部完成，卸载分区并禁用SDMMC或SPI外围设备
  esp_vfs_fat_sdmmc_unmount();
  ESP_LOGI(TAG, "Card unmounted");
}
