.include "macros.inc"

.section .data

glabel _tableSegmentRomStart
.incbin "build/assets/soundbanks/banks.tbl"
.balign 16
glabel _tableSegmentRomEnd

glabel _bankSegmentRomStart
.incbin "build/assets/soundbanks/banks.ctl"
.balign 16
glabel _bankSegmentRomEnd

glabel _seqSegmentRomStart
.incbin "build/assets/music/multilayer_midi_demo.mid"
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