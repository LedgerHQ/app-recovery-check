# Ledger Application to create and check a Sharded Secret Key Reconstruction (SSKR) seed backup

[![Build app-sskr-check](https://github.com/aido/app-sskr-check/actions/workflows/ci-workflow.yml/badge.svg)](https://github.com/aido/app-sskr-check/actions/workflows/ci-workflow.yml)
[![CodeQL](https://github.com/aido/app-sskr-check/actions/workflows/codeql-workflow.yml/badge.svg)](https://github.com/aido/app-sskr-check/actions/workflows/codeql-workflow.yml)
[![Code style check](https://github.com/aido/app-sskr-check/actions/workflows/lint-workflow.yml/badge.svg)](https://github.com/aido/app-sskr-check/actions/workflows/lint-workflow.yml)
[![License](https://img.shields.io/github/license/aido/app-sskr-check)](https://github.com/aido/app-sskr-check/blob/develop/LICENSE)

This application invites the user to type their seed on their Ledger Nano device, this seed is compared against the onboarded seed, and the user is informed whether both seeds are matching or not. There are also options to create SSKR shares from BIP-39 phrase or to confirm the onboarded seed against SSKR shares. [Sharded Secret Key Reconstruction (SSKR)](https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-011-sskr.md) is an implementation of Shamir's Secret Sharing (SSS). SSS splits a master secret, such as the master seed S for Hierarchical Deterministic Wallets described in BIP32, into unique parts which can be distributed among participants. A specified minimum number of parts is required to be supplied in order to reconstruct the original secret. Knowledge of fewer than the required number of parts does not leak information about the master secret.

