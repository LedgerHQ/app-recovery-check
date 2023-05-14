# Ledger Application providing some useful seed management utilities

[![Build app-sskr-check](https://github.com/aido/app-sskr-check/actions/workflows/ci-workflow.yml/badge.svg)](https://github.com/aido/app-sskr-check/actions/workflows/ci-workflow.yml)
[![CodeQL](https://github.com/aido/app-sskr-check/actions/workflows/codeql-workflow.yml/badge.svg)](https://github.com/aido/app-sskr-check/actions/workflows/codeql-workflow.yml)
[![Code style check](https://github.com/aido/app-sskr-check/actions/workflows/lint-workflow.yml/badge.svg)](https://github.com/aido/app-sskr-check/actions/workflows/lint-workflow.yml)
[![License](https://img.shields.io/github/license/aido/app-sskr-check)](https://github.com/aido/app-sskr-check/blob/develop/LICENSE)

This Ledger application provides some utilities for checking a backed-up seed or generating [Shamir's Secret Shares (SSS)](https://en.wikipedia.org/wiki/Shamir%27s_secret_sharing) of a seed.

## Check BIP39
The application invites the user to type a [BIP-39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki) mnemonic on their Ledger device, this is compared against the onboarded seed and the user is informed whether both seeds are matching or not.

## Generate Shamir's Secret Shares
Once the seed is validated the user is offered the option to create [Shamir's Secret Shares (SSS)](https://en.wikipedia.org/wiki/Shamir%27s_secret_sharing) from the BIP-39 phrase they provided.
The application uses [Sharded Secret Key Reconstruction (SSKR)](https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-011-sskr.md), an interoperable implementation of [Shamir's Secret Sharing (SSS)](https://en.wikipedia.org/wiki/Shamir%27s_secret_sharing). It is a way that you can divide ("shard") the master seed underlying a Bitcoin HD wallet into "shares", which you can then distribute to friends, family, or fiduciaries. If you ever lose your seed, you can then "reconstruct" it by collecting a sufficient number of your shares (the "threshold"). Knowledge of fewer than the required number of parts does not leak information about the master secret.

Further explanation of SSKR may be found [here](https://github.com/BlockchainCommons/crypto-commons/blob/master/Docs/sskr-users.md).

## Check Shamir's Secret Shares
The application also provides an option to confirm the onboarded seed against SSKR shares. 

## Generate [BIP85](https://github.com/scgbckbone/bips/blob/passwords/bip-0085.mediawiki)
Coming soon!!!! 

## Application Menus
```mermaid
---
title: SSKR Check App Flow
---
flowchart LR
    1 --- 2 --- 3 --- 4
    subgraph 1[BIP39]
        direction TB
        1.1[Check BIP39]
        1.1 --> 1.2.1[Enter 12 Words] --> 1.3{Validate BIP39 Phrases}
        1.1 --> 1.2.2[Enter 18 Words] --> 1.3
        1.1 --> 1.2.3[Enter 24 Words] --> 1.3
        1.3 --> |Valid BIP39| 1.4
        1.3 --> |Invalid BIP39| 1.3.1[Quit]
        subgraph 1.4[Generate SSKR Shares]
            direction TB
            1.4.1[Select number of shares] --> 1.4.2[Select threshold] --> 1.4.3[Generate SSKR Shares] --> 1.4.4[Display SSKR Shares] --> 1.4.5[Quit]
        end
    end
    subgraph 2[SSKR]
        direction TB
        2.1[Check SSKR] --> 2.2[Enter SSKR Shares] --> 2.3{Validate SSKR Shares}
        2.3 --> |Valid SSKR| 2.4
        2.3 --> |Invalid SSKR| 2.3.1[Quit]
        subgraph 2.4[Generate BIP39 Phrases]
            direction TB
            2.4.1[Generate BIP39 Phrases] --> 2.4.2[Display BIP39 Phrases] --> 2.4.3[Quit]
        end
    end
    subgraph 3[Version]
        direction TB
        3.1[Version]
        end
    subgraph 4[Quit]
        direction TB
        4.1[Quit]
    end
```
