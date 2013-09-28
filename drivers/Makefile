MK_SUBDIRS+=$(if $(CONFIG_ONE_WIRE),one_wire)
MK_SUBDIRS+=$(if $(CONFIG_MMC),mmc)
MK_SUBDIRS+=$(if $(CONFIG_GPIO),gpio)
MK_SUBDIRS+=$(if $(CONFIG_LCD),lcd)
MK_SUBDIRS+=$(if $(CONFIG_PLAT_HAS_SERIAL),serial)
MK_SUBDIRS+=$(if $(CONFIG_SPI),spi)
MK_SUBDIRS+=$(if $(CONFIG_GRAPHICS),graphics)
# fs must be last since builtin fs aggregates files from other drivers
MK_SUBDIRS+=$(if $(CONFIG_VFS),fs)
