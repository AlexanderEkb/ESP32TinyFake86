#ifndef SDCARD_H
#define SDCARD_H

#include "Arduino.h"
#include "config/hardware.h"
#include <dirent.h>
#include <driver/sdmmc_host.h>
#include <driver/sdspi_host.h>
#include <driver/spi_master.h>
#include <esp_err.h>
#include <esp_vfs_fat.h>
#include <sys/stat.h>

#define RG_STORAGE_SPEED SDMMC_FREQ_52M     // Used by driver 1 and 2
#define RG_STORAGE_ROOT "/sd"               // Storage mount point
#define RG_STORAGE_FLOPPIES "/sd/PC/Floppies"
#define RG_STORAGE_HDDS "/sd/PC/HDDs"

#define RG_PATH_MAX 255

#ifdef use_lib_log_serial
#define RG_LOGI(...) Serial.printf(__VA_ARGS__)
#define RG_LOGW(...) Serial.printf(__VA_ARGS__)
#define RG_LOGE(...) Serial.printf(__VA_ARGS__)
#else
#define RG_LOGI(...) (void)
#define RG_LOGW(...) (void)
#define RG_LOGE(...) (void)
#endif

enum {
    RG_SCANDIR_STAT = 1, // This will populate file size
    RG_SCANDIR_SORT = 2, // This will sort using natural order
};

typedef struct __attribute__((packed)) {
    char name[75];
} scandir_t;

