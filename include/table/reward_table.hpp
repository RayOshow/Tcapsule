#pragma once

struct rwpolicy {
	uint32_t reward_seq; // reward number
  string reward_name;
	// Status	
	uint8_t status;

	// limit info
	uint32_t limit_count;
 	uint32_t limit_time;

	// token info
	asset total_token_amount;	
	uint32_t used_count;
	uint64_t used_amount;	
	uint64_t reward_amount;	// per one reward

	uint32_t primary_key()const { return reward_seq;}	
};

// reward 
struct [[eosio::table , eosio::contract(CONTRACT_NAME)]] reward {
	uint32_t reward_type;	
	std::vector<rwpolicy> reward_policy; // Policies for reward
	uint32_t primary_key()const { return reward_type;}

	EOSLIB_SERIALIZE(
		reward,
	    (reward_type)
	    (reward_policy)
	)    	
};	

typedef eosio::multi_index< "reward"_n, reward > rewards;
