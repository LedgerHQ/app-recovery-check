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

#include "ui.h"

#if defined(TARGET_NANOS) || defined(TARGET_NANOX) || defined(TARGET_NANOS2)

bool get_next_data(bool share_step) {
    if (G_bolos_ux_context.sskr_share_index >= 1 &&
        G_bolos_ux_context.sskr_share_index <= G_bolos_ux_context.sskr_share_count) {
        SPRINTF(G_bolos_ux_context.string_buffer,
                "SSKR Share #%d",
                G_bolos_ux_context.sskr_share_index);
        memcpy(G_bolos_ux_context.words_buffer,
               G_bolos_ux_context.sskr_words_buffer + (G_bolos_ux_context.sskr_share_index - 1) *
                                                          G_bolos_ux_context.sskr_words_buffer_len /
                                                          G_bolos_ux_context.sskr_share_count,
               G_bolos_ux_context.sskr_words_buffer_len / G_bolos_ux_context.sskr_share_count);

        G_bolos_ux_context.sskr_share_index += share_step ? 1 : -1;
        return true;
    }
    if (G_bolos_ux_context.sskr_share_index < 1) {
        G_bolos_ux_context.sskr_share_index = 1;
    }
    if (G_bolos_ux_context.sskr_share_index > G_bolos_ux_context.sskr_share_count) {
        G_bolos_ux_context.sskr_share_index = G_bolos_ux_context.sskr_share_count;
    }

    return false;
}

void bnnn_paging_edgecase(void) {
    G_ux.flow_stack[G_ux.stack_count - 1].prev_index =
        G_ux.flow_stack[G_ux.stack_count - 1].index - 2;
    G_ux.flow_stack[G_ux.stack_count - 1].index--;
    ux_flow_relayout();
}

void sskr_clean_exit(void) {
    memset(G_bolos_ux_context.sskr_words_buffer, 0, G_bolos_ux_context.sskr_words_buffer_len);
    G_bolos_ux_context.sskr_words_buffer_len = 0;
    G_bolos_ux_context.sskr_share_index = 0;
    memset(G_bolos_ux_context.words_buffer, 0, sizeof(G_bolos_ux_context.words_buffer));
    memset(G_bolos_ux_context.string_buffer, 0, sizeof(G_bolos_ux_context.string_buffer));
    os_sched_exit(0);
}

void display_next_state(bool is_upper_delimiter) {
    if (is_upper_delimiter) {  // We're called from the upper delimiter.
        if (G_bolos_ux_context.current_state == STATIC_SCREEN) {
            bool dynamic_data = get_next_data(true);
            if (dynamic_data) {
                G_bolos_ux_context.current_state = DYNAMIC_SCREEN;
            }
            ux_flow_next();
        } else {
            bool dynamic_data = get_next_data(false);
            if (dynamic_data) {
                ux_flow_next();
            } else {
                G_bolos_ux_context.current_state = STATIC_SCREEN;
                ux_flow_prev();
            }
        }
    } else {
        if (G_bolos_ux_context.current_state == STATIC_SCREEN) {
            bool dynamic_data = get_next_data(false);
            if (dynamic_data) {
                // We found some data to display so enter in dynamic mode.
                G_bolos_ux_context.current_state = DYNAMIC_SCREEN;
            }
            ux_flow_prev();
        } else {
            bool dynamic_data = get_next_data(true);
            if (dynamic_data) {
                // Similar to `ux_flow_prev()` but updates layout to account for `bnnn_paging`'s
                // weird behaviour.
                bnnn_paging_edgecase();
            } else {
                G_bolos_ux_context.current_state = STATIC_SCREEN;
                ux_flow_next();
            }
        }
    }
}

UX_STEP_INIT(step_upper_delimiter, NULL, NULL, { display_next_state(true); });

UX_STEP_NOCB(step_display_shares,
             bnnn_paging,
             {
                 .title = G_bolos_ux_context.string_buffer,
                 .text = G_bolos_ux_context.words_buffer,
             });

UX_STEP_INIT(step_lower_delimiter, NULL, NULL, { display_next_state(false); });

UX_STEP_CB(step_clean_exit, pb, sskr_clean_exit(), {&C_icon_dashboard_x, "Quit"});

UX_FLOW(dynamic_flow,
        &step_upper_delimiter,
        &step_display_shares,
        &step_lower_delimiter,
        &step_clean_exit,
        FLOW_LOOP);

void generate_sskr(void) {
#if defined(TARGET_NANOS)
    // Display processing warning to user
    screen_processing_init();
    G_bolos_ux_context.processing = 1;
#endif

    // Set k-of-n threshold
    G_bolos_ux_context.sskr_group_descriptor[0][0] = 2;
    G_bolos_ux_context.sskr_group_descriptor[0][1] = 3;

    G_bolos_ux_context.sskr_share_count = 0;
    G_bolos_ux_context.sskr_words_buffer_len = 0;

    bolos_ux_bip39_to_sskr_convert((unsigned char*) G_bolos_ux_context.words_buffer,
                                   G_bolos_ux_context.words_buffer_length,
                                   G_bolos_ux_context.onboarding_kind,
                                   G_bolos_ux_context.sskr_group_descriptor[0],
                                   &G_bolos_ux_context.sskr_share_count,
                                   (unsigned char*) G_bolos_ux_context.sskr_words_buffer,
                                   &G_bolos_ux_context.sskr_words_buffer_len);
#if defined(TARGET_NANOS)
    G_bolos_ux_context.processing = 0;
#endif

    if (G_bolos_ux_context.sskr_share_count > 0) {
        PRINTF("SSKR share_count from generate_sskr(): \n%d\n",
               G_bolos_ux_context.sskr_share_count);
        for (uint8_t share = 0; share < G_bolos_ux_context.sskr_share_count; share++) {
            PRINTF("SSKR share %d:\n", share + 1);
            PRINTF("%.*s\n",
                   G_bolos_ux_context.sskr_words_buffer_len / G_bolos_ux_context.sskr_share_count,
                   G_bolos_ux_context.sskr_words_buffer +
                       share * G_bolos_ux_context.sskr_words_buffer_len /
                           G_bolos_ux_context.sskr_share_count);
        }
    }
    G_bolos_ux_context.sskr_share_index = 1;
    ux_flow_init(0, dynamic_flow, NULL);
}

#endif
