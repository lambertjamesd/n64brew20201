#ifndef _GFX_VALIDATOR_DEFS
#define _GFX_VALIDATOR_DEFS

#define DMA1_LEN(gfx)       _SHIFTR((gfx)->words.w0, 0, 16)
#define DMA1_PARAM(gfx)     _SHIFTR((gfx)->words.w0, 16, 8)

#ifdef F3DEX_GBI_2
#define DMA_MM_LEN(gfx)     ((_SHIFTR((gfx)->words.w0, 19, 5) + 1) * 8)
#define DMA_MM_OFS(gfx)     (_SHIFTR((gfx)->words.w0, 8, 8) * 8)
#define DMA_MM_IDX(gfx)     _SHIFTR((gfx)->words.w0, 0, 8)

#define MOVE_WORD_IDX(gfx)  _SHIFTR((gfx)->words.w0, 16, 8)
#define MOVE_WORD_OFS(gfx)  _SHIFTR((gfx)->words.w0, 0, 16)
#define MOVE_WORD_DATA(gfx) ((gfx)->words.w1)
#else
#define DMA_MM_LEN(gfx)     DMA1_LEN(gfx)
#define DMA_MM_OFS(gfx)     0
#define DMA_MM_IDX(gfx)     DMA1_PARAM(gfx)

#define MOVE_WORD_IDX(gfx)  _SHIFTR((gfx)->words.w0, 0, 8)
#define MOVE_WORD_OFS(gfx)  _SHIFTR((gfx)->words.w0, 8, 16)
#define MOVE_WORD_DATA(gfx) ((gfx)->words.w1)
#endif

#endif