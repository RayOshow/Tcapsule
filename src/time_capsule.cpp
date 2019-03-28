#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/name.hpp>
#include <eosiolib/dispatcher.hpp>
#include <vector>
#include <eosiolib/crypto.hpp>
#include <eosiolib/transaction.hpp>

using eosio::asset;
using eosio::permission_level;
using eosio::action;
using eosio::print;
using eosio::name;
using std::string;
using std::vector;
using eosio::datastream;
using eosio::checksum256;
using eosio::transaction;
using eosio::indexed_by;
using eosio::const_mem_fun;


// header
#include "time_capsule.hpp"
#include "capsule/capsule_define.hpp"
#include "reward/reward_define.hpp"

// table
#include "table/account_table.hpp"
#include "table/capsule_table.hpp"
#include "table/reward_table.hpp"

// account
#include "account/account_control.hpp"

// capsule
#include "capsule/capsule_control.hpp"

// reward
#include "reward/reward_control.hpp"

/**
 @class time_capsule
 @last update date 2019/03/28
 @Author Ray-OShow(raymond@todos.co.kr)
 @brief  
**/

class [[eosio::contract]] time_capsule : public eosio::contract {

	public:

		using contract::contract;
		
		time_capsule(name receiver, name code,  datastream<const char*> ds)
			: contract(receiver, code, ds)  
			, account_controller(_self)
			, capsule_controller(_self)
			, reward_controller(_self) {
		}

		/**
			Add user info. 
		**/
		[[eosio::action]]
		void signup(uint32_t user_seq){	 			
			account_controller.signup(user_seq, reward_controller.get_rewards(REWARD_TYPE_SIGNUP));
		}

		/**
			Put tokens into internal accounts. 
		**/
		[[eosio::action]]
		void transfer(name from, name to, asset quantity, string memo) {

			eosio_assert(quantity.is_valid(), ERROR_MSG_INVALID_ASSET);	

			// Recieve only
			if(from == _self) {
				return;
			}

			// [memo]
			// command-data
			string command;
			string data;

			/////////////////////////////////////////////////////////////////
			const size_t first_break = memo.find("-");

			// Just send
			if(first_break == string::npos){
				return;
			}

			command = memo.substr(0, first_break);
			data = memo.substr(first_break + 1);
			/////////////////////////////////////////////////////////////////

			if(command.empty()) {
				return;
			}

      // put-322
      // [command]-[user seq]
			if(command == COMMAND_NAME_PUT_TOKEN) {
				account_controller.charge_token(stoull(data, 0, 10), quantity);		
			}	
			// reward-1-1
			// [command]-[reward type]-[reward seq]
			else if(command == COMMAND_NAME_REWARD_TOKEN) {
				const size_t second_break = data.find("-");
				uint32_t reward_type = stoull(data.substr(0, second_break), 0, 10);
				uint32_t reward_seq = stoull(data.substr(second_break + 1),0,10);

				reward_controller.put_reward_pool(reward_type,reward_seq,quantity);
			}
		}

  
		/**
			When user want to refund.
		**/
		[[eosio::action]]
		void refund(uint32_t user_seq, asset cost, string memo) {
			account_controller.charge_token(user_seq, cost);
		}

		/**
			When user use their tokens for purchaing something.
		**/
		[[eosio::action]]
		void consume(uint32_t user_seq, asset cost, string memo) {
			account_controller.uncharge_token(user_seq, cost);	
		}
		
		/**
			Set blockcahin id.
		**/
		[[eosio::action]]
		void setbcid(name blockchain_id, uint32_t user_seq) {
			account_controller.set_blockchain_id(blockchain_id, user_seq);		
		}

		/**
			User can claim and get his or her tokens.
		**/
		[[eosio::action]]
		void withdraw(name requestor, uint32_t user_seq, asset quantity) {
			account_controller.withdraw_token(requestor,user_seq,quantity);
			send_token(_self, requestor, quantity, "withdraw " );						
		}

		/**
			Erase user data 
		**/
		[[eosio::action]]
		void eraseid(uint32_t user_seq) {
			account_controller.retire(user_seq);		
		}

		/**
			Register capsule data.
		**/
		[[eosio::action]]			
		void regcap(uint32_t capsule_seq, uint32_t user_seq,string encrypt_data, uint32_t refresh_period) {
			account_controller.check_user_exist(user_seq);
			capsule_controller.register_capsule(capsule_seq,user_seq, encrypt_data, refresh_period);
		}

		/**
			Refresh capsule data.
		**/
		[[eosio::action]]			
		void refcap(uint32_t capsule_seq) {
			capsule_controller.refresh_capsule(capsule_seq);
		}

		/**
			Finish capsule data.
		**/
		[[eosio::action]]
		void  fincap(uint32_t capsule_seq) {
			capsule_controller.finish_capsule(capsule_seq);
		}

		/**
			Delete capsule data.
		**/
		[[eosio::action]]
		void erasecap(uint32_t capsule_seq) {
			capsule_controller.delete_info(capsule_seq);
		}

		/**
			Set reward policy
		**/
		[[eosio::action]]
		void setrwdp(uint32_t reward_type, uint32_t reward_seq, string reward_name, asset quantity, uint8_t status, uint32_t limit_count, uint32_t limit_time) {
			reward_controller.set_reward_policy(reward_type, reward_seq, reward_name, quantity, status , limit_count, limit_time);
		}		

		/**
			erase reward
		**/
		[[eosio::action]]
		void eraserwd(uint32_t seq) {
			reward_controller.delete_reward(seq);
		}

	private:

		account_control account_controller;
		capsule_control capsule_controller;
		reward_control reward_controller;

		/**
			Send token by eosio.token contract outside.
			It is essential to have "eosio.code" permission.
		**/
		void send_token(name user1, name user2, asset quantity, string memo) {
		  
		  name token_contract;
		  
		  if(quantity.symbol.code().to_string() == "EOS") {
		    token_contract = EOSIO_TOKEN_CONTRACT;
		  }
		  else {
		    token_contract = CUSTOM_TOKEN_CONTRACT;
		  }
		  
			action(
				permission_level{_self, "active"_n},
				token_contract, 
				"transfer"_n,
				std::make_tuple(
					user1, 
					user2, 
					quantity, 
					memo)

			).send();
		}		
};

#define EOSIO_DISPATCH_CUSTOM( TYPE, MEMBERS ) \
extern "C" { \
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) { \
	if(code == "todoskrtoken"_n.value && action == "transfer"_n.value) {\
            eosio::execute_action(name(receiver), name(code), &time_capsule::transfer);\
        }\
  else if(code == "eosio.token"_n.value && action == "transfer"_n.value) {\
            eosio::execute_action(name(receiver), name(code), &time_capsule::transfer);\
        }\
	else {\
		if( code == receiver ) { \
	     		switch (action) {\
               			EOSIO_DISPATCH_HELPER(TYPE, MEMBERS)\
        	    	}\
		}\
	}\
   } \
} \

EOSIO_DISPATCH_CUSTOM( time_capsule, (signup)(transfer)(withdraw)(eraseid)(setbcid)(consume)(refund)(regcap)(refcap)(fincap)(erasecap)(setrwdp)(eraserwd))
