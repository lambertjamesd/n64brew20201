.include "macros.inc"

.section .data

glabel _tableSegmentRomStart
.incbin "/usr/lib/n64/soundbanks/GenMidiBank.tbl"
.balign 16
glabel _tableSegmentRomEnd

glabel _bankSegmentRomStart
.incbin "/usr/lib/n64/soundbanks/GenMidiBank.ctl"
.balign 16
glabel _bankSegmentRomEnd

glabel _seqSegmentRomStart
.incbin "build/assets/music/n64_2021_march.mid"
.balign 16
glabel _seqSegmentRomEnd

glabel _soundsSegmentRomStart
.incbin "build/assets/sounds/sounds.sounds"
.balign 16
glabel _soundsSegmentRomEnd

glabel _soundsTblSegmentRomStart
.incbin "build/assets/sounds/sounds.sounds.tbl"
.balign 16
glabel _soundsTblSegmentRomEnd