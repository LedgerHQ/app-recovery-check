# app-sskr-check TODO list

### Todo

- [ ] Add Ledger Stax to list of devices app works on
- [ ] Fix issue with using 'cx_crc32_hw()' function in 'onboarding_seed_sskr.c' when testing with Speculos
- [ ] Currently hardcoded to use a k-of-n threshold of 2-of-3. Add flow to set threshold values
- [ ] Save memory by setting the SSKR word buffer (G_bolos_ux_context.sskr_words_buffer) to a sensible size. Maybe just store SSKR Bytewords as shorter two letter minimal Bytewords rather than a 4 letter Byteword plus spaace for each share. Convert minimal ByteWords back to four letter Bytewords just prior to display.

### In Progress

- [ ] Generate BIP39 mnemonic phrases from SSKR shares
  - [ ] Write SSKR to BIP39 functionality
  - [ ] Add flow to the Check SSKR menu
  - [ ] Test with 29-word SSKR shares
  - [ ] Test with 38-word SSKR shares
  - [ ] Test with 46-word SSKR shares
  - [ ] Test on nanos
  - [ ] Test on nanosp
  - [ ] Test on nanox

### Done ✓

- [x] Clone app-recovery-check and rename to app-sskr-check
- [x] Add SSKR (bc-sskr and bc-shamir) to app-sskr-check
- [x] Generate SSKR shares from BIP39 mnemonic phrase
  - [x] Write BIP39 to SSKR functionality
  - [x] Add SSKR flow to the Check BIP39 menu
  - [x] Test with 12-word BIP39 phrases
  - [x] Test with 18-word BIP39 phrases
  - [x] Test with 24-word BIP39 phrases
  - [x] Test on nanos
  - [x] Test on nanosp
  - [x] Test on nanox