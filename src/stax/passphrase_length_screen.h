#pragma once

#if defined(SCREEN_SIZE_WALLET)

nbgl_image_t *passphrase_length_set_icon(void);
nbgl_text_area_t *passphrase_length_set_title(nbgl_obj_t *align_to);
void passphrase_length_configure_buttons(nbgl_button_t **buttons, const size_t size);
nbgl_button_t *passphrase_length_set_back_button();

#endif
