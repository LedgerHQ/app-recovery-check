# Change log

## [1.4.0] - 2023-05-14
### Added
- Added BIP39 Check for Ledger Stax

### Changed
-

### Fixed
-  Fixed warnings about deprecated functions during build

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
