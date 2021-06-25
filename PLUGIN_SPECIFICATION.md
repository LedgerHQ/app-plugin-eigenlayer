# Technical Specification

## About

This documentation describes the smart contracts and functions supported by the EigenLayer plugin.

## Smart Contracts

Smart contracts covered by the plugin shall be described here:

|  Network | Version | Smart Contract | Address |
|   ----   |   ---   |      ----      |   ---   |
| Ethereum   | --  | DelegationManager  | `0x39053D51B77DC0d36036Fc1fCc8Cb819df8Ef37A` |
| Ethereum   | --  | StrategyManager  | `0x858646372CC42E1A627fcE94aa7A7033e7CF075A` |

## Functions

For the smart contracts implemented, the functions covered by the plugin shall be described here:

|Contract |    Function   | Selector  | Displayed Parameters |
|   ---   |    ---        | ---       | --- |
| StrategyManager | depositIntoStrategy           | `0xe7a050aa`| <table><tbody> <tr><td><code>address strategy</code></td></tr> <tr><td><code>address token</code></td></tr> <tr><td><code>uint256 amount</code></td></tr> </tbody></table> |
| DelegationManager | undelegate           | `0xda8be864`| <table><tbody> <tr><td><code>address staker</code></td></tr></tbody></table> |
| DelegationManager | delegateTo          | `0xeea9064b`| <table><tbody> <tr><td><code>address operator</code></td></tr></tbody></table> |
| DelegationManager | queueWithdrawals**           | `0x0dd8dd02`| <table><tbody> <tr><td><code>address strategy</code></td></tr> <tr><td><code>uint256 shares</code></td></tr> <tr><td><code>address withdrawer</code></td></tr> </tbody></table> |
| DelegationManager | completeQueuedWithdrawals**           | `0x33404396`| <table><tbody> <tr><td><code>address staker</code></td></tr> <tr><td><code>address delegateTo</code></td></tr> <tr><td><code>address withdrawer</code></td></tr>  <tr><td><code>address token</code></td></tr></tbody></table> |

** Due to memory and struture limitation of the plugin, app will only be able to show first element of the tupples.
