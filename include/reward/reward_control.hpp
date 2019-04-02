#pragma once

class reward_control {

private:
	name self;

public:
	reward_control(name _self) : self(_self) {
	}
	
	/**
		Put reward tokens to pool.
	**/
	void put_reward_pool(uint32_t reward_type, uint32_t reward_seq, asset quantity){
		
		rewards rws(self, self.value);
		auto it = rws.find(reward_type);

		// Add first reward policy.
		if(it == rws.end()) {

			rws.emplace( self, [&]( auto& row ){
				row.reward_type = reward_type;
		
				rwpolicy rwp;
				rwp.reward_seq = reward_seq;
				rwp.status = REWARD_STATUS_RESERVE;
				rwp.total_token_amount = quantity;
				rwp.used_count = 0;
				rwp.used_amount = 0;
				row.reward_policy.push_back(rwp);
			});
		}
		// Add subsequent policy.
		else {

			rws.modify(it, self, [&]( auto& row ) {
				
				for(int index = 0; index < row.reward_policy.size(); index++) {
					// If seq already exists,                            
					if(row.reward_policy[index].reward_seq == reward_seq) {
						row.reward_policy[index].total_token_amount += quantity;
						return;
					}
				}

				rwpolicy rwp;
				rwp.reward_seq = reward_seq;
				rwp.status = REWARD_STATUS_RESERVE;
				rwp.total_token_amount = quantity;
				rwp.used_count = 0;
				rwp.used_amount = 0;
				row.reward_policy.push_back(rwp);
			});
		}
	}
	
	/**
	**/
	void set_reward_policy(uint32_t reward_type, uint32_t reward_seq, string reward_name, asset quantity, uint8_t status, uint32_t limit_count, uint32_t limit_time) {
	
		require_auth( self );
		
		rewards rws(self, self.value);
		auto it = rws.find(reward_type);
		eosio_assert( it != rws.end(),ERROR_MSG_REWARD_NOT_EXIST);
		eosio_assert( reward_name.length() <= REWARD_MAX_NAME_LEN, ERROR_MSG_MAX_REWARD_NAME);
		

		rws.modify(it, self, [&]( auto& row ) {	

			int index;
			
			for(index = 0; index < row.reward_policy.size(); index++) {

				if(row.reward_policy[index].reward_seq == reward_seq) {

					eosio_assert( row.reward_policy[index].total_token_amount.amount > quantity.amount, ERROR_MSG_INVALID_REWARD_AMOUNT);
					eosio_assert( row.reward_policy[index].total_token_amount.symbol.code() == quantity.symbol.code(), ERROR_MSG_INVALID_REWARD_TOKEN);					
					eosio_assert( status < REWARD_TYPE_MAX, ERROR_MSG_INVALID_REWARD_TYPE);
 
 					row.reward_policy[index].reward_name = reward_name;	
					row.reward_policy[index].reward_amount = quantity.amount;	// per one 
					row.reward_policy[index].status = status;
					row.reward_policy[index].limit_count = limit_count;
					row.reward_policy[index].limit_time = limit_time;
					break;
				}
			}				

			eosio_assert( index != row.reward_policy.size(), ERROR_MSG_REWARD_NOT_EXIST);				
		});				
	}
	

	/**
		It's common func for getting reward.
	**/
	void delete_reward(uint32_t seq) {
		
		require_auth(self);

		rewards rwd(self, self.value);
		auto it = rwd.find( seq );
		eosio_assert( it != rwd.end(), ERROR_MSG_REWARD_NOT_EXIST);

		rwd.erase(it);
	}
	
	/**
		It's common func for getting reward.
	**/
	asset get_rewards(uint32_t reward_type) {

		asset reward_token ;
		reward_token.amount = 0;
		
		rewards rws(self, self.value);
		auto it = rws.find(reward_type);

		// if there is no reward type policy , reward will be zero.
		if(it == rws.end()) {
			return reward_token;
		}		

		rws.modify(it, self, [&]( auto& row ) {	
			for(int index = 0; index < row.reward_policy.size(); index++) {
				if(row.reward_policy[index].status == REWARD_STATUS_USE) {

					// Check if time is over.
					if((row.reward_policy[index].limit_time > 0) && (now() >= row.reward_policy[index].limit_time)) {
						row.reward_policy[index].status = REWARD_STATUS_CLOSE;
						continue;
					}

					// Check if count it over.
					if((row.reward_policy[index].used_count > 0) && (row.reward_policy[index].used_count >= row.reward_policy[index].limit_count)) {
						row.reward_policy[index].status = REWARD_STATUS_CLOSE;
						continue;
					}

					// Update data
					reward_token = row.reward_policy[index].total_token_amount;
					reward_token.amount = row.reward_policy[index].reward_amount;
					

					row.reward_policy[index].used_amount += reward_token.amount;
					row.reward_policy[index].used_count++;	
					
					// Check this policy is ended. 	
					if((row.reward_policy[index].total_token_amount.amount <= row.reward_policy[index].used_amount) ||
						(row.reward_policy[index].total_token_amount.amount - row.reward_policy[index].used_amount < row.reward_policy[index].reward_amount)) {
						row.reward_policy[index].status = REWARD_STATUS_CLOSE;
					}

					break;
				}
			}	
		});

		return reward_token;
	}
};

