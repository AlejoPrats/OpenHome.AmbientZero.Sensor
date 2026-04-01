SECTIONS
{
    .flash_config (NOLOAD) :
    {
        __flash_config_start__ = .;
        KEEP(*(.flash_config))
        __flash_config_end__ = .;
    }
}