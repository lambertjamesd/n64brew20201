#!smake
# --------------------------------------------------------------------
#        Copyright (C) 1998 Nintendo. (Originated by SGI)
#        
#        $RCSfile: Makefile,v $
#        $Revision: 1.1.1.1 $
#        $Date: 2002/05/02 03:27:21 $
# --------------------------------------------------------------------
include /usr/include/n64/make/PRdefs

MIDICVT:=tools/midicvt
SFZ2N64:=tools/sfz2n64
SKELATOOL64:=tools/skeletool64

OPTIMIZER		:= -O0
LCDEFS			:= -DDEBUG -g -Isrc/ -I/usr/include/n64/nustd -Werror -Wall
N64LIB			:= -lultra_rom -lnustd

ifeq ($(WITH_DEBUGGER),1)
LCDEFS += -DWITH_DEBUGGER
endif

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
	$(CC) $(CFLAGS) -MM $^ -MF "$(@:.o=.d)" -MT"$@"
	$(CC) $(CFLAGS) -c -o $@ $<

build/%.o: %.s
	@mkdir -p $(@D)
	$(AS) -Wa,-Iasm -o $@ $<


####################
# Resources
####################

data/models/minionanimations/geometry.h build/data/models/minionanimations/geometry_anim.inc.h build/data/models/minionanimations/geometry_animdef.inc.h:	assets/models/minionanimations.fbx
	@mkdir -p $(@D)
	$(SKELATOOL64) -a -s 100 -n minion_animations -o data/models/minionanimations/geometry.h assets/models/minionanimations.fbx

data/models/doglow/geometry.h data/models/doglow/geometry_geo.inc.h build/data/models/doglow/geometry_anim.inc.h build/data/models/doglow/geometry_animdef.inc.h:	assets/models/commandermajor.fbx
	@mkdir -p $(@D)
	$(SKELATOOL64) -s 100 -n doglow -r -90,90,0 -o data/models/doglow/geometry.h assets/models/commandermajor.fbx

data/models/catlow/geometry.h data/models/catlow/geometry_geo.inc.h build/data/models/catlow/geometry_anim.inc.h build/data/models/catlow/geometry_animdef.inc.h:	assets/models/Commander_Admiral_Animated.fbx
	@mkdir -p $(@D)
	$(SKELATOOL64) -s 100 -n catlow -r -90,90,0 -o data/models/catlow/geometry.h assets/models/Commander_Admiral_Animated.fbx

data/models/punchtrail/geometry.h build/data/models/punchtrail/geometry_anim.inc.h build/data/models/punchtrail/geometry_animdef.inc.h:	assets/models/punchtrail.fbx
	@mkdir -p $(@D)
	$(SKELATOOL64) -s 100 -n punchtrail -o data/models/punchtrail/geometry.h assets/models/punchtrail.fbx

MUSIC = $(shell find assets/music/ -type f -name '*.mid')

CLEAN_MUSIC = $(patsubst %.mid, build/%.mid, $(MUSIC))

build/assets/music/%.mid: assets/music/%.mid
	@mkdir -p $(@D)
	$(MIDICVT) $< $@
	truncate $@ --size 32KB

RAW_SOUND_CLIPS = $(shell find assets/sounds/ -type f -name '*.aiff') $(shell find assets/sounds/ -type f -name '*.wav')

# INS_CLIPS = $(shell find assets/sounds/ -type f -name '*.ins')
INS_CLIPS =

SOUND_CLIPS = $(filter-out $(patsubst %.ins, %.wav, $(INS_CLIPS)) $(patsubst %.ins, %.aiff, $(INS_CLIPS)), $(RAW_SOUND_CLIPS)) $(INS_CLIPS)

src/audio/clips.h: build_scripts/generate_sound_ids.js $(SOUND_CLIPS)
	node build_scripts/generate_sound_ids.js -o $@ -p SOUNDS_ $(SOUND_CLIPS)

build/assets/sounds/sounds.sounds build/assets/sounds/sounds.sounds.tbl: $(SOUND_CLIPS)
	@mkdir -p $(@D)
	$(SFZ2N64) --compress -o $@ $^
	truncate build/assets/sounds/sounds.sounds --size 32KB
	truncate build/assets/sounds/sounds.sounds.tbl --size 1MB

