#include "evmdeposit.hpp"

evmdeposit::evmdeposit(name self, name code, datastream<const char *> ds) : contract(self, code, ds), _accounts(self, self.value) {}

evmdeposit::~evmdeposit() {}

ACTION evmdeposit::addallow(eosio::name toadd)
{
    require_auth(get_self());
    eosio::check(is_account(toadd), "Account does not exist");
    config_table conf(get_self(), get_self().value);
    config cfg;
    if (conf.exists()) {
        cfg = conf.get();
        std::vector<eosio::name> allowed = cfg.allowed;
        auto it = allowed.begin();
        while (it != allowed.end()) {
            eosio::check(*it == toadd, "Account already allowed");
        }
        allowed.emplace_back(toadd);
    } else {
        std::vector<eosio::name> allowed = { toadd };
        cfg = {
            allowed
        };
    }
    conf.set(cfg, get_self());
}

ACTION evmdeposit::delallow(eosio::name todel)
{
    require_auth(get_self());
    config_table conf(get_self(), get_self().value);
    eosio::check(conf.exists(), "No conf table to del from");
    auto config = conf.get();
    std::vector<eosio::name> allowed = config.allowed;
    auto it = allowed.begin();
    bool found = false;
    while (it != allowed.end()) {
        if (*it == todel) {
            it = allowed.erase(it);
            found = true;
        } else {
            ++it;
        }
    }
    check(found, "The todel account was not found");
    conf.set(config, get_self());
}

void evmdeposit::ontransfer(name from, name to, asset quantity, string memo)
{
    name rec = get_first_receiver();

    if (rec == CORE_SYM_ACCOUNT && from != get_self() && to == get_self() && quantity.symbol == CORE_SYM)
    {
        config_table conf(get_self(), get_self().value);
        eosio::check(conf.exists(), "No conf table to check the allowlist");
        auto config = conf.get();
        std::vector<eosio::name> allowed = config.allowed;
        auto it = allowed.begin();
        bool found = false;
        while (it != allowed.end()) {
            if (*it == from) {
                found = true;
                break;
            } else {
                ++it;
            }
        }
        check(found, "The sending account is not allowed to use this contract");
        check(!memo.empty(), "EVM Address is empty");
        check(memo.length() == 42 && memo.substr(0, 2) == "0x", "EVM Address should be exactly 42 characters and begin with 0x");

        auto accounts_byaddress = _accounts.get_index<eosio::name("byaddress")>();
        auto address_256 = eosio_evm::pad160(eosio_evm::toChecksum160(memo.substr(2,42)));
        auto account = accounts_byaddress.find(address_256);
        if (account == accounts_byaddress.end()) {
            action(permission_level{get_self(), name("active")}, EVM_CONTRACT, name("openwallet"),
                   make_tuple(get_self(), memo))
                    .send();
        }


        action(permission_level{get_self(), name("active")}, CORE_SYM_ACCOUNT, name("transfer"),
               make_tuple(get_self(),
                          EVM_CONTRACT,
                          quantity,
                          memo))
                .send();
    }
}
