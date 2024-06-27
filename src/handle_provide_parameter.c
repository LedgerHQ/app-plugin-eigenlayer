#include "plugin.h"

/**
 * @brief Compare two addresses
 *
 * @param a: first address
 * @param b: second address
 *
 * @return true if the addresses are the same
 */
bool compare_addresses(const char a[ADDRESS_STR_LEN], const char b[ADDRESS_STR_LEN]) {
    for (size_t i = 0; i < ADDRESS_STR_LEN; i += 1) {
        if (tolower((unsigned char) a[i]) != tolower((unsigned char) b[i])) {
            return false;
        }
    }
    return true;
}

/**
 * @brief If address is a known erc20 token, update display context with its name
 * otherwise set it to unkwown (UNKNOWN_ERC20)
 *
 * @param address: address to compare
 *
 * @returns index of the erc20 in the context or UNKNOWN_ERC20 if not found
 */
uint8_t decode_token(const char address[ADDRESS_STR_LEN]) {
    for (size_t i = 0; i < STRATEGIES_COUNT; i++) {
        if (compare_addresses(address, token_addresses[i])) {
            return i;
        }
    }
    return UNKNOWN_TOKEN;
}

/**
 * @brief If address is a known strategy, update display context with its name
 * otherwise set it to unkwown (UNKNOWN_STRATEGY)
 *
 * @param address: address to compare
 *
 * @returns index of the erc20 in the context or UNKNOWN_STRATEGY if not found
 */
uint8_t decode_strategy(const char address[ADDRESS_STR_LEN]) {
    for (size_t i = 0; i < STRATEGIES_COUNT; i++) {
        if (compare_addresses(address, strategy_addresses[i])) {
            return i;
        }
    }
    return UNKNOWN_STRATEGY;
}

/**
 * @brief Handle the parameters for the depositIntoStrategy selector
 *
 * @param msg: message containing the parameter
 * @param context: context to update
 *
 */
