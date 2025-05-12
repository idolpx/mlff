idf_component_register(SRCS "mlff_boot.c"
                       INCLUDE_DIRS "."
                       REQUIRES bootloader bootloader_support esp_app_format nvs_flash spi_flash)

# Use the default linker scripts
idf_build_get_property(scripts BOOTLOADER_LINKER_SCRIPT)
target_linker_script(${COMPONENT_LIB} INTERFACE "${scripts}")