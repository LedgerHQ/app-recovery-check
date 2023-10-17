<p align="center"><img src="icons/seed_tool.png" alt="Seed Tool" style="width:10%;height:10%"/></p>

# Seed Tool: A Ledger application that provides some useful seed management utilities

[![Build app-seed-tool](https://github.com/aido/app-seed-tool/actions/workflows/ci-workflow.yml/badge.svg)](https://github.com/aido/app-seed-tool/actions/workflows/ci-workflow.yml)
[![CodeQL](https://github.com/aido/app-seed-tool/actions/workflows/codeql-workflow.yml/badge.svg)](https://github.com/aido/app-seed-tool/actions/workflows/codeql-workflow.yml)
[![Code style check](https://github.com/aido/app-seed-tool/actions/workflows/lint-workflow.yml/badge.svg)](https://github.com/aido/app-seed-tool/actions/workflows/lint-workflow.yml)
[![License](https://img.shields.io/github/license/aido/app-seed-tool)](https://github.com/aido/app-seed-tool/blob/develop/LICENSE)

![nanos](https://img.shields.io/badge/nanos-working-green)
![nanox](https://img.shields.io/badge/nanox-working-green])
![nanosp](https://img.shields.io/badge/nanosp-working-green)
![stax](https://img.shields.io/badge/stax-in_progress-orange)

Use the utilities provided by this Ledger application to check a backed up seed or generate [Shamir's Secret Sharing (SSS)](https://en.wikipedia.org/wiki/Shamir%27s_secret_sharing) for a seed.

## Check BIP39
The application invites the user to type a [BIP-39](https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki) mnemonic on their Ledger device. The BIP-39 mnemonic is compared to the onboarded seed and the application notifies the user whether both seeds match or not.

## Generate Shamir's secret sharing
When the seed is validated, the user can create [Shamir's secret sharing (SSS)](https://en.wikipedia.org/wiki/Shamir%27s_secret_sharing) from the BIP-39 phrase that they provided.
The application uses [Sharded Secret Key Reconstruction (SSKR)](https://github.com/BlockchainCommons/Research/blob/master/papers/bcr-2020-011-sskr.md), an interoperable implementation of [Shamir's Secret Sharing (SSS)](https://en.wikipedia.org/wiki/Shamir%27s_secret_sharing). This provides a way for you to divide or 'shard' the master seed underlying a Bitcoin HD wallet into 'shares', which you can then distribute to friends, family, or fiduciaries. If you lose your seed, you can reconstruct it by collecting a sufficient number of your shares (the 'threshold'). Knowledge of fewer than the required number of parts ensures that information about the master secret is not leaked.

For more information about SSKR, see [SSKR for Users](https://github.com/BlockchainCommons/crypto-commons/blob/master/Docs/sskr-users.md).

## Check Shamir's secret shares
The Ledger application also provides an option to confirm the onboarded seed against SSKR shares.

## Generate [BIP85](https://github.com/bitcoin/bips/blob/master/bip-0085.mediawiki)
Coming soon!!!!

## Ledger Nano application menus
```mermaid
---
title: Seed Tool Nano App Flow
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

## [BIP85](https://github.com/bitcoin/bips/blob/master/bip-0085.mediawiki)
BIP85 allows you to do crazy stuff like this:
```mermaid
---
title: One Seed to rule them all - Multi wallet
---
flowchart TB
    1.1 --> |Backup| 1.2
    1 --> |BIP85 Child 0| 2.1.1
    1 --> |BIP85 Child 1| 2.1.2
    1 --> |BIP85 Child 2| 2.2.1
    1 --> |BIP85 Child 3| 2.2.2
    1 --> |BIP85 Child 4| 2.3.1
    1 --> |BIP85 Child 5| 2.3.2
    1 --> |BIP85 Child 6| 2.4.1
    1 --> |BIP85 Child 7| 2.4.2
    subgraph 1[Parent]
        direction TB
        1.1[Root Seed]
        subgraph 1.2[2-of-3 Shamir's Secret Shares]
            direction BT
            1.2.1[Share 1]
            1.2.2[Share 2]
            1.2.3[Share 3]
        end
    end
    subgraph 2[Children]
        direction TB
        subgraph 2.1[Cold Wallet]
            direction LR
            2.1.1[BIP39 #1]
            2.1.2[Password #1]
            end
            subgraph 2.2[Hardware Wallet]
            direction LR
            2.2.1[BIP39 #2]
            2.2.2[Password #2]
            end
            subgraph 2.3[Lightning Wallet]
            direction LR
            2.3.1[BIP39 #3]
            2.3.2[Password #3]
            end
            subgraph 2.4[Phone Wallet]
            direction LR
            2.4.1[BIP39 #4]
            2.4.2[Password #4]
            end
    end
```
```mermaid
---
title: One Seed to rule them all - MultiSig
---
flowchart TB
    1.1 --> |Backup| 1.2
    1 --> |BIP85 Child 0| 2.1.1
    1 --> |BIP85 Child 1| 2.1.2
    1 --> |BIP85 Child 2| 2.2.1
    1 --> |BIP85 Child 3| 2.2.2
    1 --> |BIP85 Child 4| 2.3.1
    1 --> |BIP85 Child 5| 2.3.2
    2.1 --> 3.1
    2.2 --> 3.2
    2.3 --> 3.3
    subgraph 1[Parent]
        direction TB
        1.1[Root Seed]
        subgraph 1.2[2-of-3 Shamir's Secret Shares]
            direction BT
            1.2.1[Share 1]
            1.2.2[Share 2]
            1.2.3[Share 3]
        end
    end
    subgraph 2[Children]
        direction TB
        subgraph 2.1[Wallet #1]
            direction LR
            2.1.1[BIP39 #1]
            2.1.2[Password #1]
            end
            subgraph 2.2[Wallet #2]
            direction LR
            2.2.1[BIP39 #2]
            2.2.2[Password #2]
            end
            subgraph 2.3[Wallet #3]
            direction LR
            2.3.1[BIP39 #3]
            2.3.2[Password #3]
            end
    end
    subgraph 3[2-of-3 MultiSig Wallet]
        direction LR
        3.1[Signer 1]
        3.2[Signer 2]
        3.3[Signer 3]
    end
```

