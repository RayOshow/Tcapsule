#pragma once

class capsule_control {

private:
	name self;
		
public:
	capsule_control(name _self) : self(_self) {
	}
	
	/**
		Register capsule data
	**/
	void register_capsule(uint32_t capsule_seq, uint32_t user_seq, string encrypt_data, uint32_t refresh_period) {
		
		require_auth(self);	
		eosio_assert(encrypt_data.length() < CAPSULE_MAX_DATA_LEN, CAPSULE_ERROR_MSG_OVER_MAX_DATA_LEN);
		
		capsules cap(self, self.value);
		auto it = cap.find(capsule_seq);
		eosio_assert(it == cap.end(),CAPSULE_ERROR_MSG_ALREADY_EXIST_SEQ);
		
		// Add capsule info to table.
		cap.emplace(self, [&]( auto& row ) {
			row.capsule_seq = capsule_seq;
			row.user_seq = user_seq;
			row.encrypt_data = encrypt_data;
			
			row.refresh_period = refresh_period;
			row.register_time = now();
			row.last_refresh_time = row.register_time;
			row.end_flag = 0;
		});		
	}
	
	/**
		Refresh 
	**/
	void refresh_capsule(uint32_t capsule_seq) {
		
		require_auth(self);	
		
		capsules cap(self, self.value);
		auto it = cap.find(capsule_seq);
		eosio_assert(it != cap.end(),CAPSULE_ERROR_MSG_NOT_EXIST_SEQ);
		
		cap.modify(it, self, [&]( auto& row ) {
			eosio_assert(row.end_flag < 1, CAPSULE_ERROR_MSG_ALREADY_END);
			row.last_refresh_time = now();			
		});
	}
	
	/**
		finish 
	**/
	void finish_capsule(uint32_t capsule_seq) {

		require_auth(self);

		capsules cap(self, self.value);
		auto it = cap.find(capsule_seq);
		eosio_assert(it != cap.end(),CAPSULE_ERROR_MSG_NOT_EXIST_SEQ);
		
		cap.modify(it, self, [&]( auto& row ) {
			eosio_assert(row.end_flag < 1, CAPSULE_ERROR_MSG_ALREADY_END);
			row.end_flag = 1;
		});
	}		

	void delete_info(uint32_t capsule_seq) 
	{
		require_auth(self);

		capsules cap(self, self.value);
		auto it = cap.find(capsule_seq);
		eosio_assert(it != cap.end(), CAPSULE_ERROR_MSG_NO_FIND_SEQ);
		cap.erase(it);
	}

};

