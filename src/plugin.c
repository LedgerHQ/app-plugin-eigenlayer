/*******************************************************************************
 *   Plugin eigenlayer
 *   (c) 2023 Ledger
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#include <stdint.h>
#include "plugin.h"

// This array will be automatically expanded to map all selector_t names with the correct value.
// Do not modify !
const uint32_t SELECTORS[SELECTOR_COUNT] = {SELECTORS_LIST(TO_VALUE)};

const char strategy_addresses[STRATEGIES_COUNT][ADDRESS_STR_LEN] = {
    "0x54945180dB7943c0ed0FEE7EdaB2Bd24620256bc",  // cbETH
    "0x93c4b944D05dfe6df7645A86cd2206016c51564D",  // stETH
    "0x1BeE69b7dFFfA4E2d53C2a2Df135C388AD25dCD2",  // rETH
    "0x9d7eD45EE2E8FC5482fa2428f15C971e6369011d",  // ETHx
    "0x13760F50a9d7377e4F20CB8CF9e4c26586c658ff",  // ankrETH
    "0xa4C637e0F704745D182e4D38cAb7E7485321d059",  // OETH
    "0x57ba429517c3473B6d34CA9aCd56c0e735b94c02",  // osETH
    "0x0Fe4F44beE93503346A3Ac9EE5A26b130a5796d6",  // swETH
    "0x7CA911E83dabf90C90dD3De5411a10F1A6112184",  // wBETH
    "0x8CA7A5d6f3acd3A7A8bC468a8CD0FB14B6BD28b6",  // sfrxETH
    "0x298aFB19A105D59E74658C4C334Ff360BadE6dd2"   // mETH
};

const char token_addresses[STRATEGIES_COUNT][ADDRESS_STR_LEN] = {
    "0xBe9895146f7AF43049ca1c1AE358B0541Ea49704",  // cbETH
    "0xae7ab96520DE3A18E5e111B5EaAb095312D7fE84",  // stETH
    "0xae78736Cd615f374D3085123A210448E74Fc6393",  // rETH
    "0xA35b1B31Ce002FBF2058D22F30f95D405200A15b",  // ETHx
    "0xE95A203B1a91a908F9B9CE46459d101078c2c3cb",  // ankrETH
    "0x856c4Efb76C1D1AE02e20CEB03A2A6a08b0b8dC3",  // OETH
    "0xf1C9acDc66974dFB6dEcB12aA385b9cD01190E38",  // osETH
    "0xf951E335afb289353dc249e82926178EaC7DEd78",  // swETH
    "0xa2E3356610840701BDf5611a53974510Ae27E2e1",  // wBETH
    "0xac3E018457B222d93114458476f3E3416Abbe38F",  // sfrxETH
    "0xd5F7838F5C461fefF7FE49ea5ebaF7728bB0ADfa",  // mETH
};

const char tickers[STRATEGIES_COUNT][MAX_TICKER_LEN] = {"cbETH",
                                                        "stETH",
                                                        "rETH",
                                                        "ETHx",
                                                        "ankrETH",
                                                        "OETH",
                                                        "osETH",
                                                        "swETH",
                                                        "wBETH",
                                                        "sfrxETH",
                                                        "mETH"};
