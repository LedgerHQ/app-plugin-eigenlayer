#include "plugin.h"

void handle_finalize(ethPluginFinalize_t *msg) {
    context_t *context = (context_t *) msg->pluginContext;

    msg->uiType = ETH_UI_TYPE_GENERIC;

    // The total number of screen you will need.
    switch (context->selectorIndex) {
        case DEPOSIT_INTO_STRATEGY:
            msg->numScreens = 2;
            break;
        case UNDELEGATE:
            msg->numScreens = 1;
            break;
        case DELEGATE_TO:
            msg->numScreens = 1;
            break;
        case QUEUE_WITHDRAWAL_PARAMS:
            // withdrawer
            msg->numScreens = 1;
            msg->numScreens += context->tx.queue_withdrawal.strategies_count;
            break;
        case COMPLETE_QUEUED_WITHDRAWALS:
            // withdrawer
            msg->numScreens = 1;
            msg->numScreens += context->tx.complete_queued_withdrawals.strategies_count;
            break;
        default:
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }

    msg->result = ETH_PLUGIN_RESULT_OK;
}
