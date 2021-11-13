#include "textbox.h"

#include <string.h>
#include "kickflipfont.h"
#include "graphics/spritefont.h"
#include "graphics/gfx.h"
#include "math/mathf.h"
#include "util/time.h"

struct Coloru8 gMenuBlue = {127, 174, 177, 255};
struct Coloru8 gMenuBlack = {41, 41, 35, 255};

struct TextBox gTextBox;

#define BOX_HEIGHT      36
#define TOP_PADDING     2
#define SIDE_PADDING    7
#define SLIDE_DURATION  0.5f


void textBoxInitEmpty(struct TextBox* textBox) {
    textBox->currState = TextBoxStateHidden;
    textBox->nextState = TextBoxStateHidden;
}

void textBoxInit(struct TextBox* textBox, char* message, int minWidth, int xCenter, int yCenter) {
    textBox->currState = TextBoxStateHidden;
    textBox->nextState = TextBoxStateShowing;
    textBox->animateTimer = SLIDE_DURATION;
    textBox->width = minWidth;
    textBox->xCenter = xCenter;
    textBox->yPos = yCenter - BOX_HEIGHT / 2;
    textBox->iconRenderer = 0;
    textBox->iconRendererData = 0;
    textBoxChangeText(textBox, message);
}

void textBoxSetIcon(struct TextBox* textBox, IconRenderer iconRenderer, void* data) {
    textBox->iconRenderer = iconRenderer;
    textBox->iconRendererData = data;
}

void textBoxUpdate(struct TextBox* textBox) {
    if (textBoxIsVisible(textBox)) {
        if (textBox->currState != textBox->nextState) {
            float targetTime = (textBox->nextState == TextBoxStateHidden) ? SLIDE_DURATION : 0.0f;
            textBox->animateTimer = mathfMoveTowards(textBox->animateTimer, targetTime, gTimeDelta);

            if (textBox->animateTimer == targetTime) {
                textBox->currState = textBox->nextState;
            }
        }
    }
}

void textBoxRender(struct TextBox* textBox, struct RenderState* renderState) {
    if (textBoxIsVisible(textBox)) {
        
        float timeSqr = textBox->animateTimer * textBox->animateTimer / (SLIDE_DURATION * SLIDE_DURATION);
        unsigned bgRectPos = textBox->xCenter - (textBox->width >> 1) - (int)(timeSqr * SCREEN_WD);
        unsigned fbRectPos = textBox->xCenter - (textBox->width >> 1) + (int)(timeSqr * SCREEN_WD);
        unsigned textLen = strlen(textBox->message);
        unsigned actualLen = (unsigned)(textLen * (1.0f - textBox->animateTimer / SLIDE_DURATION));
        char subMessage[MAX_TEXBOX_LENGTH];
        strncpy(subMessage, textBox->message, actualLen);
        unsigned textWidth = fontMeasure(&gKickflipFont, subMessage, 0);

        if (textBox->iconRenderer) {
            textWidth += textBox->iconRenderer(0, textBox->iconRendererData, 0, 0);
        }

        unsigned textPos = textBox->xCenter - (textWidth >> 1);

        spriteSetColor(renderState, LAYER_SOLID_COLOR, gMenuBlue);
        spriteSolid(renderState, LAYER_SOLID_COLOR, bgRectPos, textBox->yPos, textBox->width, BOX_HEIGHT);
        spriteSetColor(renderState, LAYER_SOLID_COLOR, gMenuBlack);
        spriteSolid(renderState, LAYER_SOLID_COLOR, fbRectPos + SIDE_PADDING, textBox->yPos + TOP_PADDING, textBox->width - SIDE_PADDING * 2, BOX_HEIGHT - TOP_PADDING);

        int textY = textBox->yPos + ((BOX_HEIGHT - 16) >> 1);

        if (textBox->iconRenderer) {
            textPos += textBox->iconRenderer(renderState, textBox->iconRendererData, textPos, textY);
        }

        fontRenderText(renderState, &gKickflipFont, subMessage, textPos, textY, 0);
    }
}

void textBoxHide(struct TextBox* textBox) {
    textBox->nextState = TextBoxStateHidden;
}

void textBoxChangeText(struct TextBox* textBox, char* message) {
    strncpy(textBox->message, message, MAX_TEXBOX_LENGTH-1);
    int textSize = fontMeasure(&gKickflipFont, textBox->message, 0) + SIDE_PADDING * 2;
    if (textBox->width < textSize) {
        textBox->width = textSize;
    }
}

int textBoxIsVisible(struct TextBox* textBox) {
    return textBox->currState != TextBoxStateHidden || textBox->nextState != TextBoxStateHidden;
}