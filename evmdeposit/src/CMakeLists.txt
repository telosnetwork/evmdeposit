project(evmdeposit)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( evmdeposit evmdeposit evmdeposit.cpp )
target_include_directories( evmdeposit PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_include_directories( evmdeposit PUBLIC ${CMAKE_SOURCE_DIR}/../external )
target_ricardian_directory( evmdeposit ${CMAKE_SOURCE_DIR}/../ricardian )