#include <os.h>
#include "glyphs.h"

#if defined(TARGET_STAX)

#include <nbgl_obj.h>

#define UPPER_MARGIN    4
#define BUTTON_DIAMETER 80
#define ICON_X          0
#define ICON_Y          148

nbgl_image_t *generic_screen_set_icon(const nbgl_icon_details_t *icon) {
    nbgl_image_t *image = (nbgl_image_t *) nbgl_objPoolGet(IMAGE, 0);
    image->foregroundColor = BLACK;
    image->buffer = icon;
    image->bpp = NBGL_BPP_1;
    image->alignmentMarginX = ICON_X;
    image->alignmentMarginY = ICON_Y;
    image->alignment = TOP_MIDDLE;
    image->alignTo = NULL;
    return image;
}

nbgl_text_area_t *generic_screen_set_title(nbgl_obj_t *align_to) {
    nbgl_text_area_t *textArea = (nbgl_text_area_t *) nbgl_objPoolGet(TEXT_AREA, 0);
    textArea->textColor = BLACK;
    textArea->text = "";
    textArea->textAlignment = CENTER;
    textArea->fontId = BAGL_FONT_INTER_MEDIUM_32px;
    textArea->width = SCREEN_WIDTH - 2 * BORDER_MARGIN;
    textArea->height = nbgl_getTextHeight(textArea->fontId, textArea->text);
    textArea->style = NO_STYLE;
    textArea->alignment = BOTTOM_MIDDLE;
    textArea->alignTo = align_to;
    textArea->alignmentMarginX = 0;
    textArea->alignmentMarginY = BORDER_MARGIN;
    return textArea;
}

void generic_screen_configure_buttons(nbgl_button_t **buttons, const size_t size) {
    nbgl_button_t *button;
    for (size_t i = 0; i < size; i++) {
        button = buttons[i];
        button->innerColor = WHITE;
        button->borderColor = LIGHT_GRAY;
        button->foregroundColor = BLACK;
        button->width = SCREEN_WIDTH - 2 * BORDER_MARGIN;
        button->height = BUTTON_DIAMETER;
        button->radius = BUTTON_RADIUS;
        button->fontId = BAGL_FONT_INTER_SEMIBOLD_24px;
        button->icon = NULL;
        button->localized = true;
        button->alignmentMarginX = 0;
        button->alignmentMarginY = (button->height + 8) * i + BORDER_MARGIN;
        button->alignment = BOTTOM_MIDDLE;
        button->alignTo = NULL;
        button->touchMask = (1 << TOUCHED);
    }
}

nbgl_button_t *generic_screen_set_back_button() {
    nbgl_button_t *button = (nbgl_button_t *) nbgl_objPoolGet(BUTTON, 0);
    button->innerColor = WHITE;
    button->borderColor = WHITE;
    button->foregroundColor = BLACK;
    button->width = BUTTON_DIAMETER;
    button->height = BUTTON_DIAMETER;
    button->radius = BUTTON_RADIUS;
    button->text = NULL;
    button->icon = &C_leftArrow32px;
    button->alignmentMarginX = 0;
    button->alignmentMarginY = UPPER_MARGIN;
    button->alignment = TOP_LEFT;
    button->alignTo = NULL;
    button->touchMask = (1 << TOUCHED);
    return button;
}

#endif
