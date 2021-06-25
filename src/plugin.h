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

#pragma once

#include <string.h>
#include <ctype.h>
#include "cx.h"
#include "eth_plugin_interface.h"

// All possible selectors of your plugin.
// A Xmacro below will create for you:
//     - an enum named selector_t with every NAME
//     - a map named SELECTORS associating each NAME with it's value
#define SELECTORS_LIST(X)                  \
    X(DEPOSIT_INTO_STRATEGY, 0xe7a050aa)   \
    X(DELEGATE_TO, 0xeea9064b)             \
    X(UNDELEGATE, 0xda8be864)              \
    X(QUEUE_WITHDRAWAL_PARAMS, 0x0dd8dd02) \
    X(COMPLETE_QUEUED_WITHDRAWALS, 0x33404396)

// Xmacro helpers to define the enum and map
// Do not modify !
#define TO_ENUM(selector_name, selector_id)  selector_name,
#define TO_VALUE(selector_name, selector_id) selector_id,

// This enum will be automatically expanded to hold all selector names.
// The value SELECTOR_COUNT can be used to get the number of defined selectors
// Do not modify !
typedef enum selector_e {
    SELECTORS_LIST(TO_ENUM) SELECTOR_COUNT,
} selector_t;

// This array will be automatically expanded to map all selector_t names with the correct value.
// Do not modify !
extern const uint32_t SELECTORS[SELECTOR_COUNT];

#define STRATEGIES_COUNT           11
#define UNKNOWN_TOKEN              15
#define UNKNOWN_STRATEGY           15
#define ERC20_DECIMALS             18
#define MAX_DISPLAYABLE_STRATEGIES 32  // > STRATEGIES_COUNT
// ADDRESS_STR_LEN is 0x + addr + \0
#define ADDRESS_STR_LEN 43

extern const char strategy_addresses[STRATEGIES_COUNT][ADDRESS_STR_LEN];
extern const char token_addresses[STRATEGIES_COUNT][ADDRESS_STR_LEN];
extern const char tickers[STRATEGIES_COUNT][MAX_TICKER_LEN];

// Enumeration used to parse the smart contract data.
typedef enum {
    STRATEGY = 0,
    TOKEN,
    AMOUNT,
    STAKER,
    OPERATOR,
    NONE,
    DATA_OFFSET,
    SHARE,
    WITHDRAWER,
    TUPLE_SIZE,
    SHARES_OFFSET,
    TUPLE_OFFSET,
    WITHDRAWER_OFFSET,
    WITHDRAWER_SIZE,
    SHARES_SIZE,
    STRATEGY_OFFSET,
    STRATEGY_SIZE,
    WITHDRAWALS_OFFSET,
    TOKEN_TUPLE_SIZE,
    TOKEN_OFFSET,
    TOKEN_SIZE,
    DELEGATE,
    TOKEN_ELEMENT_OFFSET,
    SIGNATURE_OFFSET,
    APPROVER_SALT,
    SIGNATURE_SIG_OFFSET,
    SIGNATURE_EXPIRACY,
    SIGNATURE_LENGTH,
    SIGNATURE_PACKETS,
    WITHDRAWALS_SIZE,
    WITHDRAWALS_ITEM_OFFSET,
    MIDDLEWARE_TIMES_OFFSET,
    MIDDLEWARE_TIMES_SIZE,
    RECEIVE_AS_TOKENS_OFFSET,
    TOKENS_SIZE,
    TOKEN_OFFSET_ITEMS,
    TOKENS_ITEM_SIZE,
    TOKENS_ITEM_ELEMENT,
    NONCE,
    START_BLOCK,
    RECEIVE_AS_TOKEN_SIZE,
    MIDDLEWARE_TIMES_ITEM,
    RECEIVE_AS_TOKEN_ITEM,
    UNEXPECTED_PARAMETER,
} parameter;

typedef struct {
    uint8_t value[ADDRESS_LENGTH];
} address_t;

typedef struct {
    uint16_t len;
    uint8_t text[PARAMETER_LENGTH + 1];
    bool ellipsis;
} name_t;

