/*******************************************************************************
 *   (c) 2016-2022 Ledger SAS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#pragma once

#include "ux_common/common.h"

#if defined(HAVE_BOLOS_UX) && defined(TARGET_FATSTACKS)

#include <nbgl_obj.h>
#include <nbgl_layout.h>


/**
 * @brief struct to represent a keyboard (@ref KEYBOARD type)
 *
 */
typedef struct PACKED__ keyboard_s {
    struct nbgl_obj_s; ///< common part
    color_t textColor; ///< color set to letters.
    color_t borderColor; ///< color set to key borders
    bool lettersOnly; ///< if true, only display letter keys and Backspace
    bool upperCase; ///< if true, display letter keys in upper case
    keyboardMode_t mode; ///< keyboard mode to start with
    uint32_t keyMask; ///< mask used to disable some keys in letters only mod. The 26 LSB bits of mask are used, for the 26 letters of a QWERTY keyboard. Bit[0] for Q, Bit[1] for W and so on
    keyboardCallback_t callback; ///< function called when an active key is pressed
} keyboard_t;


/**
 * @brief  struct to represent a progress bar (@ref PROGRESS_BAR type)
 * @note if withBorder, the stroke of the border is fixed (3 pixels)
 */
typedef struct PACKED__ progress_bar_s {
    struct nbgl_obj_s; // common part
    bool withBorder; ///< if set to true, a border in black surround the whole object
    uint8_t state; ///< state of the progress, in % (from 0 to 100).
    color_t foregroundColor; ///< color of the inner progress bar and border (if applicable)
} progress_bar_t;

/**
 * @brief This structure contains info to build a progress bar with info
 *
 */
typedef struct {
    uint8_t percentage; ///< percentage of completion, from 0 to 100.
    char *text; ///< text in black, on top of progress bar
    char *subText; ///< text in gray, under progress bar
} layoutKeyboard_t;


int nbgl_layoutAddKeyboard(nbgl_layout_t *layout, nbgl_layoutProgressBar_t *barLayout) {
  nbgl_layoutInternal_t *layoutInt = (nbgl_layoutInternal_t *)layout;
  nbgl_text_area_t *textArea;
  nbgl_text_area_t *subTextArea;
  nbgl_progress_bar_t *progress;

  LOG_DEBUG(LAYOUT_LOGGER,"nbgl_layoutAddProgressBar():\n");
  if (layout == NULL)
    return -1;
  // if (barLayout->text != NULL) {
  //   textArea = (nbgl_text_area_t *)nbgl_objPoolGet(TEXT_AREA,((nbgl_layoutInternal_t *)layout)->layer);
  //   textArea->textColor=BLACK;
  //   textArea->text = PIC(barLayout->text);
  //   textArea->textAlignment = MID_LEFT;
  //   textArea->fontId = BAGL_FONT_INTER_REGULAR_24px;
  //   textArea->width = GET_AVAILABLE_WIDTH(((nbgl_layoutInternal_t *)layout));
  //   textArea->height = 24*nbgl_getTextNbLines(textArea->text);
  //   textArea->style = NO_STYLE;
  //   textArea->alignment = NO_ALIGNMENT;
  //   textArea->alignmentMarginX = BORDER_MARGIN;
  //   textArea->alignmentMarginY = BORDER_MARGIN;
  //   textArea->alignTo = NULL;
  //   addObjectToLayout(layoutInt,(nbgl_obj_t*)textArea);
  // }
  progress = (nbgl_progress_bar_t *)nbgl_objPoolGet(PROGRESS_BAR,((nbgl_layoutInternal_t *)layout)->layer);
  progress->foregroundColor = BLACK;
  progress->withBorder = true;
  progress->state = barLayout->percentage;
  progress->width = GET_AVAILABLE_WIDTH(((nbgl_layoutInternal_t *)layout));
  progress->height = 32;
  progress->alignment = NO_ALIGNMENT;
  progress->alignmentMarginX = BORDER_MARGIN;
  progress->alignmentMarginY = BORDER_MARGIN;
  progress->alignTo = NULL;
  addObjectToLayout(layoutInt,(nbgl_obj_t*)progress);
  // if (barLayout->subText != NULL) {
  //   subTextArea = (nbgl_text_area_t *)nbgl_objPoolGet(TEXT_AREA,((nbgl_layoutInternal_t *)layout)->layer);
  //   subTextArea->textColor=LIGHT_GRAY;
  //   subTextArea->text = PIC(barLayout->subText);
  //   subTextArea->textAlignment = MID_LEFT;
  //   subTextArea->fontId = BAGL_FONT_INTER_REGULAR_24px;
  //   subTextArea->width = GET_AVAILABLE_WIDTH(((nbgl_layoutInternal_t *)layout));
  //   subTextArea->height = 24*nbgl_getTextNbLines(subTextArea->text);
  //   subTextArea->style = NO_STYLE;
  //   subTextArea->alignment = NO_ALIGNMENT;
  //   subTextArea->alignmentMarginX = BORDER_MARGIN;
  //   subTextArea->alignmentMarginY = BORDER_MARGIN;
  //   subTextArea->alignTo = NULL;
  //   addObjectToLayout(layoutInt,(nbgl_obj_t*)subTextArea);
  // }
  return 0;
}


#endif  // HAVE_BOLOS_UX && TARGET_NANOS
