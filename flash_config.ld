SECTIONS
{
    .flash_config (NOLOAD) :
    {
        . = ORIGIN(FLASH) + LENGTH(FLASH) / 2 - 0x1000;

        __flash_config_start__ = .;

        KEEP(*(.flash_config))

        __flash_config_end__ = .;
    } > FLASH
}
