/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <booking.hpp>

namespace booking {

void CreateOffer::apply() {
}

void CreateReq::apply() {
}

void ChargeReq::apply() {
}

void RefundReq::apply() {
}

} //namespace booking


/**
 *  The init() and apply() methods must have C calling convention so that the blockchain can lookup and
 *  call these methods.
 */

extern "C" {

    /**
     *  This method is called once when the contract is published or updated.
     */
    void init()  {
        eosio::print( "Init Booking!\n" );
    }

    /// The apply method implements the dispatch of events to this contract
    void apply( uint64_t code, uint64_t action ) {
        if (code == N(booking)) {
            switch (action) {
            case N(create_offer):
                eosio::current_message<booking::CreateOffer>().apply();
                break;
            case N(create_request):
                eosio::current_message<booking::CreateReq>().apply();
                break;
            case N(charge_request):
                eosio::current_message<booking::ChargeReq>().apply();
                break;
            case N(refund_request):
                eosio::current_message<booking::RefundReq>().apply();
                break;
            }
        }
        eosio::print( "apply: ", eosio::name(code), "->", eosio::name(action), "\n" );
    }

} // extern "C"
