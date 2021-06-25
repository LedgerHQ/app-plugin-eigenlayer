#include "plugin.h"

// Sets the first screen to display.
void handle_query_contract_id(ethQueryContractID_t *msg) {
    const context_t *context = (const context_t *) msg->pluginContext;
    // msg->name will be the upper sentence displayed on the screen.
    // msg->version will be the lower sentence displayed on the screen.

    // For the first screen, display the plugin name.
    strlcpy(msg->name, APPNAME, msg->nameLength);

    switch (context->selectorIndex) {
        case DEPOSIT_INTO_STRATEGY:
            strlcpy(msg->version, "Deposit into Strategy", msg->versionLength);
            msg->result = ETH_PLUGIN_RESULT_OK;
            break;
        case DELEGATE_TO:
            strlcpy(msg->version, "Delegate to", msg->versionLength);
            msg->result = ETH_PLUGIN_RESULT_OK;
            break;
        case UNDELEGATE:
            strlcpy(msg->version, "Undelegate", msg->versionLength);
            msg->result = ETH_PLUGIN_RESULT_OK;
            break;
        case QUEUE_WITHDRAWAL_PARAMS:
            strlcpy(msg->version, "Queued Withdrawal", msg->versionLength);
            msg->result = ETH_PLUGIN_RESULT_OK;
            break;
        case COMPLETE_QUEUED_WITHDRAWALS:
            strlcpy(msg->version, "Complete Queued Withdrawals", msg->versionLength);
            msg->result = ETH_PLUGIN_RESULT_OK;
            break;
        default:
            PRINTF("Selector index: %d not supported\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }
}
