/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <booking.hpp>

namespace booking {

void CreateOffer::checkAuth(Hotel & initiator) {
    require_auth(initiator.owner);
}

void CreateOffer::onApply(Hotel & initiator) {
    assert(roomInfo.get_size() > 0, "empty roomInfo");
    assert(arrivalDate > now(), "arrivalDate should be > now time");
    assert((bool)price, "price should be > 0");

    Offer newOffer;
    newOffer.roomInfo = roomInfo;
    newOffer.arrivalDate = arrivalDate;
    newOffer.price = price;

    newOffer.id = { (HotelId)initiator.id, initiator.totalOffers++ };
    initiator.openOffers++;

    Offers::store(newOffer);
    Hotels::update(initiator);

    eosio::print("CreateOffer: ", newOffer);
}


void CreateReq::checkAuth(Hotel & initiator) {
    require_auth(initiator.owner);
}

void CreateReq::onApply(Hotel & initiator) {
    Offer targetOffer;
    assert(Offers::get(offerId, targetOffer), "offer not found");

    assert(initiator.balance >= targetOffer.price, "not enough balance");

    Request newRequest;
    newRequest.offerId = targetOffer.id;
    newRequest.pubKey = pubKey;

    newRequest.id = { (HotelId)initiator.id, initiator.totalRequests++ };
    initiator.openRequests++;

    Requests::store(newRequest);

    initiator.balance -= targetOffer.price;
    Hotels::update(initiator);

    eosio::print("CreateReq: ", newRequest);
}


void ChargeReq::checkAuth(Hotel & initiator) {
    require_auth(initiator.owner);
}

void ChargeReq::onApply(Hotel & initiator) {
    Request targetReq;
    assert(Requests::get(requestId, targetReq), "request not found");
    assert(!targetReq.charged, "request already charged");

    Offer assigneeOffer;
    assert(Offers::get(targetReq.offerId, assigneeOffer), "offer not found");

    assert(initiator.id == (HotelIdI64)assigneeOffer.id.hotelId, "only assignee offer owner can charge request");

    targetReq.chargeData = chargeData;
    targetReq.charged = true;

    Requests::update(targetReq);

    initiator.balance.quantity += assigneeOffer.price.quantity;
    Hotels::update(initiator);
}


void RefundReq::checkAuth(Hotel & initiator) {
    require_auth(initiator.owner);
}

void RefundReq::onApply(Hotel & initiator) {
    Request targetReq;
    assert(Requests::get(requestId, targetReq), "request not found");

    Offer assigneeOffer;
    assert(Offers::get(targetReq.offerId, assigneeOffer), "offer not found");


    assert ((HotelId)initiator.id == targetReq.id.hotelId, "request can refunded only by creator");

    Requests::remove(targetReq);

    if (!targetReq.charged) {
        initiator.balance.quantity += assigneeOffer.price.quantity;
        initiator.openRequests--;
        Hotels::update(initiator);
    }

    Hotel offerCreator;
    Hotels::get((HotelIdI64)assigneeOffer.id.hotelId, offerCreator);

    offerCreator.openOffers--;
    Hotels::update(offerCreator);
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

        booking::Hotel hotel1 = { 1, N(inita), booking::Token(static_cast<uint64_t>(100)) };
        booking::Hotel hotel2 = { 2, N(initb), booking::Token(static_cast<uint64_t>(100)) };

        booking::Hotels::store(hotel1);
        booking::Hotels::store(hotel2);

        eosio::print( "Init Booking!\n" );
    }

    /// The apply method implements the dispatch of events to this contract
    void apply( uint64_t code, uint64_t action ) {
        if (code == N(booking)) {
            switch (action) {
            case N(CreateOffer):
                eosio::current_message<booking::CreateOffer>().apply();
                break;
            case N(CreateReq):
                eosio::current_message<booking::CreateReq>().apply();
                break;
            case N(ChargeReq):
                eosio::current_message<booking::ChargeReq>().apply();
                break;
            case N(RefundReq):
                eosio::current_message<booking::RefundReq>().apply();
                break;
            default:
                eosio::print("unknown action: ", eosio::name(action));
            }
        }
        eosio::print( "apply: ", eosio::name(code), "->", eosio::name(action), "\n" );
    }

} // extern "C"
