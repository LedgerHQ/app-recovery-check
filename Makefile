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

# Main app configuration

APPNAME = "Recovery check"
APPVERSION = 1.0.0
ICONNAME = icon.gif
APP_LOAD_PARAMS = --appFlags 0x10 $(COMMON_LOAD_PARAMS) --apdu --curve secp256k1 --path ""

# Build configuration


DEFINES += APPVERSION=\"$(APPVERSION)\"

DEFINES += OS_IO_SEPROXYHAL IO_SEPROXYHAL_BUFFER_SIZE_B=128
DEFINES += HAVE_BAGL HAVE_SPRINTF
#DEFINES   += HAVE_PRINTF PRINTF=screen_printf
DEFINES += PRINTF\(...\)=
DEFINES   += BOLOS_APP_ICON_SIZE_B=\(9+32\)
DEFINES   += CX_COMPLIANCE_141
DEFINES   += HAVE_ELECTRUM


DEFINES += IO_USB_MAX_ENDPOINTS=7 IO_HID_EP_LENGTH=64

# Compiler, assembler, and linker

CC := $(CLANGPATH)clang
CFLAGS += -O3 -Os

AS := $(GCCPATH)arm-none-eabi-gcc
AFLAGS +=

LD := $(GCCPATH)arm-none-eabi-gcc
LDFLAGS += -O3 -Os
LDLIBS += -lm -lgcc -lc

# import rules to compile glyphs(/pone)
include $(BOLOS_SDK)/Makefile.glyphs

APP_SOURCE_PATH += src src_common
#SDK_SOURCE_PATH += lib_stusb lib_stusb_impl

# Main rules


load: all
	python -m ledgerblue.loadApp $(APP_LOAD_PARAMS)

delete:
	python -m ledgerblue.deleteApp $(COMMON_DELETE_PARAMS)

# Import generic rules from the SDK

include $(BOLOS_SDK)/Makefile.rules
