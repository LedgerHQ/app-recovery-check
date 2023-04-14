# Ledger Application to create and check a Sharded Secret Key Reconstruction (SSKR) seed backup

[![Build app-sskr-check](https://github.com/aido/app-sskr-check/actions/workflows/ci-workflow.yml/badge.svg)](https://github.com/aido/app-sskr-check/actions/workflows/ci-workflow.yml)
[![CodeQL](https://github.com/aido/app-sskr-check/actions/workflows/codeql-workflow.yml/badge.svg)](https://github.com/aido/app-sskr-check/actions/workflows/codeql-workflow.yml)
[![Code style check](https://github.com/aido/app-sskr-check/actions/workflows/lint-workflow.yml/badge.svg)](https://github.com/aido/app-sskr-check/actions/workflows/lint-workflow.yml)
[![License](https://img.shields.io/github/license/aido/app-sskr-check)](https://github.com/aido/app-sskr-check/blob/develop/LICENSE)

This application invites the user to type a BIP-39 seed on their Ledger device, this seed is compared against the onboarded seed, and the user is informed whether both seeds are matching or not. Once the seed is proven the user is offered the option to create SSKR shares from the BIP-39 phrase they provided.
The app also provides an option to confirm the onboarded seed against SSKR shares. 
[Sharded Secret Key Reconstruction (SSKR)](https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-011-sskr.md) is an implementation of Shamir's Secret Sharing (SSS). SSS splits a master secret, such as the master seed S for Hierarchical Deterministic Wallets described in BIP32, into unique parts which can be distributed amongst participants. A specified minimum number of parts is required to be supplied in order to reconstruct the original secret. Knowledge of fewer than the required number of parts does not leak information about the master secret.
