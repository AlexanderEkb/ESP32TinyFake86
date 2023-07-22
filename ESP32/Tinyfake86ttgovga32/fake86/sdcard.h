#ifndef SDCARD_H
#define SDCARD_H

#include <stdint.h>
#include <driver/sdmmc_host.h>
#include <esp_vfs_fat.h>
#include <esp_vfs.h>

#define RG_LOGI(...) Serial.printf(__VA_ARGS__)
#define RG_LOGW(...) Serial.printf(__VA_ARGS__)
#define RG_LOGE(...) Serial.printf(__VA_ARGS__)

// Storage
#define RG_STORAGE_DRIVER 2                 // 0 = Host, 1 = SDSPI, 2 = SDMMC, 3 = USB, 4 = Flash
#define RG_STORAGE_HOST SDMMC_HOST_SLOT_1   // Used by driver 1 and 2
#define RG_STORAGE_SPEED SDMMC_FREQ_DEFAULT
#define RG_STORAGE_ROOT "/sd"               // Storage mount point
#define RG_PATH_MAX 255

typedef struct FileImageInfo {
    uint32_t Index;
    char Name[8];
};

typedef struct __attribute__((packed)) {
    uint32_t is_valid : 1;
    uint32_t is_file : 1;
    uint32_t is_dir : 1;
    uint32_t unused : 5;
    uint32_t size : 24;
    char name[76];
} rg_scandir_t;

enum {
    RG_SCANDIR_STAT = 1, // This will populate file size
    RG_SCANDIR_SORT = 2, // This will sort using natural order
};

class SdCard {
  public:
    SdCard() {disk_mounted = false;};
    void Init()
    {
      if (disk_mounted)
        storage_deinit();

      int error_code = -1;

      sdmmc_host_t host_config = SDMMC_HOST_DEFAULT();
      host_config.flags = SDMMC_HOST_FLAG_1BIT;
      host_config.do_transaction = &sdcard_do_transaction;
#ifdef RG_STORAGE_HOST
      host_config.slot = RG_STORAGE_HOST;
#endif
#ifdef RG_STORAGE_SPEED
      host_config.max_freq_khz = RG_STORAGE_SPEED;
#endif

      sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
      slot_config.width = 1;

      esp_vfs_fat_mount_config_t mount_config = {.max_files = 8};

      esp_err_t err = esp_vfs_fat_sdmmc_mount(RG_STORAGE_ROOT, &host_config, &slot_config, &mount_config, NULL);
      if (err == ESP_ERR_TIMEOUT || err == ESP_ERR_INVALID_RESPONSE || err == ESP_ERR_INVALID_CRC) {
        RG_LOGW("SD Card mounting failed (0x%x), retrying at lower speed...\n", err);
        host_config.max_freq_khz = SDMMC_FREQ_PROBING;
        err = esp_vfs_fat_sdmmc_mount(RG_STORAGE_ROOT, &host_config, &slot_config, &mount_config, NULL);
      }
      error_code = err;

      if (!error_code)
      {
        RG_LOGI("Storage mounted at %s. driver=%d\n", RG_STORAGE_ROOT, RG_STORAGE_DRIVER);
        rg_storage_scandir(RG_STORAGE_ROOT, nullptr, 0);
      } else
        RG_LOGE("Storage mounting failed. driver=%d, err=0x%x\n", RG_STORAGE_DRIVER, error_code);

      disk_mounted = !error_code;
      // disk_led = rg_settings_get_number(NS_GLOBAL, SETTING_DISK_ACTIVITY, 1);
    };
    FileImageInfo * FindFirst();
    FileImageInfo * FindNext();
  private:
    sdmmc_host_t foo;
    bool disk_mounted;
    static esp_err_t sdcard_do_transaction(int slot, sdmmc_command_t *cmdinfo) {
        // bool use_led = (disk_led && !rg_system_get_led());

        // if (use_led)
        //     rg_system_set_led(1);

        esp_err_t ret = sdmmc_host_do_transaction(slot, cmdinfo);

        // if (use_led)
        //     rg_system_set_led(0);

        return ret;
    }
    void storage_deinit(void) {

        int error_code = 0;

        esp_err_t err = esp_vfs_fat_sdmmc_unmount();
        error_code = err;

        if (!error_code)
            RG_LOGI("Storage unmounted.\n");
        else
            RG_LOGE("Storage unmounting failed. err=0x%x\n", error_code);

        disk_mounted = false;
    }

    rg_scandir_t *rg_storage_scandir(const char *path, bool (*validator)(const char *path), uint32_t flags) {
        DIR *dir = opendir(path);
        if (!dir)
            return nullptr;

        rg_scandir_t *results = reinterpret_cast<rg_scandir_t *>(calloc(1, sizeof(rg_scandir_t)));
        size_t capacity = 0;
        size_t count = 0;
        struct dirent *ent;
        struct stat statbuf;

        char fullpath[RG_PATH_MAX + 1] = {0};
        char *basename = fullpath + sprintf(fullpath, "%s/", path);
        size_t basename_len = RG_PATH_MAX - (basename - fullpath);

        while ((ent = readdir(dir))) {
            strncpy(basename, ent->d_name, basename_len);
            Serial.printf("%s\r\n", basename);

            if (basename[0] == '.') // For backwards compat we'll ignore all hidden files...
                continue;

            if (validator && !validator(fullpath))
                continue;

            if (count + 1 >= capacity) {
                capacity += 100;
                void *temp = reinterpret_cast <void *>(realloc(results, (capacity + 1) * sizeof(rg_scandir_t)));
                if (!temp) {
                    RG_LOGW("Not enough memory to finish scan!\n");
                    break;
                }
                results = reinterpret_cast<rg_scandir_t *>(temp);
            }

            rg_scandir_t *result = &results[count++];

            strncpy(result->name, basename, sizeof(result->name) - 1);
            result->is_valid = 1;
            flags |= RG_SCANDIR_STAT;

            if ((flags & RG_SCANDIR_STAT) && stat(fullpath, &statbuf) == 0) {
                result->is_file = S_ISREG(statbuf.st_mode);
                result->is_dir = S_ISDIR(statbuf.st_mode);
                result->size = statbuf.st_size;
            }
        }
        memset(&results[count], 0, sizeof(rg_scandir_t));

        return results;
    }
};

#endif /* SDCARD_H */