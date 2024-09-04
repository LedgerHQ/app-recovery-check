#*******************************************************************************
#   Ledger Blue
#   (c) 2016 Ledger
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#*******************************************************************************

ifeq ($(BOLOS_SDK),)
$(error Environment variable BOLOS_SDK is not set)
endif
include $(BOLOS_SDK)/Makefile.defines

all: default

APPNAME = "Recovery Check"
APPVERSION_M = 1
APPVERSION_N = 4
APPVERSION_P = 1
APPVERSION   = "$(APPVERSION_M).$(APPVERSION_N).$(APPVERSION_P)"

VARIANT_PARAM = NONE
VARIANT_VALUES = recovery_check

CURVE_APP_LOAD_PARAMS = secp256k1
PATH_APP_LOAD_PARAMS = ""
HAVE_APPLICATION_FLAG_DERIVE_MASTER = 1

ICON_NANOS = icons/nanos_recovery_check.gif
ICON_NANOSP = icons/nanox_recovery_check.gif
ICON_NANOX = icons/nanox_recovery_check.gif
ICON_STAX = icons/stax_recovery_check.gif
ICON_FLEX = icons/flex_recovery_check.gif

DEFINES += OS_IO_SEPROXYHAL
DEFINES += HAVE_WEBUSB WEBUSB_URL_SIZE_B=0 WEBUSB_URL=""
DEFINES += BOLOS_APP_ICON_SIZE_B=\(9+32\)
#DEFINES += HAVE_ELECTRUM
DEFINES += IO_USB_MAX_ENDPOINTS=4 IO_HID_EP_LENGTH=64
DEFINES += HAVE_SPRINTF

ifneq ($(TARGET_NAME), $(filter $(TARGET_NAME), TARGET_STAX TARGET_FLEX))
    $(info Using BAGL)
    DEFINES += HAVE_BAGL
    ifneq ($(TARGET_NAME), TARGET_NANOS)
        DEFINES += IO_SEPROXYHAL_BUFFER_SIZE_B=300
        DEFINES += HAVE_GLO096
        DEFINES += BAGL_WIDTH=128 BAGL_HEIGHT=64
        DEFINES += HAVE_BAGL_ELLIPSIS # long label truncation feature
        DEFINES += HAVE_BAGL_FONT_OPEN_SANS_REGULAR_11PX
        DEFINES += HAVE_BAGL_FONT_OPEN_SANS_EXTRABOLD_11PX
        DEFINES += HAVE_BAGL_FONT_OPEN_SANS_LIGHT_16PX
    else
        DEFINES += IO_SEPROXYHAL_BUFFER_SIZE_B=128
    endif
else
    $(info Using NBGL)
    DEFINES += IO_SEPROXYHAL_BUFFER_SIZE_B=300
    DEFINES += NBGL_KEYBOARD
endif

DEBUG = 0

APP_SOURCE_PATH += src

include $(BOLOS_SDK)/Makefile.standard_app
