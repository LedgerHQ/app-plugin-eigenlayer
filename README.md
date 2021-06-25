# Ledger Plugin EigenLayer
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
---

![zondax_light](docs/zondax_light.png#gh-light-mode-only)
![zondax_dark](docs/zondax_dark.png#gh-dark-mode-only)

_Please visit our website at [zondax.ch](https://www.zondax.ch)_

---

This is a plugin for the Ethereum application which helps parsing and displaying relevant information when signing a EigenLayer smart contract.

- Ledger Nano S/S+/X/Stax EigenLayer plugin
- Specs / Documentation
- Ragger tests

## ATTENTION

Please:

- **Do not use in production**
- **Do not use a Ledger device with funds for development purposes.**
- **Have a separate and marked device that is used ONLY for development and testing**



## Documentation

Need more information about the interface, the architecture, or general stuff about ethereum plugins? You can find more about them in the [ethereum-app documentation](https://github.com/LedgerHQ/app-ethereum/blob/master/doc/ethapp_plugins.adoc).

## Smart Contracts

Smart contracts covered by the plugin:

|  Network | Version | Smart Contract | Address |
|   ----   |   ---   |      ----      |   ---   |
| Ethereum   | --  | DelegationManager  | `0x39053D51B77DC0d36036Fc1fCc8Cb819df8Ef37A` |
| Ethereum   | --  | StrategyManager  | `0x858646372CC42E1A627fcE94aa7A7033e7CF075A` |

## Functions

For the smart contracts implemented, the functions covered by the plugin are the following:

|Contract |    Function   | Selector  | Displayed Parameters |
|   ---   |    ---        | ---       | --- |
| StrategyManager | depositIntoStrategy           | `0xe7a050aa`| <table><tbody> <tr><td><code>address strategy</code></td></tr> <tr><td><code>address token</code></td></tr> <tr><td><code>uint256 amount</code></td></tr> </tbody></table> |
| DelegationManager | undelegate           | `0xda8be864`| <table><tbody> <tr><td><code>address staker</code></td></tr></tbody></table> |
| DelegationManager | delegateTo          | `0xeea9064b`| <table><tbody> <tr><td><code>address operator</code></td></tr></tbody></table> |
| DelegationManager | queueWithdrawals**           | `0x0dd8dd02`| <table><tbody> <tr><td><code>address strategy</code></td></tr> <tr><td><code>uint256 shares</code></td></tr> <tr><td><code>address withdrawer</code></td></tr> </tbody></table> |
| DelegationManager | completeQueuedWithdrawals**           | `0x33404396`| <table><tbody> <tr><td><code>address staker</code></td></tr> <tr><td><code>address delegateTo</code></td></tr> <tr><td><code>address withdrawer</code></td></tr>  <tr><td><code>address token</code></td></tr></tbody></table> |

** Due to memory and structure limitation of the plugin, app will only be able to show first element of the tupples.

## How to build

Ledger's recommended [plugin guide](https://developers.ledger.com/docs/dapp/embedded-plugin/code-overview/) is out-dated and doesn't work since they introduced a lot of new changes. Here's a simple way to get started with this repo:
1. Clone this repo (along with git submodules)
2. Install [Xquartz](https://www.xquartz.org/) and make sure you have enabled "Allow connections from network clients" enabled under "Security" settings.
3. Install and start Docker (Note: If Docker is already running, quit and start it after starting Xquartz, otherwise docker cannot connect to Xquartz).
4. Install the [Ledger Dev Tools VS Code plugin](https://marketplace.visualstudio.com/items?itemName=LedgerHQ.ledger-dev-tools#:~:text=ledger%2Dvscode%2Dextension,Plus%2C%20Nano%20X%2C%20Stax) and makes sure it's enabled
5. Once you have installed the plugin and open the repo, the plugin should by default try to create and start the containers. If it doesn't, you can simply click "Update Container" under "Ledger Dev Tools" in the Activity Side Bar on VS Code.
6. On the "Ledger Dev Tools" side bar, Select a target and then click on Build. 
7. Once build is complete, click on "Run tests" to run the tests

