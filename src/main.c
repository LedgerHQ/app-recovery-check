/*******************************************************************************
*   Ledger Blue
*   (c) 2016 Ledger
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

#include "os.h"
#include "cx.h"

#include "os_io_seproxyhal.h"

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

static unsigned int current_text_pos; // parsing cursor in the text to display
static unsigned int text_y;           // current location of the displayed text

// UI currently displayed
extern enum UI_STATE { UI_IDLE, UI_TEXT, UI_APPROVAL };

extern enum UI_STATE uiState;

ux_state_t ux;

static unsigned char display_text_part(void);

#define MAX_CHARS_PER_LINE 49

static char lineBuffer[50];

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
    case CHANNEL_KEYBOARD:
        break;

    // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
    case CHANNEL_SPI:
        if (tx_len) {
            io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

            if (channel & IO_RESET_AFTER_REPLIED) {
                reset();
            }
            return 0; // nothing received from the master so far (it's a tx
                      // transaction)
        } else {
            return io_seproxyhal_spi_recv(G_io_apdu_buffer,
                                          sizeof(G_io_apdu_buffer), 0);
        }

    default:
        THROW(INVALID_PARAMETER);
    }
    return 0;
}

static void sample_main(void) {

    // next timer callback in 500 ms
    //UX_CALLBACK_SET_INTERVAL(500);

    uint8_t flags;

    for (;;) {
        io_exchange(CHANNEL_APDU | flags, 0);

        flags |= IO_ASYNCH_REPLY;
    }

return_to_dashboard:
    return;
}


// Pick the text elements to display
static unsigned char display_text_part() {
    unsigned int i;
    WIDE char *text = (char*) G_io_apdu_buffer + 5;
    if (text[current_text_pos] == '\0') {
        return 0;
    }
    i = 0;
    while ((text[current_text_pos] != 0) && (text[current_text_pos] != '\n') &&
           (i < MAX_CHARS_PER_LINE)) {
        lineBuffer[i++] = text[current_text_pos];
        current_text_pos++;
    }
    if (text[current_text_pos] == '\n') {
        current_text_pos++;
    }
    lineBuffer[i] = '\0';

    return 1;
}

#include "bolos_ux.h"

unsigned char io_event(unsigned char channel) {
    // nothing done with the event, throw an error on the transport layer if
    // needed

    // can't have more than one tag in the reply, not supported yet.
    switch (G_io_seproxyhal_spi_buffer[0]) {
    case SEPROXYHAL_TAG_FINGER_EVENT:
        UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
        UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
        break;

    case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
        if ((uiState == UI_TEXT) &&
            (os_seph_features() & SEPROXYHAL_TAG_SESSION_START_EVENT_FEATURE_SCREEN_BIG)) {
                UX_REDISPLAY();
            }
        else {
            if(G_bolos_ux_context.processing == 1)
            {
                UX_DISPLAYED_EVENT(foo(););
            }
            else
            {
                UX_DISPLAYED_EVENT();
            }
        }
        break;

    case SEPROXYHAL_TAG_TICKER_EVENT:
        #ifdef TARGET_NANOS
            UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {
                // defaulty retrig very soon (will be overriden during
                // stepper_prepro)
                UX_CALLBACK_SET_INTERVAL(500);
                UX_REDISPLAY();
            });
        #endif 
        break;

    // unknown events are acknowledged
    default:
        UX_DEFAULT_EVENT();
        break;
    }

    // close the event if not done previously (by a display or whatever)
    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }

    // command has been processed, DO NOT reset the current APDU transport
    return 1;
}

__attribute__((section(".boot"))) int main(void) {
    // exit critical section
    __asm volatile("cpsie i");

    current_text_pos = 0;
    text_y = 60;
    uiState = UI_IDLE;

    // ensure exception will work as planned
    os_boot();

    UX_INIT();

    BEGIN_TRY {
        TRY {
            io_seproxyhal_init();
            //ui_idle();
            ui_idle_init();
            sample_main();
        }
        CATCH_OTHER(e) {
        }
        FINALLY {
        }
    }
    END_TRY;
}