static void handle_deposit_into_strategy(ethPluginProvideParameter_t *msg, context_t *context) {
    uint8_t buffer[ADDRESS_LENGTH] = {0};
    char address[ADDRESS_STR_LEN] = {0};

    switch (context->next_param) {
        case STRATEGY:
            copy_address(buffer, msg->parameter, sizeof(buffer));
            if (!getEthDisplayableAddress(buffer, address, sizeof(address), 0)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->tx.deposit_into_strategy.strategy = decode_strategy(address);
            context->next_param = TOKEN;
            break;
        case TOKEN:
            copy_address(buffer, msg->parameter, sizeof(buffer));
            if (!getEthDisplayableAddress(buffer, address, sizeof(address), 0)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->tx.deposit_into_strategy.token = decode_token(address);
            context->next_param = AMOUNT;
            break;
        case AMOUNT:
            copy_parameter(context->tx.deposit_into_strategy.amount.value,
                           msg->parameter,
                           sizeof(context->tx.deposit_into_strategy.amount.value));
            context->next_param = UNEXPECTED_PARAMETER;
            break;
        default:
            PRINTF("Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

/**
 * @brief Handle the parameters for the undelegate selector
 *
 * @param msg: message containing the parameter
 * @param context: context to update
 *
 */
static void handle_undelegate(ethPluginProvideParameter_t *msg, context_t *context) {
    switch (context->next_param) {
        case STAKER:
            copy_address(context->tx.undelegate.staker.value,
                         msg->parameter,
                         sizeof(context->tx.undelegate.staker.value));
            context->next_param = UNEXPECTED_PARAMETER;
            break;
        default:
            PRINTF("Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

/**
 * @brief Handle the parameters for the delegate selector
 *
 * @param msg: message containing the parameter
 * @param context: context to update
 *
 */
static void handle_delegate(ethPluginProvideParameter_t *msg, context_t *context) {
    uint16_t offset = 0;
    switch (context->next_param) {
        case OPERATOR:
            copy_address(context->tx.delegate_to.operator.value,
                         msg->parameter,
                         sizeof(context->tx.delegate_to.operator.value));
            context->next_param = SIGNATURE_OFFSET;
            break;
        case SIGNATURE_OFFSET:
            if (!U2BE_from_parameter(msg->parameter, &offset) || offset != PARAMETER_LENGTH * 3) {
                // OFFSET bigger than this offset + signature salt
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->next_param = APPROVER_SALT;
            break;
        case APPROVER_SALT:
            context->next_param = SIGNATURE_SIG_OFFSET;
            break;
        case SIGNATURE_SIG_OFFSET:
            if (!U2BE_from_parameter(msg->parameter, &offset) || offset != PARAMETER_LENGTH * 2) {
                // OFFSET bigger than this offset + expericy
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->next_param = SIGNATURE_EXPIRACY;
            break;
        case SIGNATURE_EXPIRACY:
            context->next_param = SIGNATURE_LENGTH;
            break;
        case SIGNATURE_LENGTH:
            if (!U2BE_from_parameter(msg->parameter,
                                     &context->tx.delegate_to.signature_packet_count)) {
                msg->result = ETH_PLUGIN_RESULT_UNSUCCESSFUL;
            }
            if (context->tx.delegate_to.signature_packet_count == 0) {
                context->next_param = NONE;
            } else {
                context->next_param = SIGNATURE_PACKETS;
            }
            break;
        case SIGNATURE_PACKETS:
            context->tx.delegate_to.signature_packet_count -= 1;
            if (context->tx.delegate_to.signature_packet_count == 0) {
                context->next_param = NONE;
            }
            break;
        case NONE:
            break;
        default:
            PRINTF("Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

/**
 * @brief Handle the parameters for the queuedWithdrawal selector
 *
 * @param msg: message containing the parameter
 * @param context: context to update
 *
 */
static void handle_queue_withdrawal(ethPluginProvideParameter_t *msg, context_t *context) {
    queue_withdrawal_t *tx = &context->tx.queue_withdrawal;
    switch (context->next_param) {
        case DATA_OFFSET: {  // Data offset to beggin of the withdrawals tupple
            uint16_t offset = 0;
            if (!U2BE_from_parameter(msg->parameter, &offset) || offset != PARAMETER_LENGTH) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->next_param = WITHDRAWALS_SIZE;
            break;
        }
        case WITHDRAWALS_SIZE:  // Get size of tupple
            if (!U2BE_from_parameter(msg->parameter, &tx->queued_withdrawals_count)) {
                msg->result = ETH_PLUGIN_RESULT_UNSUCCESSFUL;
            }
            tx->current_item_count = tx->queued_withdrawals_count;
            context->next_param = WITHDRAWALS_ITEM_OFFSET;
            break;
        case WITHDRAWALS_ITEM_OFFSET: {
            // We have limited size on the context and can't store all the offset values
            // of the queuedWithdrawal structs. So we compute their checksum and expect to
            // be able to recompute it using the offset of the parsed structures later.
            // _preview will be equal to _value at the end of the parsing if everything is fine
            // Idea taken from Kiln plugin
            uint16_t offset = 0;
            if (!U2BE_from_parameter(msg->parameter, &offset)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            checksum_offset_params_t h_params;
            memset(&h_params, 0, sizeof(h_params));
            memcpy(&h_params.prev_checksum,
                   &(tx->qwithdrawals_offsets_checksum_preview),
                   sizeof(h_params.prev_checksum));

            // we hash the previous checksum with the offset of the beginning of the structure.
            // the offset we parse is actually after SELECTOR + the 2 above param so we add them to
            // it.
            h_params.new_offset = offset + SELECTOR_SIZE + PARAMETER_LENGTH * 2;

            if (cx_keccak_256_hash((void *) &h_params,
                                   sizeof(h_params),
                                   tx->qwithdrawals_offsets_checksum_preview) != CX_OK) {
                PRINTF("unable to compute keccak hash\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            // we skip all the queuewithdrawal structs offsets
            PRINTF("CURRENT ITEM COUNT: %d\n", tx->current_item_count);
            if (tx->current_item_count > 0) {
                tx->current_item_count -= 1;
            }

            if (tx->current_item_count == 0) {
                context->next_param = STRATEGY_OFFSET;
            }
            break;
        }
        case STRATEGY_OFFSET: {
            uint16_t offset = 0;
            if (!U2BE_from_parameter(msg->parameter, &offset)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            };
            // here we are at the beginning of the queuedWithdrawal struct, so we want
            // to compute the offset of the struct we're at for the queuedWithdrawal struct array
            // offsets checksum

            checksum_offset_params_t h_params;
            memset(&h_params, 0, sizeof(h_params));
            memcpy(&h_params.prev_checksum,
                   &(tx->qwithdrawals_offsets_checksum_value),
                   sizeof(h_params.prev_checksum));
            h_params.new_offset = msg->parameterOffset;

            if (cx_keccak_256_hash((void *) &h_params,
                                   sizeof(h_params),
                                   tx->qwithdrawals_offsets_checksum_value) != CX_OK) {
                PRINTF("unable to compute keccak hash\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            if (tx->queued_withdrawals_count == 1) {
                // if we are on the last item of the array of queuedWithdrawal struct
                // we can check the checksum
                if (memcmp(tx->qwithdrawals_offsets_checksum_preview,
                           tx->qwithdrawals_offsets_checksum_value,
                           sizeof(tx->qwithdrawals_offsets_checksum_preview)) != 0) {
                    PRINTF("Checksums do not match\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
            }

            if (offset != PARAMETER_LENGTH * 3) {
                // valid offset should only skip this + shares_offset + withdrawer
                PRINTF("Unexpected parameter offset %d != %d\n", offset, PARAMETER_LENGTH * 3);
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->next_param = SHARES_OFFSET;
            break;
        }
        case SHARES_OFFSET:
            // we can only check this value once we know strategies array length
            if (!U2BE_from_parameter(msg->parameter, &tx->shares_array_offset)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            context->next_param = WITHDRAWER;
            break;
        case WITHDRAWER: {
            {
                uint8_t buffer[ADDRESS_LENGTH];
                copy_address(buffer, msg->parameter, sizeof(buffer));
                char address_buffer[ADDRESS_STR_LEN];
                if (!getEthDisplayableAddress(buffer, address_buffer, sizeof(address_buffer), 0)) {
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
                // we only support same withdrawer accross all the withdrawals
                if (tx->withdrawer[0] == '\0') {
                    memcpy(tx->withdrawer, address_buffer, sizeof(tx->withdrawer));
                } else if (strcmp(tx->withdrawer, address_buffer) != 0) {
                    PRINTF("Unexpected withdrawer address, %s != expected %s\n",
                           msg->parameter,
                           tx->withdrawer);
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
            }
            context->next_param = STRATEGY_SIZE;
            break;
        }
        case STRATEGY_SIZE: {
            if (!U2BE_from_parameter(msg->parameter, &tx->current_item_count)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            };

            {  // check that the shares array offset is correct
                // -- 1. we skip shares_offset + withdrawer + strategies_length
                uint16_t should_be = PARAMETER_LENGTH * 3
                                     // -- 2. we skip the strategies array length
                                     + PARAMETER_LENGTH + PARAMETER_LENGTH * tx->current_item_count;

                if (tx->shares_array_offset != should_be) {
                    PRINTF("Unexpected parameter offset %d != %d\n",
                           tx->shares_array_offset,
                           should_be);
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
            }

            if (tx->current_item_count == 0) {
                // if no strategies we go to the shares array
                context->next_param = SHARES_SIZE;
            } else {
                context->next_param = STRATEGY;
            }
            break;
        }

        case STRATEGY: {
            // get strategy we need to display
            {
                uint8_t buffer[ADDRESS_LENGTH];
                copy_address(buffer, msg->parameter, sizeof(buffer));
                char address_buffer[ADDRESS_STR_LEN];
                if (!getEthDisplayableAddress(buffer, address_buffer, sizeof(address_buffer), 0)) {
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }

                uint8_t strategy_index = decode_strategy(address_buffer);
                tx->strategies[tx->strategies_count] =
                    (strategy_index != UNKNOWN_STRATEGY) ? strategy_index : UNKNOWN_STRATEGY;

                PRINTF("STRATEGY #: %d STRATEGY: %d\n", tx->strategies_count, strategy_index);
            }

            // we just processed one strategy item
            tx->strategies_count += 1;
            if (tx->current_item_count > 0) {
                tx->current_item_count -= 1;
            }
            if (tx->current_item_count == 0) {
                // when we arrive at the end of the strategies array we go to the shares array
                context->next_param = SHARES_SIZE;
            }
            break;
        }
        case SHARES_SIZE:
            if (!U2BE_from_parameter(msg->parameter, &tx->current_item_count)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            };
            // get number of items to parse

            context->next_param = SHARE;
            break;
        case SHARE:  // get shares value and finish
            // we skip parsing shares item as they are not needed for clearsigning
            // as not having ETH / ERC20 amount to display would confuse users
            if (tx->current_item_count > 0) {
                tx->current_item_count -= 1;
            }
            if (tx->current_item_count == 0) {
                // here we arrive at the end of the queuedWithdrawal array element

                // check if there are other queuedWithdrawals to parse
                tx->queued_withdrawals_count -= 1;
                if (tx->queued_withdrawals_count == 0) {
                    // if not we finished parsing
                    context->next_param = NONE;
                } else {
                    // if there are other queuedWithdrawals we go back to parsing the
                    // next queueWithdrawal struct
                    context->next_param = STRATEGY_OFFSET;
                }
            }
            break;
        case NONE:
            break;
        default:
            PRINTF("Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

/**
 * @brief Handle the parameters for the completeQueuedWithdrawals selector
 *
 * @param msg: message containing the parameter
 * @param context: context to update
 *
 */
static void handle_complete_queued_withdrawals(ethPluginProvideParameter_t *msg,
                                               context_t *context) {
    complete_queued_withdrawals_t *tx = &context->tx.complete_queued_withdrawals;

    switch (context->next_param) {
        case WITHDRAWALS_OFFSET: {
            uint16_t offset;
            if (!U2BE_from_parameter(msg->parameter, &offset) || offset != 4 * PARAMETER_LENGTH) {
                // valid offset should only skip this offset + tokens + middlewareTimesIndexes +
                // receiveAsTokens offsets
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            context->next_param = TOKEN_OFFSET;
            break;
        }
        case TOKEN_OFFSET:
            if (!U2BE_from_parameter(msg->parameter, &tx->tokens_offset)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->next_param = MIDDLEWARE_TIMES_OFFSET;
            break;
        case MIDDLEWARE_TIMES_OFFSET:
            if (!U2BE_from_parameter(msg->parameter, &tx->middlewareTimesIndexes_offset)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->next_param = RECEIVE_AS_TOKENS_OFFSET;
            break;
        case RECEIVE_AS_TOKENS_OFFSET:
            if (!U2BE_from_parameter(msg->parameter, &tx->receiveAsTokens_offset)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->next_param = WITHDRAWALS_SIZE;
            break;
        case WITHDRAWALS_SIZE:
            if (!U2BE_from_parameter(msg->parameter, &tx->parent_item_count)) {
                msg->result = ETH_PLUGIN_RESULT_UNSUCCESSFUL;
            }
            tx->current_item_count = tx->parent_item_count;

            if (tx->parent_item_count == 0) {
                context->next_param = TOKENS_SIZE;
            } else {
                context->next_param = WITHDRAWALS_ITEM_OFFSET;
            }
            break;
        case WITHDRAWALS_ITEM_OFFSET: {
            uint16_t offset;
            if (!U2BE_from_parameter(msg->parameter, &offset)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            // We have limited size on the context and can't store all the offset values
            // of the Withdrawal structs. So we compute their checksum and expect to
            // be able to recompute it using the offset of the parsed structures later.
            // _preview will be equal to _value at the end of the parsing if everything is fine
            checksum_offset_params_t h_params;
            memset(&h_params, 0, sizeof(h_params));
            memcpy(&h_params.prev_checksum,
                   &(tx->withdrawals_offsets_checksum_preview),
                   sizeof(h_params.prev_checksum));
            // we hash the previous checksum with the offset of the beginning of the structure.
            // the offset we parse is actually after SELECTOR + the 5 above param so we add them to
            // it.
            h_params.new_offset = offset + SELECTOR_SIZE + PARAMETER_LENGTH * 5;
            if (cx_keccak_256_hash((void *) &h_params,
                                   sizeof(h_params),
                                   tx->withdrawals_offsets_checksum_preview) != CX_OK) {
                PRINTF("unable to compute keccak hash\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            if (tx->current_item_count > 0) {
                tx->current_item_count -= 1;
            }
            if (tx->current_item_count == 0) {
                context->next_param = STAKER;
            }
            break;
        }
        case STAKER: {
            // here we are at the beginning of the queuedWithdrawal struct, so we want
            // to compute the offset of the struct we're at for the queuedWithdrawal struct array
            // offsets checksum

            checksum_offset_params_t h_params;
            memset(&h_params, 0, sizeof(h_params));
            memcpy(&(h_params.prev_checksum),
                   &(tx->withdrawals_offsets_checksum_value),
                   sizeof(h_params.prev_checksum));
            h_params.new_offset = msg->parameterOffset;

            if (cx_keccak_256_hash((void *) &h_params,
                                   sizeof(h_params),
                                   tx->withdrawals_offsets_checksum_value) != CX_OK) {
                PRINTF("unable to compute keccak hash\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            if (tx->parent_item_count == 1) {
                // if we are on the last item of the array of withdrawal struct
                // we can check the checksum
                if (memcmp(tx->withdrawals_offsets_checksum_preview,
                           tx->withdrawals_offsets_checksum_value,
                           sizeof(tx->withdrawals_offsets_checksum_preview)) != 0) {
                    PRINTF("Checksums do not match\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
            }
            context->next_param = DELEGATE_TO;
            break;
        }
        case DELEGATE_TO:
            context->next_param = WITHDRAWER;
            break;
        case WITHDRAWER: {
            {
                uint8_t buffer[ADDRESS_LENGTH];
                copy_address(buffer, msg->parameter, sizeof(buffer));
                // we only support same withdrawer accross all the withdrawals
                if (allzeroes(tx->withdrawer, sizeof(tx->withdrawer)) == 1) {
                    memcpy(tx->withdrawer, buffer, sizeof(tx->withdrawer));
                } else if (strcmp((const char *) tx->withdrawer, (const char *) buffer) != 0) {
                    PRINTF("Unexpected withdrawer address, %s != expected %s\n",
                           msg->parameter,
                           tx->withdrawer);
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
            }
            context->next_param = NONCE;
            break;
        }
        case NONCE:
            context->next_param = START_BLOCK;
            break;
        case START_BLOCK:
            context->next_param = STRATEGY_OFFSET;
            break;
        case STRATEGY_OFFSET: {
            {
                uint16_t offset;
                if (!U2BE_from_parameter(msg->parameter, &offset)) {
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }

                if (offset != PARAMETER_LENGTH * 7) {
                    // valid offset should only skip the above elements in the structure + this one
                    // + shares offset
                    PRINTF("Unexpected parameter offset %d != %d\n", offset, PARAMETER_LENGTH * 7);
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
            }
            context->next_param = SHARES_OFFSET;
            break;
        }
        case SHARES_OFFSET: {
            uint16_t offset;
            if (!U2BE_from_parameter(msg->parameter, &offset)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            // save offset to verify it on array start
            // offset starts at the beginning of the structure (current scope)
            tx->cached_offset = offset + (msg->parameterOffset - PARAMETER_LENGTH * 6);

            context->next_param = STRATEGY_SIZE;
            break;
        }
        case STRATEGY_SIZE:
            if (!U2BE_from_parameter(msg->parameter, &tx->current_item_count)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            if (tx->current_item_count == 0) {
                context->next_param = SHARES_SIZE;
            } else {
                context->next_param = STRATEGY;
            }
            break;
        case STRATEGY: {
            // get strategy we need to display
            {
                uint8_t buffer[ADDRESS_LENGTH];
                copy_address(buffer, msg->parameter, sizeof(buffer));
                char address_buffer[ADDRESS_STR_LEN];
                if (!getEthDisplayableAddress(buffer, address_buffer, sizeof(address_buffer), 0)) {
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }

                uint8_t strategy_index = decode_strategy(address_buffer);
                if (tx->strategies_count >= MAX_DISPLAYABLE_STRATEGIES) {
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
                uint8_t withdrawal = tx->withdrawals_count;

                uint8_t strategy =
                    (strategy_index != UNKNOWN_STRATEGY) ? strategy_index : UNKNOWN_STRATEGY;

                if (withdrawal >= 16 || strategy >= 16) {
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }

                tx->strategies[tx->strategies_count] = (withdrawal << 4) | (strategy & 0x0F);
                ;
            }

            // we just processed one strategy item
            tx->strategies_count += 1;
            if (tx->current_item_count > 0) {
                tx->current_item_count -= 1;
            }
            if (tx->current_item_count == 0) {
                // when we arrive at the end of the strategies array we go to the shares
                // array
                context->next_param = SHARES_SIZE;
            }
            break;
        }
        case SHARES_SIZE:

            if (tx->cached_offset != msg->parameterOffset) {
                PRINTF("Unexpected shares parameter offset %d != %d\n",
                       tx->cached_offset,
                       msg->parameterOffset);
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            if (!U2BE_from_parameter(msg->parameter, &tx->current_item_count)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            if (tx->current_item_count == 0 && tx->parent_item_count > 0) {
                // shares array is empty AND we have other queuedWithdrawals to parse
                tx->parent_item_count -= 1;
                tx->withdrawals_count += 1;
                context->next_param = STAKER;
            } else {
                context->next_param = SHARE;
            }
            break;
        case SHARE:
            // we don't need to parse shares items as they are not needed for clearsigning
            // as not having ETH / ERC20 amount to display would confuse users
            if (tx->current_item_count > 0) {
                tx->current_item_count -= 1;
            }
            if (tx->current_item_count == 0) {
                // we arrive at the end of the Withdrawal struct
                tx->parent_item_count -= 1;
                tx->withdrawals_count += 1;
                if (tx->parent_item_count == 0) {
                    // we arrive at the end of the Withdrawals array
                    context->next_param = TOKEN_SIZE;
                } else {
                    // we have other Withdrawals to parse
                    context->next_param = STAKER;
                }
            }
            break;
        case TOKEN_SIZE:

            if (tx->tokens_offset != msg->parameterOffset - SELECTOR_SIZE) {
                PRINTF("Unexpected tokens parameter offset %d != %d\n",
                       tx->tokens_offset,
                       msg->parameterOffset);
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            if (!U2BE_from_parameter(msg->parameter, &tx->parent_item_count)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            if (tx->parent_item_count > tx->withdrawals_count) {
                PRINTF("Unexpected number of tokens, %d > withdrawals %d\n",
                       tx->parent_item_count,
                       tx->withdrawals_count);
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            tx->current_item_count = tx->parent_item_count;
            // reset checksums and cached offset
            memset(&tx->withdrawals_offsets_checksum_preview,
                   0,
                   sizeof(tx->withdrawals_offsets_checksum_preview));
            memset(&tx->withdrawals_offsets_checksum_value,
                   0,
                   sizeof(tx->withdrawals_offsets_checksum_value));
            tx->cached_offset = 0;

            if (tx->parent_item_count == 0) {
                context->next_param = MIDDLEWARE_TIMES_SIZE;
            } else {
                context->next_param = TOKEN_OFFSET_ITEMS;
            }
            break;
        case TOKEN_OFFSET_ITEMS: {
            {
                // We have limited size on the context and can't store all the offset values
                // of the tokens structs. So we compute their checksum and expect to
                // be able to recompute it using the offset of the parsed items later.
                // _preview will be equal to _value at the end of the parsing if everything is
                // fine
                checksum_offset_params_t h_params;
                memset(&h_params, 0, sizeof(h_params));
                memcpy(&h_params.prev_checksum,
                       &(tx->withdrawals_offsets_checksum_preview),
                       sizeof(h_params.prev_checksum));
                // we hash the previous checksum with the offset we receive.
                uint16_t offset;
                if (!U2BE_from_parameter(msg->parameter, &offset)) {
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
                // if we are on the first element of the array, we save the offset, which all
                // received offset values will be based on for checksum computation
                if (tx->cached_offset == 0) {
                    tx->cached_offset = msg->parameterOffset;
                }

                h_params.new_offset = offset + tx->cached_offset;

                if (cx_keccak_256_hash((void *) &h_params,
                                       sizeof(h_params),
                                       tx->withdrawals_offsets_checksum_preview) != CX_OK) {
                    PRINTF("unable to compute keccak hash\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
            }

            if (tx->current_item_count > 0) {
                tx->current_item_count -= 1;
            }
            if (tx->current_item_count == 0) {
                context->next_param = TOKENS_ITEM_SIZE;
            }
            break;
        }
        case TOKENS_ITEM_SIZE: {
            {
                checksum_offset_params_t h_params;
                memset(&h_params, 0, sizeof(h_params));
                memcpy(&h_params.prev_checksum,
                       &(tx->withdrawals_offsets_checksum_value),
                       sizeof(h_params.prev_checksum));
                // we hash the previous checksum with the offset of the beginning of the
                // structure.
                h_params.new_offset = msg->parameterOffset;

                if (cx_keccak_256_hash((void *) &h_params,
                                       sizeof(h_params),
                                       tx->withdrawals_offsets_checksum_value) != CX_OK) {
                    PRINTF("unable to compute keccak hash\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
            }

            if (!U2BE_from_parameter(msg->parameter, &tx->current_item_count)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            if (tx->parent_item_count == 1) {
                // if we are on the last item of the array of tokens struct
                // we can check the checksum
                if (memcmp(tx->withdrawals_offsets_checksum_preview,
                           tx->withdrawals_offsets_checksum_value,
                           sizeof(tx->withdrawals_offsets_checksum_preview)) != 0) {
                    PRINTF("Checksums do not match\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
            }

            if (tx->current_item_count == 0) {
                context->next_param = MIDDLEWARE_TIMES_SIZE;
            } else {
                context->next_param = TOKENS_ITEM_ELEMENT;
            }

            break;
        }
        case TOKENS_ITEM_ELEMENT: {
            {
                uint8_t buffer[ADDRESS_LENGTH];
                copy_address(buffer, msg->parameter, sizeof(buffer));
                char address_buffer[ADDRESS_STR_LEN];
                if (!getEthDisplayableAddress(buffer, address_buffer, sizeof(address_buffer), 0)) {
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }

                uint8_t token_index = decode_token(address_buffer);
                // we check if the token matches the corresponding strategy
                uint8_t strategy_index = tx->strategies[tx->tokens_count] & 0x0F;
                if (strategy_index != UNKNOWN_STRATEGY && token_index != strategy_index) {
                    PRINTF("Token idx %d does not match strategy idx %d\n",
                           token_index,
                           strategy_index);
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }

                tx->tokens_count += 1;
            }

            if (tx->current_item_count > 0) {
                tx->current_item_count -= 1;
            }
            if (tx->current_item_count == 0) {
                // we arrive at the end of the tokens array
                tx->parent_item_count -= 1;
                if (tx->parent_item_count == 0) {
                    // if we don't have other Tokens to parse
                    context->next_param = MIDDLEWARE_TIMES_SIZE;
                } else {
                    // if we have other Tokens to parse
                    context->next_param = TOKEN_SIZE;
                }
            }
            break;
        }
        case MIDDLEWARE_TIMES_SIZE:

            if (tx->middlewareTimesIndexes_offset != msg->parameterOffset - SELECTOR_SIZE) {
                PRINTF("Unexpected middlewareTimesIndexes parameter offset %d != %d\n",
                       tx->middlewareTimesIndexes_offset,
                       msg->parameterOffset);
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            if (!U2BE_from_parameter(msg->parameter, &tx->current_item_count)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            if (tx->current_item_count > tx->withdrawals_count) {
                PRINTF("Unexpected middlewareTimesIndexes parameter length %d > withdrawals %d\n ",
                       tx->current_item_count,
                       msg->parameterOffset);
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            if (tx->current_item_count == 0) {
                context->next_param = RECEIVE_AS_TOKEN_SIZE;
            } else {
                context->next_param = MIDDLEWARE_TIMES_ITEM;
            }
            break;
        case MIDDLEWARE_TIMES_ITEM:
            if (tx->current_item_count > 0) {
                tx->current_item_count -= 1;
            }
            if (tx->current_item_count == 0) {
                context->next_param = RECEIVE_AS_TOKEN_SIZE;
            }
            break;
        case RECEIVE_AS_TOKEN_SIZE:

            if (tx->receiveAsTokens_offset != msg->parameterOffset - SELECTOR_SIZE) {
                PRINTF("Unexpected receiveAsTokens parameter offset %d != %d\n",
                       tx->receiveAsTokens_offset,
                       msg->parameterOffset);
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            if (!U2BE_from_parameter(msg->parameter, &tx->current_item_count)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            if (tx->current_item_count > tx->withdrawals_count) {
                PRINTF("Unexpected receiveAsTokens length %d > withdrawals length %d\n",
                       tx->current_item_count,
                       tx->withdrawals_count);
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            // we save the number of redelegations to parse
            tx->relegations_count = tx->current_item_count;

            if (tx->current_item_count == 0) {
                // reached the end
                context->next_param = NONE;
            } else {
                context->next_param = RECEIVE_AS_TOKEN_ITEM;
            }
            break;
        case RECEIVE_AS_TOKEN_ITEM: {
            if (tx->current_item_count > 0) {
                tx->current_item_count -= 1;
            }
            if (tx->current_item_count == 0) {
                // reached the end
                context->next_param = NONE;
            }
            break;
        }
        case NONE:
            break;
        default:
            PRINTF("Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

void handle_provide_parameter(ethPluginProvideParameter_t *msg) {
    context_t *context = (context_t *) msg->pluginContext;
    // We use `%.*H`: it's a utility function to print bytes. You first give
    // the number of bytes you wish to print (in this case, `PARAMETER_LENGTH`) and then
    // the address (here `msg->parameter`).
    PRINTF("plugin provide parameter: offset %d\nBytes: %.*H\n",
           msg->parameterOffset,
           PARAMETER_LENGTH,
           msg->parameter);

    msg->result = ETH_PLUGIN_RESULT_OK;

    switch (context->selectorIndex) {
        case DEPOSIT_INTO_STRATEGY:
            handle_deposit_into_strategy(msg, context);
            break;
        case UNDELEGATE:
            handle_undelegate(msg, context);
            break;
        case DELEGATE_TO:
            handle_delegate(msg, context);
            break;
        case QUEUE_WITHDRAWAL_PARAMS:
            handle_queue_withdrawal(msg, context);
            break;
        case COMPLETE_QUEUED_WITHDRAWALS:
            handle_complete_queued_withdrawals(msg, context);
            break;
        default:
            PRINTF("Selector Index not supported: %d\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}
