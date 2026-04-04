#
# Makefile for LightWave plugins using m68k-amigaos-gcc
# Run inside Docker: sacredbanana/amiga-compiler:m68k-amigaos
#

CC       = m68k-amigaos-gcc
AS       = m68k-amigaos-as
AR       = m68k-amigaos-ar

SDK_INC  = sdk/include
SDK_SRC  = sdk/source
SDK_LIB  = sdk/lib

SRC      = src
BUILD    = build

CFLAGS   = -noixemul -m68020 -O2 -Wall -I$(SDK_INC)
LDFLAGS  = -noixemul -nostartfiles -m68020
LIBS     = $(SDK_LIB)/server.a -lm -lgcc

STARTUP  = $(SDK_LIB)/serv_gcc.o
STUBS    = $(BUILD)/stubs.o

# ---- SDK library build ----

SLIB_OBJS = $(BUILD)/slib1.o $(BUILD)/slib2.o $(BUILD)/slib3.o $(BUILD)/slib4.o

$(BUILD):
	mkdir -p $(BUILD)

$(SDK_LIB):
	mkdir -p $(SDK_LIB)

$(BUILD)/serv_gcc.o: $(SDK_SRC)/serv_gcc.s | $(BUILD)
	$(AS) -m68020 -o $@ $<

$(BUILD)/slib%.o: $(SDK_SRC)/slib%.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/stubs.o: $(SDK_SRC)/stubs.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(SDK_LIB)/server.a: $(SLIB_OBJS) | $(SDK_LIB)
	$(AR) rcs $@ $^

$(SDK_LIB)/serv_gcc.o: $(BUILD)/serv_gcc.o | $(SDK_LIB)
	cp $< $@

sdk: $(SDK_LIB)/server.a $(SDK_LIB)/serv_gcc.o $(STUBS)

# ---- Plugin build rule ----
# Usage: $(call build-plugin,output.p,source.o [source2.o ...])
define build-plugin
$(CC) $(LDFLAGS) -o $(1) $(STARTUP) $(2) $(STUBS) $(LIBS)
endef

# ---- Plugins ----

# ObjSwap - Object replacement by frame number
$(BUILD)/objswap.o: $(SRC)/objswap/objswap.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/objswap.p: $(BUILD)/objswap.o sdk $(STUBS)
	$(call build-plugin,$@,$<)

objswap: $(BUILD)/objswap.p

# ---- Targets ----

all: sdk objswap

clean:
	rm -f $(BUILD)/*.o $(BUILD)/*.p $(SDK_LIB)/server.a $(SDK_LIB)/serv_gcc.o

.PHONY: all sdk frameswap clean
