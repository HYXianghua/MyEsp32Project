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
#include "sdmmc_cmd.h"

#include "tf.h"
static const char *TAG = "[FAT]";
//
void vTfInit(void)
{
  ESP_LOGI(TAG, "Initializing SD card");
  gpio_config_t io_conf;
  io_conf.pin_bit_mask = 1ULL << 32;
  io_conf.mode = GPIO_MODE_OUTPUT;
  gpio_config(&io_conf);
  gpio_set_level(32, 0);
  ESP_LOGI(TAG, "TF Power ON");

  vTaskDelay(100 / portTICK_RATE_MS);

  ESP_LOGI(TAG, "Using SDMMC peripheral");
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  //初始化没有卡检测(CD)和写保护(WP)信号的插槽。
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  //安装文件系统的选项。
  //如果format_if_mount_failed设置为true, SD卡将格式化
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false,
      .max_files = 5,
      .allocation_unit_size = 16 * 1024};
  //使用上面定义的设置初始化SD卡和挂载FAT文件系统。
  sdmmc_card_t *card;
  esp_err_t ret = esp_vfs_fat_sdmmc_mount("/tf", &host, &slot_config, &mount_config, &card);

  if (ret != ESP_OK)
  {
    if (ret == ESP_FAIL)
    {
      ESP_LOGE(TAG, "未格式化!");
    }
    else
    {
      ESP_LOGE(TAG, "挂载失败!(%s). ",
               esp_err_to_name(ret));
    }
    return;
  }

  // Card has been initialized, print its properties
  //SD已初始化，打印其属性
  sdmmc_card_print_info(stdout, card);
}
void vTFExample(void)
{
  // Use POSIX and C standard library functions to work with files.
  // First create a file.
  //使用POSIX和C标准库函数处理文件。
  //首先创建一个文件。
  ESP_LOGI(TAG, "Opening file");
  FILE *f = fopen("/tf/hello.txt", "w");
  if (f == NULL)
  {
    ESP_LOGE(TAG, "Failed to open file for writing");
    return;
  }
  fprintf(f, "Hello TF!\n");
  fclose(f);
  ESP_LOGI(TAG, "File written");

  //重命名前检查目标文件是否存在
  struct stat st;
  if (stat("/tf/foo.txt", &st) == 0)
  {
    // Delete it if it exists
    unlink("/tf/foo.txt");
  }

  //重命名原始文件
  ESP_LOGI(TAG, "Renaming file");
  if (rename("/tf/hello.txt", "/tf/foo.txt") != 0)
  {
    ESP_LOGE(TAG, "Rename failed");
    return;
  }

  //打开重命名文件进行读取
  ESP_LOGI(TAG, "Reading file");
  f = fopen("/tf/foo.txt", "r");
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
