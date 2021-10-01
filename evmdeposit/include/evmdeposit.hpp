#include <intx/base.hpp>

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <eosio/crypto.hpp>
#include <eosio/symbol.hpp>

#include "util.hpp"
#include "datastream.hpp"


using namespace std;
using namespace eosio;

CONTRACT evmdeposit : public contract
{

public:
evmdeposit(name self, name code, datastream<const char *> ds);

~evmdeposit();

static constexpr name CORE_SYM_ACCOUNT = name("eosio.token");
static constexpr symbol CORE_SYM = symbol("TLOS", 4);
static constexpr name EVM_CONTRACT = name("eosio.evm");

ACTION addallow(eosio::name toadd);
ACTION delallow(eosio::name toadd);

[[eosio::on_notify("eosio.token::transfer")]] void ontransfer(name from, name to, asset quantity, string memo);

TABLE config {
    std::vector<eosio::name> allowed;
    EOSLIB_SERIALIZE(config, (allowed))
};

struct [[eosio::table, eosio::contract("eosio.evm")]] Account {
    uint64_t index;
    eosio::checksum160 address;
    eosio::name account;
    uint64_t nonce;
    std::vector<uint8_t> code;
    eosio::checksum256 balance;

    Account () = default;
    Account (uint256_t _address): address(eosio_evm::addressToChecksum160(_address)) {}
    uint64_t primary_key() const { return index; };

    uint64_t get_account_value() const { return account.value; };
    uint256_t get_address() const { return eosio_evm::checksum160ToAddress(address); };

    // TODO: make this work if we need to lookup EVM balances, which we don't for this contract
    //uint256_t get_balance() const { return balance; };
    //bool is_empty() const { return nonce == 0 && balance == 0 && code.size() == 0; };
    uint64_t get_nonce() const { return nonce; };
    std::vector<uint8_t> get_code() const { return code; };

    eosio::checksum256 by_address() const { return eosio_evm::pad160(address); };

    EOSLIB_SERIALIZE(Account, (index)(address)(account)(nonce)(code)(balance));
};

typedef eosio::multi_index<"account"_n, Account,
        eosio::indexed_by<eosio::name("byaddress"), eosio::const_mem_fun<Account, eosio::checksum256, &Account::by_address>>,
eosio::indexed_by<eosio::name("byaccount"), eosio::const_mem_fun<Account, uint64_t, &Account::get_account_value>>
> account_table;

typedef eosio::singleton<"config"_n, config> config_table;

account_table _accounts;

};