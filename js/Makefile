MK_SUBDIRS+=class_prototypes builtins $(if $(CONFIG_MODULES),modules)

MK_OBJS=js.o js_builtins.o js_eval.o js_obj.o js_scan.o js_event.o
MK_OBJS+=$(if $(CONFIG_MODULES),js_module.o)
