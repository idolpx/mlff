// Meatloaf File Flasher (MLFF)
// https://github.com/idolpx/mlff
// Copyright(C) 2025 James Johnston
//
// MLFF is free software : you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// MLFF is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with MLFF. If not, see <http://www.gnu.org/licenses/>.

#include <stdbool.h>
#include <sys/reent.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_log_color.h"
#include "bootloader_init.h"
#include "bootloader_utility.h"
#include "bootloader_common.h"
#include "bootloader_flash_priv.h"
#include "esp_app_desc.h"
#include "nvs_bootloader.h"

static const char* TAG = "MLFF_BOOT";

// Using NVS bootloader to check if update setting
static bool get_nvs_update(void) {
    nvs_bootloader_read_list_t read_list[] = {
        { .namespace_name = "system",  .key_name = "update",  .value_type = NVS_TYPE_U8 }
    };
    size_t read_list_count = sizeof(read_list) / sizeof(read_list[0]);

    // call the read function
    esp_err_t ret = nvs_bootloader_read((const char*)"nvs", read_list_count, read_list);

    // Error code ESP_OK means that the read function was successful and individual, per record results are stored in the read_list
    if (ret == ESP_OK) {
        return read_list[0].value.u8_val == 1;
    }

    return false;
}


/*
 * We arrive here after the ROM bootloader finished loading this second stage bootloader from flash.
 * The hardware is mostly uninitialized, flash cache is down and the app CPU is in reset.
 * We do have a stack, so we can do the initialization in C.
 */
void __attribute__((noreturn)) call_start_cpu0(void)
{
    int select = -1; // Default Factory
    bootloader_state_t bs = {0};

    // 1. Hardware initialization
    if (bootloader_init() != ESP_OK) {
        bootloader_reset();
    }

    if (!get_nvs_update()) {
        ESP_LOGE(TAG, "Booting Main...");
    } else {
        ESP_LOGE(TAG, "Booting Update...");
        select = 0;
    } 

    // 2. Load partition table
    if (!bootloader_utility_load_partition_table(&bs)) {
        ESP_LOGE(TAG, "load partition table error!");
        bootloader_reset();
    }

    // 3. Load the app image for booting
    bootloader_utility_load_boot_image(&bs, select);
}


#if CONFIG_LIBC_NEWLIB
// Return global reent struct if any newlib functions are linked to bootloader
struct _reent *__getreent(void)
{
    return _GLOBAL_REENT;
}
#endif