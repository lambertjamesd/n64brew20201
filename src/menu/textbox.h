#ifndef _MENU_TEXTBOX_H
#define _MENU_TEXTBOX_H

#include "graphics/render_state.h"
#include "math/color.h"

extern struct Coloru8 gMenuBlue;
extern struct Coloru8 gMenuBlack;

#define MAX_TEXBOX_LENGTH 20

enum TextBoxState {
    TextBoxStateHidden,
    TextBoxStateShowing,
};

typedef int (*IconRenderer)(struct RenderState* renderState, void* data, int x, int y);

struct TextBox {
    short xCenter;
    short yPos;
    short width;
    char currState;
    char nextState;
    float animateTimer;
    char message[MAX_TEXBOX_LENGTH];
    IconRenderer iconRenderer;
    void* iconRendererData;
};

extern struct TextBox gTextBox;

void textBoxInitEmpty(struct TextBox* textBox);
void textBoxInit(struct TextBox* textBox, char* message, int minWidth, int xCenter, int yCenter);
void textBoxSetIcon(struct TextBox* textBox, IconRenderer iconRenderer, void* data);
void textBoxUpdate(struct TextBox* textBox);
void textBoxRender(struct TextBox* textBox, struct RenderState* renderState);
void textBoxHide(struct TextBox* textBox);
void textBoxChangeText(struct TextBox* textBox, char* message);
int textBoxIsVisible(struct TextBox* textBox);

#endif