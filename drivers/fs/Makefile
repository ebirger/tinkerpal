MK_OBJS=vfs.o $(if $(CONFIG_JS),js_fs.o)
MK_DESCS=fs.desc

MK_SUBDIRS+=$(if $(CONFIG_FAT_FS),fat)
MK_SUBDIRS+=$(if $(CONFIG_LOCAL_FS),local_fs)
# Builtin FS must be last since all builtin files must be set
MK_SUBDIRS+=$(if $(CONFIG_BUILTIN_FS),builtin_fs)
