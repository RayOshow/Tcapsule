#pragma once

#define TEST_MODE_ON 1

#define CUSTOM_TOKEN_CONTRACT "binitkrtoken"_n
#define EOSIO_TOKEN_CONTRACT "eosio.token"_n

#define CONTRACT_NAME "time_capsule"

// Token transfer command
#define COMMAND_NAME_PUT_TOKEN "put"
#define COMMAND_NAME_REWARD_TOKEN "reward"

// Erorr code
#define ERROR_MSG_ID_ALREADY_EXIST "{\"CODE\":\"0001\",\"MSG\":\"ERROR_MSG_ID_ALREADY_EXIST\"}"
#define ERROR_MSG_ID_NOT_EXIST "{\"CODE\":\"0002\",\"MSG\":\"ERROR_MSG_ID_NOT_EXIST \"}"
#define ERROR_MSG_ID_NOT_MATCH "{\"CODE\":\"0003\",\"MSG\":\"ERROR_MSG_ID_NOT_MATCH \"}"
#define ERROR_MSG_INVALID_ASSET "{\"CODE\":\"0004\",\"MSG\":\"ERROR_MSG_INVALID_ASSET\"}"
#define ERROR_MSG_NO_ENOUGH_TOKEN "{\"CODE\":\"0005\",\"MSG\":\"ERROR_MSG_NO_ENOUGH_TOKEN\"}"


