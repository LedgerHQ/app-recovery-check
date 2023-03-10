#include <os.h>
#include "glyphs.h"

#if defined(TARGET_STAX)

#include <nbgl_obj.h>

#define UPPER_MARGIN    4
#define BUTTON_DIAMETER 80

nbgl_image_t *passphrase_length_set_icon() {
    nbgl_image_t *image = (nbgl_image_t *) nbgl_objPoolGet(IMAGE, 0);
    image->foregroundColor = BLACK;
    image->buffer = &C_stax_recovery_64px;
    image->bpp = NBGL_BPP_1;
    image->alignmentMarginX = 0;
    image->alignmentMarginY = 148;
    image->alignment = TOP_MIDDLE;
    image->alignTo = NULL;
    return image;
}

nbgl_text_area_t *passphrase_length_set_title(nbgl_obj_t *align_to) {
    nbgl_text_area_t *textArea = (nbgl_text_area_t *) nbgl_objPoolGet(TEXT_AREA, 0);
    textArea->textColor = BLACK;
    textArea->text = "How long is your\nRecovery Phrase?";
    textArea->textAlignment = CENTER;
    textArea->fontId = BAGL_FONT_INTER_REGULAR_32px;
    textArea->width = SCREEN_WIDTH - 2 * BORDER_MARGIN;
    textArea->height = nbgl_getTextHeight(textArea->fontId, textArea->text);
    textArea->style = NO_STYLE;
    textArea->alignment = BOTTOM_MIDDLE;
    textArea->alignTo = align_to;
    textArea->alignmentMarginX = 0;
    textArea->alignmentMarginY = BORDER_MARGIN;
    return textArea;
}

void passphrase_length_configure_buttons(nbgl_button_t **buttons,
                                         const size_t size,
                                         nbgl_touchCallback_t callback) {
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
        button->touchCallback = callback;
    }
}

nbgl_button_t *passphrase_length_set_back_button(nbgl_touchCallback_t callback) {
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
    button->touchCallback = callback;
    return button;
}

#endif
