#pragma once

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] capsule {
	uint32_t capsule_seq;
	uint32_t user_seq;
	string encrypt_data;  
	uint32_t last_refresh_time;
	uint32_t refresh_period;	
	uint32_t register_time;
	uint8_t end_flag;
	
	uint64_t primary_key() const { return capsule_seq; }
	uint64_t get_secondary_1() const { return user_seq; }
	
	EOSLIB_SERIALIZE(
		capsule,
	    (capsule_seq)
	    (user_seq)
	    (encrypt_data)
	    (last_refresh_time)
	    (refresh_period)
	    (register_time)
		(end_flag)
	)    
};

typedef eosio::multi_index< "capsule"_n, capsule ,eosio::indexed_by<"byuser"_n, eosio::const_mem_fun<capsule, uint64_t, &capsule::get_secondary_1>>> capsules;