class SdCard {
  public:
    static const uint32_t DRIVE_COUNT = 5;
    SdCard() {
      disk_mounted = false;
      imgList = nullptr;
      for(uint32_t i=0; i<DRIVE_COUNT; i++)
        drives[i].imgIndex = -1;
    }
    bool Init() {
        if (disk_mounted)
            Deinit();

        int error_code = -1;
#if RG_STORAGE_DRIVER == 0 // Host (stdlib)

        error_code = 0;

#elif RG_STORAGE_DRIVER == 1 // SDSPI

        sdmmc_host_t host_config = SDSPI_HOST_DEFAULT();
        host_config.slot = RG_STORAGE_HOST;
        host_config.max_freq_khz = RG_STORAGE_SPEED;
        host_config.do_transaction = &DoTransaction;
        esp_err_t err;

        // Starting with 4.2.0 we have to initialize the SPI bus ourselves
        // if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 2, 0)
        sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
        slot_config.host_id = RG_STORAGE_HOST;
        slot_config.gpio_cs = SDSPI_CS;
        spi_bus_config_t bus_cfg = {
            .mosi_io_num = SDSPI_MOSI,
            .miso_io_num = SDSPI_MISO,
            .sclk_io_num = SDSPI_CLK,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
        };
        err = spi_bus_initialize(RG_STORAGE_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
        if (err != ESP_OK) // check but do not abort, let esp_vfs_fat_sdspi_mount decide
            RG_LOGE("SPI bus init failed (0x%x)\n", err);

        // sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
        // slot_config.gpio_miso = SDSPI_MISO;
        // slot_config.gpio_mosi = SDSPI_MOSI;
        // slot_config.gpio_sck = SDSPI_CLK;
        // slot_config.gpio_cs = SDSPI_CS;
        // slot_config.dma_channel = 1;
#define esp_vfs_fat_sdspi_mount esp_vfs_fat_sdmmc_mount

        esp_vfs_fat_mount_config_t mount_config;
        mount_config.max_files = 8;

        err = esp_vfs_fat_sdspi_mount(RG_STORAGE_ROOT, &host_config, &slot_config, &mount_config, NULL);
        if (err == ESP_ERR_TIMEOUT || err == ESP_ERR_INVALID_RESPONSE || err == ESP_ERR_INVALID_CRC) {
            RG_LOGW("SD Card mounting failed (0x%x), retrying at lower speed...\n", err);
            host_config.max_freq_khz = SDMMC_FREQ_PROBING;
            err = esp_vfs_fat_sdspi_mount(RG_STORAGE_ROOT, &host_config, &slot_config, &mount_config, NULL);
        }
        error_code = err;

#elif RG_STORAGE_DRIVER == 2 // SDMMC

        sdmmc_host_t host_config = SDMMC_HOST_DEFAULT();
        host_config.flags = SDMMC_HOST_FLAG_1BIT;
        host_config.slot = RG_STORAGE_HOST;
        host_config.max_freq_khz = RG_STORAGE_SPEED;
        host_config.do_transaction = &DoTransaction;

        sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
        slot_config.width = 1;
#if SOC_SDMMC_USE_GPIO_MATRIX
        slot_config.clk = RG_GPIO_SDSPI_CLK;
        slot_config.cmd = RG_GPIO_SDSPI_CMD;
        slot_config.d0 = RG_GPIO_SDSPI_D0;
        // d1 and d3 normally not used in width=1 but sdmmc_host_init_slot saves them, so just in case
        slot_config.d1 = slot_config.d3 = -1;
#endif

        esp_vfs_fat_mount_config_t mount_config = {false, 8};

        esp_err_t err = esp_vfs_fat_sdmmc_mount(RG_STORAGE_ROOT, &host_config, &slot_config, &mount_config, NULL);
        if (err == ESP_ERR_TIMEOUT || err == ESP_ERR_INVALID_RESPONSE || err == ESP_ERR_INVALID_CRC) {
            RG_LOGW("SD Card mounting failed (0x%x), retrying at lower speed...\n", err);
            host_config.max_freq_khz = SDMMC_FREQ_PROBING;
            err = esp_vfs_fat_sdmmc_mount(RG_STORAGE_ROOT, &host_config, &slot_config, &mount_config, NULL);
        }
        error_code = err;

#elif RG_STORAGE_DRIVER == 3 // USB OTG

#warning "USB OTG isn't available on your SOC"
        error_code = -1;

#elif RG_STORAGE_DRIVER == 4 // SPI Flash

        wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
        esp_err_t err = esp_vfs_fat_spiflash_mount(RG_STORAGE_ROOT, "storage", &s_wl_handle);
        error_code = err;

#else

#error "No supported storage driver selected!"

#endif

        if (!error_code) {
          OnMountSuccess();
        } else
            RG_LOGE("Storage mounting failed. driver=%d, err=0x%x\n", RG_STORAGE_DRIVER, error_code);

        disk_mounted = !error_code;
        return disk_mounted;
    }

    scandir_t *getList()
    {
      return imgList;
    }

  private:
    typedef struct DRIVE_DESC
    {
        static const uint32_t MAX_NAME_LENGTH = 256;
        uint32_t    heads;
        uint32_t    cylinders;
        uint32_t    sectors;
        uint32_t    sectorSize;
        FILE *      pImage;
        int32_t     imgIndex;
    };
    DRIVE_DESC drives[DRIVE_COUNT];
    scandir_t * imgList;
    bool disk_mounted;
    void Deinit(void) {
        int error_code = 0;
        if(imgList != nullptr)
          free((void *)imgList);
#if RG_STORAGE_DRIVER == 1 || RG_STORAGE_DRIVER == 2
        esp_err_t err = esp_vfs_fat_sdmmc_unmount();
        error_code = err;
#endif

        if (!error_code)
            RG_LOGI("Storage unmounted.\n");
        else
            RG_LOGE("Storage unmounting failed. err=0x%x\n", error_code);

        disk_mounted = false;
    }

    static esp_err_t DoTransaction(int slot, sdmmc_command_t *cmdinfo) {
#if RG_STORAGE_DRIVER == 1
        // spi_device_acquire_bus(spi_handle, portMAX_DELAY);
        esp_err_t ret = sdspi_host_do_transaction(slot, cmdinfo);
        // spi_device_release_bus(spi_handle);
#else
        esp_err_t ret = sdmmc_host_do_transaction(slot, cmdinfo);
#endif
        return ret;
    }

    scandir_t *scandir()
    {
        RG_LOGI("Scanning...\r\n");
        DIR *dir = opendir(RG_STORAGE_FLOPPIES);
        if (!dir)
            return NULL;
        if (imgList != nullptr)
            free((void *)imgList);
        imgList = (scandir_t *)calloc(1, sizeof(scandir_t));
        size_t capacity = 0;
        size_t count = 0;
        struct dirent *ent;
        struct stat statbuf;

        char fullpath[RG_PATH_MAX + 1] = {0};
        char *basename = fullpath + sprintf(fullpath, "/");
        size_t basename_len = RG_PATH_MAX - (basename - fullpath);

        while ((ent = readdir(dir)))
        {
            strncpy(basename, ent->d_name, basename_len);

            if (basename[0] == '.') // For backwards compat we'll ignore all hidden files...
              continue;

            if (ent->d_type != DT_REG)
              continue;

            uint32_t length = strlen(basename);
            char *extension = &basename[length - 4];
            if ((strncmp(extension, ".img", 4) != 0) &&
                (strncmp(extension, ".IMG", 4) != 0) &&
                (strncmp(extension, ".ima", 4) != 0) &&
                (strncmp(extension, ".IMA", 4) != 0))
              continue;

            if (count + 1 >= capacity)
            {
              capacity += 100;
              void *temp = realloc(imgList, (capacity + 1) * sizeof(scandir_t));
              if (!temp)
              {
                RG_LOGW("Not enough memory to finish scan!\n");
                break;
              }
              imgList = (scandir_t *)temp;
            }

            scandir_t *result = &imgList[count++];
            strncpy(result->name, basename, sizeof(result->name) - 1);
        }
        memset(&imgList[count], 0, sizeof(scandir_t));
        RG_LOGI("%i entries found.\r\n", count);
        closedir(dir);
        return imgList;
    }

    void OnMountSuccess()
    {
      RG_LOGI("Storage mounted at %s. driver=%d\n", RG_STORAGE_ROOT, RG_STORAGE_DRIVER);
      scandir();
    }
};

#endif /* SDCARD_H */