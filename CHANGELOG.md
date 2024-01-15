# Change log

## [1.6.1] - 2024-01-15
### Added
-

### Changed
- Updated version of cmocka used for unit tests
- Updated version of github actions used

### Fixed
-

## [1.6.0] - 2024-01-14
### Added
- Use CX_CHECK macro in `compare_recovery_phrase()`
- Added a `cx_crc32()` function
  - The implementation of `cx_crc32_hw()` on Ledger devices is buggy and produces incorrect CRC32 checks. Ledger are fixing `cx_crc32_hw()` on each device either through SDK or OS updates but until then `cx_crc32()` can be used.

### Changed
-

### Fixed
-

## [1.5.4] - 2023-11-30
### Added
- Added mandatory Ledger embedded application manifest file

### Changed
- Combined BIP39 wordlist and SSKR wordlist unit tests

### Fixed
-

## [1.5.3] - 2023-11-18
### Added
- Added unit tests for BIP39
- Added unit tests for BIP39 word list and SSKR word list

### Changed
- 

### Fixed
- Fixed CodeQL warnings about sign check of a bitwise operation
- Fixed issue with restarting input from a previous word on Nano S

## [1.5.2] - 2023-11-15
### Added
-

### Changed
- Save memory by setting the SSKR word buffer to a sensible size
  - There is just enough memory available on Nano S to hold the phrases for 10 shares. Other devices can hold the full 16 shares.
- Tidied up code that sets 'Processing' screen on Nano S devices
- Changed all Variable Length Arrays to a defined length

### Fixed
- Fix freezing at 'Processing' screen on Nano S devices

## [1.5.1] - 2023-11-09
### Added
- Added unit tests for shamir
- Added unit tests for SSKR
- Added unit tests for BIP39 <-> SSKR roundtrip

### Changed
- Reduce size of Nano binaries slightly by removing duplicate flows
- Make generic SSKR functionality more Ledger specific

### Fixed
-

## [1.5.0] - 2023-10-20
### Added
- Added option to generate BIP39 mnemonics from SSKR shares even if shares do not validate against seed on device
  - A user may have lost or damaged original device and now needs to generate the recovery phrase from another secure device

### Changed
-

### Fixed
- Fixed build warning about UNUSED macro

## [1.4.1] - 2023-10-13
### Added
- Clear buffers before exiting

### Changed
-

### Fixed
-

## [1.4.0] - 2023-05-14
### Added
- Added BIP39 Check for Ledger Stax

### Changed
-

### Fixed
- Fixed warnings about deprecated functions during build

## [1.3.2] - 2023-05-08
### Added
- Added some Ledger specific preprocessor conditionals to bc-sskr and bc-shamir

### Changed
- Changed memset(x, 0, y) to memzero(x, y) macro

### Fixed
-

## [1.3.1] - 2023-05-03
### Added
-

### Changed
-

### Fixed
- Fixed static analyzer warning about zero-length array
- Fixed CodeQL warnings about comparison of narrow type with wide type in loop condition

## [1.3.0] - 2023-04-27
### Added
-

### Changed
- Removed duplicated nano code
- Simplified flow code

### Fixed
-

## [1.2.0] - 2023-04-21
### Added
- Added automated tests
- Added flow to set SSKR threshold values

### Changed
-

### Fixed
-

## [1.1.1] - 2023-04-06
### Added
-

### Changed
-

### Fixed
- Fix issue with using 'cx_crc32_hw()' function in 'onboarding_seed_sskr.c' when testing with Speculos
- Some CodeQL suggested tidy ups

## [1.1.0] - 2023-04-04
### Added
- Generate BIP39 mnemonic phrases from SSKR shares
  - Add 'SSKR Check' menu option
  - Add flow to the 'SSKR Check' menu
  - Write SSKR to BIP39 functionality
  - Test with 29-word SSKR shares
  - Test with 46-word SSKR shares
  - Test on nanos
  - Test on nanosp
  - Test on nanox

### Changed
-

### Fixed
-

## [1.0.1] - 2023-03-21

### Added
- Clone app-recovery-check and rename to app-sskr-check
- Add SSKR (bc-sskr and bc-shamir) to app-sskr-check
- Generate SSKR shares from BIP39 mnemonic phrase
  - Write BIP39 to SSKR functionality
  - Add SSKR flow to the Check BIP39 menu
  - Test with 12-word BIP39 phrases
  - Test with 18-word BIP39 phrases
  - Test with 24-word BIP39 phrases
  - Test on nanos
  - Test on nanosp
  - Test on nanox

### Changed
-

### Fixed
-
