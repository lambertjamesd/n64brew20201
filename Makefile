#!smake
# --------------------------------------------------------------------
#        Copyright (C) 1998 Nintendo. (Originated by SGI)
#        
#        $RCSfile: Makefile,v $
#        $Revision: 1.1.1.1 $
#        $Date: 2002/05/02 03:27:21 $
# --------------------------------------------------------------------
include /usr/include/n64/make/PRdefs

MIDICVT:=/home/james/go/src/github.com/lambertjamesd/midicvt/midicvt

OPTIMIZER		:= -O0
LCDEFS			:= -DDEBUG -g -Isrc/ -I/usr/include/n64/nustd -Werror
N64LIB			:= -lultra_rom -lnustd

BASE_TARGET_NAME = build/moba64

LD_SCRIPT	= moba64.ld
CP_LD_SCRIPT	= build/moba64

ASMFILES    =	$(shell find asm/ -type f -name '*.s')

ASMOBJECTS  =	$(patsubst %.s, build/%.o, $(ASMFILES))

CODEFILES = $(shell find src/ -type f -name '*.c')

CODEOBJECTS = $(patsubst %.c, build/%.o, $(CODEFILES))

CODESEGMENT =	build/codesegment

DATAFILES = $(shell find data/ -type f -name '*.c')

DATAOBJECTS =	$(patsubst %.c, build/%.o, $(DATAFILES))

BOOT		=	/usr/lib/n64/PR/bootcode/boot.6102
BOOT_OBJ	=	build/boot.6102.o

OBJECTS		=	$(DATAOBJECTS) $(ASMOBJECTS) $(BOOT_OBJ)

DEPS = $(patsubst %.c, build/%.d, $(CODEFILES)) $(patsubst %.c, build/%.d, $(DATAFILES))

-include $(DEPS)

LCINCS =	-I/usr/include/n64/PR 
LCDEFS +=	-DF3DEX_GBI_2
#LCDEFS +=	-DF3DEX_GBI_2 -DFOG
#LCDEFS +=	-DF3DEX_GBI_2 -DFOG -DXBUS
#LCDEFS +=	-DF3DEX_GBI_2 -DFOG -DXBUS -DSTOP_AUDIO

LDIRT  =	$(BASE_TARGET_NAME).elf $(CP_LD_SCRIPT) $(BASE_TARGET_NAME).z64 $(BASE_TARGET_NAME)_no_debug.map $(ASMOBJECTS)

LDFLAGS =	-L/usr/lib/n64 $(N64LIB)  -L$(N64_LIBGCCDIR) -lgcc

default:	$(BASE_TARGET_NAME).z64

include $(COMMONRULES)

.s.o:
	$(AS) -Wa,-Iasm -o $@ $<

build/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(GCINCS) $(LCINCS) -MF"$(@:.o=.d)" -MG -MM -MP -MT"$@" "$<"
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.o: %.s
	@mkdir -p $(@D)
	$(AS) -Wa,-Iasm -o $@ $<


####################
# Resources
####################

data/models/minionanimations/geometry.h build/data/models/minionanimations/geometry_anim.inc.h build/data/models/minionanimations/geometry_animdef.inc.h:	assets/models/minionanimations.fbx
	@mkdir -p $(@D)
	skeletool64 -a -s 100 -n minion_animations -o data/models/minionanimations/geometry.h assets/models/minionanimations.fbx

data/models/doglow/geometry.h build/data/models/doglow/geometry_anim.inc.h build/data/models/doglow/geometry_animdef.inc.h:	assets/models/commandermajor.fbx
	@mkdir -p $(@D)
	skeletool64 -s 100 -n doglow -o data/models/doglow/geometry.h assets/models/commandermajor.fbx

MUSIC = $(shell find assets/music/ -type -f -name '*.mid')

build/assets/music/%.mid: assets/music/%.mid
	@mkdir -p $(@D)
	$(MIDICVT) $< $@

asm/sound_data.s: build/assets/music/battle.mid

####################
## Linking
####################

$(BOOT_OBJ): $(BOOT)
	$(OBJCOPY) -I binary -B mips -O elf32-bigmips $< $@

# without debugger
CODEOBJECTS_NO_DEBUG = $(CODEOBJECTS) build/debugger/debugger_stub.o

CODEOBJECTS_DEBUG = $(CODEOBJECTS) build/debugger/debugger.o build/debugger/serial.o 

$(CODESEGMENT)_no_debug.o:	$(CODEOBJECTS_NO_DEBUG)
	$(LD) -o $(CODESEGMENT)_no_debug.o -r $(CODEOBJECTS_NO_DEBUG) $(LDFLAGS)


$(CP_LD_SCRIPT)_no_debug.ld: $(LD_SCRIPT)
	cpp -P -Wno-trigraphs $(LCDEFS) -DCODE_SEGMENT=$(CODESEGMENT)_no_debug.o -o $@ $<

$(BASE_TARGET_NAME).z64: $(CODESEGMENT)_no_debug.o $(OBJECTS) $(CP_LD_SCRIPT)_no_debug.ld
	$(LD) -L. -T $(CP_LD_SCRIPT)_no_debug.ld -Map $(BASE_TARGET_NAME)_no_debug.map -o $(BASE_TARGET_NAME).elf
	$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $(BASE_TARGET_NAME).elf $(BASE_TARGET_NAME).z64 -O binary
	makemask $(BASE_TARGET_NAME).z64

# with debugger
CODEOBJECTS_DEBUG = $(CODEOBJECTS) build/debugger/debugger.o build/debugger/serial.o 

$(CODESEGMENT)_debug.o:	$(CODEOBJECTS_DEBUG)
	$(LD) -o $(CODESEGMENT)_debug.o -r $(CODEOBJECTS_DEBUG) $(LDFLAGS)

$(CP_LD_SCRIPT)_debug.ld: $(LD_SCRIPT)
	cpp -P -Wno-trigraphs $(LCDEFS) -DCODE_SEGMENT=$(CODESEGMENT)_debug.o -o $@ $<

$(BASE_TARGET_NAME)_debug.z64: $(CODESEGMENT)_debug.o $(OBJECTS) $(CP_LD_SCRIPT)_debug.ld
	$(LD) -L. -T $(CP_LD_SCRIPT)_debug.ld -Map $(BASE_TARGET_NAME)_debug.map -o $(BASE_TARGET_NAME)_debug.elf
	$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $(BASE_TARGET_NAME)_debug.elf $(BASE_TARGET_NAME)_debug.z64 -O binary
	makemask $(BASE_TARGET_NAME)_debug.z64

clean:
	rm -rf build