typedef struct {
    uint8_t value[INT256_LENGTH];
    bool ellipsis;
} bytes32_t;

typedef struct {
    int strategy;
    int token;
    bytes32_t amount;
} deposit_into_strategy_t;

typedef struct {
    address_t staker;
} undelegate_t;

typedef struct {
    address_t operator;
    uint16_t signature_packet_count;
} delegate_to_t;

typedef struct {
    uint8_t prev_checksum[CX_KECCAK_256_SIZE];
    uint32_t new_offset;
} checksum_offset_params_t;

typedef struct {
    // for parsing
    uint16_t queued_withdrawals_count;
    uint16_t current_item_count;
    uint16_t shares_array_offset;
    uint8_t qwithdrawals_offsets_checksum_preview[CX_KECCAK_256_SIZE];
    uint8_t qwithdrawals_offsets_checksum_value[CX_KECCAK_256_SIZE];

    // for display
    char withdrawer[ADDRESS_STR_LEN];
    uint8_t strategies_count;
    // list of strategies indexes **INCREMENTED BY 1** to display in the UI
    // 0 is reserved for end of array
    // UNKNOWN_LR_STRATEGY is used to display the "unknown" strategy
    // assumptions:
    // (i) in practice, we should not encounter more than
    //      LR_STRATEGIES_COUNT +~ a few potential unsupported
    //      strategies in the plugin. So * 3 should be a good enough buffer.
    // (ii) in practice there should not be more than (2 ** 8) - 2 known strategies
    uint8_t strategies[MAX_DISPLAYABLE_STRATEGIES];
} queue_withdrawal_t;

// must be size of MAX_DISPLAYABLE_STRATEGIES
typedef struct {
    uint32_t bits;
} bitfield;

typedef struct {
    // -- utils
    uint16_t parent_item_count;
    uint16_t current_item_count;
    // -- total values
    uint16_t relegations_count;
    uint8_t withdrawals_count;
    uint16_t strategies_count;
    uint16_t tokens_count;
    // -- offset verification
    uint16_t tokens_offset;
    uint16_t middlewareTimesIndexes_offset;
    uint16_t receiveAsTokens_offset;
    uint16_t cached_offset;
    uint8_t withdrawals_offsets_checksum_preview[CX_KECCAK_256_SIZE];
    uint8_t withdrawals_offsets_checksum_value[CX_KECCAK_256_SIZE];

    // -- display
    uint8_t withdrawer[ADDRESS_LENGTH];
    // list of strategies indexes **INCREMENTED BY 1** to display in the UI
    // UNKNOWN_LR_STRATEGY is used to display the "unknown" strategy
    // assumptions:
    // (i) in practice, we should not encounter more than
    //      LR_STRATEGIES_COUNT +~ a few potential unsupported
    //      strategies in the plugin. So * 3 should be a good enough buffer.
    // (ii) in practice there should not be more than (2 ** 4) - 2 known strategies
    // (iii) the first 4 bytes are the withdrawal index, the next 4 bytes are the strategy index
    uint8_t strategies[MAX_DISPLAYABLE_STRATEGIES];
} complete_queued_withdrawals_t;

// Shared global memory with Ethereum app. Must be at most 5 * 32 bytes.
typedef struct context_s {
    // For display
    union {
        deposit_into_strategy_t deposit_into_strategy;
        undelegate_t undelegate;
        delegate_to_t delegate_to;
        queue_withdrawal_t queue_withdrawal;
        complete_queued_withdrawals_t complete_queued_withdrawals;
    } tx;

    // For parsing data.
    uint8_t next_param;  // Set to be the next param we expect to parse.
    uint16_t offset;     // Offset at which the array or struct starts.
    bool go_to_offset;   // If set, will force the parsing to iterate through parameters until
                         // `offset` is reached.

    // For both parsing and display.
    selector_t selectorIndex;
} context_t;

// Check if the context structure will fit in the RAM section ETH will prepare for us
// Do not remove!
ASSERT_SIZEOF_PLUGIN_CONTEXT(context_t);
