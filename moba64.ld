OUTPUT_ARCH (mips)

#include "src/sk64/skelatool_defs.h"

#define BEGIN_SEG(name, addr) \
    _##name##SegmentStart = ADDR(.name); \
    _##name##SegmentRomStart = __romPos; \
    .name addr : AT(__romPos)

#define END_SEG(name) \
    _##name##SegmentEnd = ADDR(.name) + SIZEOF(.name); \
    _##name##SegmentRomEnd = __romPos + SIZEOF(.name); \
    __romPos += SIZEOF(.name);

#define BEGIN_NOLOAD(name) \
    _##name##SegmentBssStart = ADDR(.name.noload); \
    .name.noload (NOLOAD) :

#define END_NOLOAD(name) \
    _##name##SegmentBssEnd = ADDR(.name.noload) + SIZEOF(.name.noload); \
    _##name##SegmentBssSize = SIZEOF(.name.noload);

#define DEFINE_LEVEL(humanName, name, theme, maxPlayers, flags) \
   BEGIN_SEG(name, 0x04000000) \
   { \
      build/data/levels/name/level.o(.data); \
      build/data/levels/name/level.o(.bss); \
   } \
   END_SEG(name) \
   BEGIN_SEG(name##_wireframe, 0x07000000) \
   { \
      build/data/levels/name/level.wire.o(.data); \
      build/data/levels/name/level.wire.o(.bss); \
   } \
   END_SEG(name##_wireframe) \

#define DEFINE_THEME(theme) \
   BEGIN_SEG(theme, 0x05000000) \
   { \
      build/data/level_themes/theme/the##me.o(.data); \
      build/data/level_themes/theme/the##me.o(.bss); \
   } \
   END_SEG(theme) \


SECTIONS
{
   __romPos = 0;

   BEGIN_SEG(boot, 0x04000000)
   {
      build/asm/rom_header.o(.text);
      build/boot.6102.o(.data);
   }
   END_SEG(boot)

   BEGIN_SEG(code, 0x80000400) SUBALIGN(16)
   {
      build/asm/entry.o(.text);
      CODE_SEGMENT(.text);
      /usr/lib/n64/PR/rspboot.o(.text);
      /usr/lib/n64/PR/gspF3DEX2.fifo.o(.text);
      /usr/lib/n64/PR/aspMain.o(.text);

      /* data */
      CODE_SEGMENT(.data*);
      /usr/lib/n64/PR/rspboot.o(.data*);
      /usr/lib/n64/PR/gspF3DEX2.fifo.o(.data*);
      /usr/lib/n64/PR/aspMain.o(.data*);

      /* rodata */
      CODE_SEGMENT(.rodata*);
   }
   END_SEG(code)
   BEGIN_NOLOAD(code)
   {
      CODE_SEGMENT(COMMON);
      CODE_SEGMENT(.scommon*);
      CODE_SEGMENT(.bss*);
      . = ALIGN(0x8);
   }
   END_NOLOAD(code)
   
   _codeSegmentBssEnd = .;
   
   _heapStart = .;
   
   BEGIN_SEG(static, 0x01000000)
   {
      build/data/gfxinit.o(.data);
      build/data/gfxinit.o(.bss);
   }
   END_SEG(static)
   
   BEGIN_SEG(gameplaymenu, 0x02000000)
   {
      build/data/gameplaymenu/menu.o(.data);
      build/data/gameplaymenu/menu.o(.bss);
   }
   END_SEG(gameplaymenu)
   
   BEGIN_SEG(mainmenu, 0x02000000)
   {
      build/data/mainmenu/menu.o(.data);
      build/data/mainmenu/menu.o(.bss);
   }
   END_SEG(mainmenu)
   
   BEGIN_SEG(fonts, 0x06000000)
   {
      build/data/fonts/fonts.o(.data);
      build/data/fonts/fonts.o(.bss);
   }
   END_SEG(fonts)

   BEGIN_SEG(characters, 0x03000000)
   {
      build/data/models/characters.o(.data);
      build/data/models/characters.o(.bss);
   }
   END_SEG(characters)

   BEGIN_SEG(level_test, 0x04000000)
   {
      build/data/levels/test/test.o(.data);
      build/data/levels/test/test.o(.bss);
   }
   END_SEG(level_test)

#include "src/levels/level_list.h"
#include "src/levels/theme_list.h"

   BEGIN_SEG(character_animations, CHARACTER_ANIMATION_SEGMENT_ADDRESS)
   {
      build/data/models/character_animations.o(.data);
      build/data/models/character_animations.o(.bss);
   }
   END_SEG(character_animations)

   . = 0x80200000;

   BEGIN_SEG(sound_data, __romPos)
   {
      build/asm/sound_data.o(.data);
      build/asm/sound_data.o(.bss);
   }
   END_SEG(sound_data)

   /* Discard everything not specifically mentioned above. */
   /DISCARD/ :
   {
      *(.eh_frame)
      *(.MIPS.abiflags)
   }
}
