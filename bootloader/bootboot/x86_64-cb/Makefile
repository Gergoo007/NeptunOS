TARGET=../dist/bootbootcb.elf

# get build environment
ifndef LIBCONFIG_PATH
LIBCONFIG_PATH := ../../coreboot/payloads/libpayload
ifeq ($(wildcard $(LIBCONFIG_PATH)/*),)
LIBCONFIG_PATH := ../../../../libpayload
endif
endif
LIBPAYLOAD_DIR=$(CURDIR)/libpayload
XCOMPILE=$(LIBPAYLOAD_DIR)/libpayload.xcompile
# build libpayload and put .config file in $(CURDIR) instead of ../libpayload
# to avoid pollute the libpayload source directory and possible conflicts
LPOPTS=obj="$(CURDIR)/build" DESTDIR="$(CURDIR)" DOTCONFIG="$(CURDIR)/.config"
CFLAGS += -Wall -Wextra -Wvla -Werror -Os -ffreestanding -nostdinc -nostdlib

all: $(TARGET)

$(LIBPAYLOAD_DIR):
	echo $(LIBCONFIG_PATH)
	ls $(LIBCONFIG_PATH)
	@cp lib.config $(LIBCONFIG_PATH)/.config
	$(MAKE) -C $(LIBCONFIG_PATH) $(LPOPTS) oldconfig
	$(MAKE) -C $(LIBCONFIG_PATH) $(LPOPTS)
	$(MAKE) -C $(LIBCONFIG_PATH) $(LPOPTS) install

ifneq ($(strip $(wildcard libpayload)),)
include $(XCOMPILE)
LPGCC = CC="$(GCC_CC_x86_32)" "$(LIBPAYLOAD_DIR)/bin/lpgcc"
$(TARGET):
	$(LPGCC) $(CFLAGS) -o $(TARGET) bootboot.c tinflate.c smp.S
else
# If libpayload is not found, first build libpayload,
# then do the make, this time it'll find libpayload
# and generate the elf target
$(TARGET): $(LIBPAYLOAD_DIR)
	$(MAKE) all
endif

clean:
	rm -f $(TARGET)

distclean: clean
	rm -rf build libpayload .config .config.old

.PHONY: all clean distclean
