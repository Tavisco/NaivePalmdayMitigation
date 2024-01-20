TOOLCHAIN		?=	/home/tavisco/palm/palmdev_V3/buildtools/toolchain/bin
SDK				?=	/home/tavisco/palm/palmdev_V3/buildtools/palm-os-sdk-master/sdk-5r3/include
PILRC			?=	/home/tavisco/palm/palmdev_V3/buildtools/pilrc3_3_unofficial/bin/pilrc
CC				=	$(TOOLCHAIN)/m68k-none-elf-gcc
LD				=	$(TOOLCHAIN)/m68k-none-elf-gcc
OBJCOPY			=	$(TOOLCHAIN)/m68k-none-elf-objcopy
COMMON			=	-Wno-multichar -funsafe-math-optimizations -Os -m68000 -mno-align-int -mpcrel -fpic -fshort-enums -mshort
WARN			=	-Wsign-compare -Wextra -Wall -Werror -Wno-unused-parameter -Wno-old-style-declaration -Wno-unused-function -Wno-unused-variable -Wno-error=cpp -Wno-error=switch -Wno-error=maybe-uninitialized
LKR				=	linker.lkr
CCFLAGS			=	$(LTO) $(WARN) $(COMMON) -I. -ffunction-sections -fdata-sections
LDFLAGS			=	$(LTO) $(WARN) $(COMMON) -Wl,--gc-sections -Wl,-T $(LKR)
SRCS			=	MyDateTemplateToAscii.c MySelectDay.c
RCP				=	NaivePalmdayMitigation.rcp
RSC				=	/
OBJS			=	$(SRCS:.c=.o)
TARGET			=	NaivePalmdayMitigation
CREATOR			=	NPDP
TYPE			=	HACK

#add PalmOS SDK
INCS			+=	-isystem$(SDK)
INCS			+=	-isystem$(SDK)/Core
INCS			+=	-isystem$(SDK)/Core/Hardware
INCS			+=	-isystem$(SDK)/Core/System
INCS			+=	-isystem$(SDK)/Core/UI
INCS			+=	-isystem$(SDK)/Dynamic
INCS			+=	-isystem$(SDK)/Libraries
INCS			+=	-isystem$(SDK)/Libraries/PalmOSGlue

$(TARGET).prc: MyDateTemplateToAscii.bin MySelectDay.bin
	@echo "  --  Generating PRC"
	$(PILRC) -ro -o $(TARGET).prc -creator $(CREATOR) -type $(TYPE) -name $(TARGET) -I $(RSC) $(RCP) 

%.o: %.c Makefile
	@echo "  --  Compiling $<..."
	$(CC) $(CCFLAGS) $(INCS) -c $< -o $@

%.elf: %.o
	@echo "  --  Generating ELF for $<..."
	$(LD) -o $@ $(LDFLAGS) $<

%.bin: %.elf
	@echo "  --  Gennerating BIN for $<..."
	$(OBJCOPY) -O binary $< $@ -j.vec -j.text -j.rodata

clean:
	rm -rf *.o *.elf *.bin *.prc
 
.PHONY: clean
