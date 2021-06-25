#include "plugin.h"

// Set UI for any address screen.
static bool set_address_ui(ethQueryContractUI_t *msg, address_t *value) {
    // Prefix the address with `0x`.
    msg->msg[0] = '0';
    msg->msg[1] = 'x';

    // We need a random chainID for legacy reasons with `getEthAddressStringFromBinary`.
    // Setting it to `0` will make it work with every chainID :)
    uint64_t chainid = 0;

    // Get the string representation of the address stored in `context->beneficiary`. Put it in
    // `msg->msg`.
    return getEthAddressStringFromBinary(
        value->value,
        msg->msg + 2,  // +2 here because we've already prefixed with '0x'.
        chainid);
}

static bool set_addr_ui(ethQueryContractUI_t *msg, address_t *address, const char *title) {
    strlcpy(msg->title, title, msg->titleLength);
    return set_address_ui(msg, address);
}

static bool handle_deposit_into_strategy(ethQueryContractUI_t *msg,
                                         context_t *context,
                                         uint8_t screenIndex) {
    switch (screenIndex) {
        case 0:
            strlcpy(msg->title, "Strategy", msg->titleLength);
            if (context->tx.deposit_into_strategy.strategy == UNKNOWN_STRATEGY) {
                strlcpy(msg->msg, "UNKNOWN", msg->msgLength);
            } else {
                strlcpy(msg->msg,
                        tickers[context->tx.deposit_into_strategy.strategy],
                        MAX_TICKER_LEN);
            }
            return true;
        case 1:
            strlcpy(msg->title, "Amount", msg->titleLength);
            amountToString(context->tx.deposit_into_strategy.amount.value,
                           sizeof(context->tx.deposit_into_strategy.amount.value),
                           ERC20_DECIMALS,
                           context->tx.deposit_into_strategy.token == UNKNOWN_TOKEN
                               ? "UNKNOWN"
                               : tickers[context->tx.deposit_into_strategy.token],
                           msg->msg,
                           msg->msgLength);
            return true;
        default:
            PRINTF("Received an invalid screenIndex\n");
            return false;
    }
}

static bool handle_queue_withdrawal(ethQueryContractUI_t *msg,
                                    context_t *context,
                                    uint8_t screenIndex) {
    queue_withdrawal_t *params = &context->tx.queue_withdrawal;

    switch (screenIndex) {
        case 0:
            strlcpy(msg->title, "Withdrawer", msg->titleLength);
            strlcpy(msg->msg, params->withdrawer, msg->msgLength);
            return true;
        default: {
            // removing the first screen to current screen index
            // to get the index of the withdrawal
            uint8_t withdrawal_index = msg->screenIndex - 1;

            if (withdrawal_index < params->strategies_count) {
                strlcpy(msg->title, "Strategy", msg->titleLength);
                uint8_t strategy = params->strategies[withdrawal_index];

                if (strategy == UNKNOWN_STRATEGY) {
                    strlcpy(msg->msg, "UNKNOWN", msg->msgLength);
                } else {
                    strlcpy(msg->msg, tickers[strategy], msg->msgLength);
                }
            }

            return true;
        }
            PRINTF("Received an invalid screenIndex\n");
            return false;
    }
}

static bool handle_complete_queued_withdrawals(ethQueryContractUI_t *msg,
                                               context_t *context,
                                               uint8_t screenIndex) {
    complete_queued_withdrawals_t *params = &context->tx.complete_queued_withdrawals;

    switch (screenIndex) {
        case 0:
            strlcpy(msg->title, "Withdrawer", msg->titleLength);
            char address_buffer[ADDRESS_STR_LEN];
            getEthDisplayableAddress(params->withdrawer, address_buffer, sizeof(address_buffer), 0);
            strlcpy(msg->msg, address_buffer, msg->msgLength);
            return true;
        default: {
            uint8_t strategy_index = msg->screenIndex - 1;

            if (strategy_index < params->strategies_count) {
                strlcpy(msg->title, "Strategy", msg->titleLength);
                uint8_t strategy = params->strategies[strategy_index];

                if (strategy == UNKNOWN_STRATEGY) {
                    strlcpy(msg->msg, "UNKNOWN", msg->msgLength);
                } else {
                    strlcpy(msg->msg, tickers[strategy], msg->msgLength);
                }
            }
            return true;
        }
            PRINTF("Received an invalid screenIndex\n");
            return false;
    }
}

void handle_query_contract_ui(ethQueryContractUI_t *msg) {
    context_t *context = (context_t *) msg->pluginContext;
    bool ret = false;

    // msg->title is the upper line displayed on the device.
    // msg->msg is the lower line displayed on the device.

    // Clean the display fields.
    memset(msg->title, 0, msg->titleLength);
    memset(msg->msg, 0, msg->msgLength);

    switch (context->selectorIndex) {
        case DEPOSIT_INTO_STRATEGY:
            ret = handle_deposit_into_strategy(msg, context, msg->screenIndex);
            break;
        case UNDELEGATE:
            ret = set_addr_ui(msg, &context->tx.undelegate.staker, "Staker");
            break;
        case DELEGATE_TO:
            ret = set_addr_ui(msg, &context->tx.delegate_to.operator, "Operator");
            break;
        case QUEUE_WITHDRAWAL_PARAMS:
            ret = handle_queue_withdrawal(msg, context, msg->screenIndex);
            break;
        case COMPLETE_QUEUED_WITHDRAWALS:
            ret = handle_complete_queued_withdrawals(msg, context, msg->screenIndex);
            break;
        default:
            PRINTF("Selector index: %d not supported\n", context->selectorIndex);
            ret = false;
    }
    msg->result = ret ? ETH_PLUGIN_RESULT_OK : ETH_PLUGIN_RESULT_ERROR;
}