asm/sound_data.s: build/assets/music/multilayer_midi_demo.mid \
	build/assets/music/n64_2021_march.mid \
	build/assets/sounds/sounds.sounds \
	build/assets/sounds/sounds.sounds.tbl \
	build/assets/soundbanks/banks.ctl \
	build/assets/soundbanks/banks.tbl

build/assets/soundbanks/banks.ctl build/assets/soundbanks.banks.tbl: assets/soundbanks/banks.ins ${SFZ2N64}
	@mkdir -p $(@D)
	${SFZ2N64} --compress -o build/assets/soundbanks/banks.ctl assets/soundbanks/banks.ins
	truncate build/assets/soundbanks/banks.ctl --size 32KB
	truncate build/assets/soundbanks/banks.tbl --size 1MB

LEVELS = $(shell find assets/levels/ -type f -name '*.fbx')

src/levels/level_list.h: assets/levels/level_definition.yaml assets/materials/levels.yaml $(LEVELS)
	${SKELATOOL64} -s 100 -r -90,0,0 -m assets/materials/levels.yaml -d assets/levels/level_definition.yaml

####################
## Linking
####################

$(BOOT_OBJ): $(BOOT)
	$(OBJCOPY) -I binary -B mips -O elf32-bigmips $< $@

# without debugger

CODEOBJECTS_NO_DEBUG = $(CODEOBJECTS)

ifeq ($(WITH_DEBUGGER),1)
CODEOBJECTS_NO_DEBUG += build/debugger/debugger_stub.o
endif

$(CODESEGMENT)_no_debug.o:	$(CODEOBJECTS_NO_DEBUG)
	$(LD) -o $(CODESEGMENT)_no_debug.o -r $(CODEOBJECTS_NO_DEBUG) $(LDFLAGS)


$(CP_LD_SCRIPT)_no_debug.ld: $(LD_SCRIPT)
	cpp -P -Wno-trigraphs $(LCDEFS) -DCODE_SEGMENT=$(CODESEGMENT)_no_debug.o -o $@ $<

$(BASE_TARGET_NAME).z64: $(CODESEGMENT)_no_debug.o $(OBJECTS) $(CP_LD_SCRIPT)_no_debug.ld
	$(LD) -L. -T $(CP_LD_SCRIPT)_no_debug.ld -Map $(BASE_TARGET_NAME)_no_debug.map -o $(BASE_TARGET_NAME).elf
	$(OBJCOPY) --pad-to=0x100000 --gap-fill=0xFF $(BASE_TARGET_NAME).elf $(BASE_TARGET_NAME).z64 -O binary
	makemask $(BASE_TARGET_NAME).z64

# with debugger
CODEOBJECTS_DEBUG = $(CODEOBJECTS) 

ifeq ($(WITH_DEBUGGER),1)
CODEOBJECTS_DEBUG += build/debugger/debugger.o build/debugger/serial.o 
endif

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

################
# Patch info
################

build/musicpatch.zip: $(BASE_TARGET_NAME).z64 \
	$(CLEAN_MUSIC) \
	build/assets/soundbanks/banks.ctl \
	build/assets/soundbanks.banks.tbl \
	build/assets/sounds/sounds.sounds \
	build/assets/sounds/sounds.sounds.tbl

	@mkdir -p build/musicpatch

	tools/createpatch build/musicpatch/patch $(BASE_TARGET_NAME).z64 \
		$(CLEAN_MUSIC) \
		build/assets/soundbanks/banks.ctl \
		build/assets/soundbanks/banks.tbl \
		build/assets/sounds/sounds.sounds \
		build/assets/sounds/sounds.sounds.tbl

	node build_scripts/generate_patch_script.js -m patch $(MUSIC) -i patch/banks.ctl assets/soundbanks/banks.ins -s patch/sounds.sounds $(SOUND_CLIPS) > build/musicpatch/apply.bat

	@mkdir -p build/musicpatch/assets
	cp -r assets/music build/musicpatch/assets
	cp -r assets/soundbanks build/musicpatch/assets
	cp -r assets/sounds build/musicpatch/assets

	# @mkdir -p build/musicpatch/tools
	# cp -r tools/applypatch.exe build/musicpatch/tools
	# cp -r tools/midicvt.exe build/musicpatch/tools
	# cp -r tools/sfz2n64.exe build/musicpatch/tools
	# cp -r tools/makemask.exe build/musicpatch/tools

	zip -r build/musicpatch.zip build/musicpatch/