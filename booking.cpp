/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <booking.hpp>

namespace booking {

void createoffer::checkAuth(Account & initiator) {
    require_auth(initiator.owner);
}

void createoffer::onApply(Account & initiator) {
    assert(initiator.isHotel, "only for hotels");
    assert(arrivalDate > now(), "arrivalDate should be > now time");
    assert((bool)price, "price should be > 0");

    Offer newOffer;
    memcpy(&newOffer.roomInfo, &roomInfo, sizeof(roomInfo));

    newOffer.arrivalDate = arrivalDate;
    newOffer.price.quantity = price;

    newOffer.id = { (AccountId)initiator.id, initiator.totalOffers++ };
    initiator.openOffers++;

    Offers::store(newOffer);
    Accounts::update(initiator);

    eosio::print("createoffer: ", newOffer, "\n");
}


void createreq::checkAuth(Account & initiator) {
    require_auth(initiator.owner);
}

void createreq::onApply(Account & initiator) {
    Offer targetOffer;
    assert(Offers::get(offerId, targetOffer), "offer not found");

    assert(initiator.balance >= targetOffer.price, "not enough balance");

    Request newRequest;
    newRequest.offerId = targetOffer.id;
    newRequest.pubKey = { (char*)pubKey.data, sizeof(pubKey.data), true };

    newRequest.id = { (AccountId)initiator.id, initiator.totalRequests++ };
    initiator.openRequests++;

    Requests::store(newRequest);

    initiator.balance -= targetOffer.price;
    Accounts::update(initiator);

    eosio::print("createreq: ", newRequest, "\n");
}


void chargereq::checkAuth(Account & initiator) {
    require_auth(initiator.owner);
}

void chargereq::onApply(Account & initiator) {
    Request targetReq;
    assert(Requests::get(requestId, targetReq), "request not found");
    assert(!targetReq.charged, "request already charged");

    Offer assigneeOffer;
    assert(Offers::get(targetReq.offerId, assigneeOffer), "offer not found");

    assert(initiator.id == (AccountIdI64)assigneeOffer.id.accountId, "only assignee offer owner can charge request");

    targetReq.chargeData = chargeData;
    targetReq.charged = true;

    Requests::update(targetReq);

    initiator.balance.quantity += assigneeOffer.price.quantity;
    Accounts::update(initiator);
}


void refundreq::checkAuth(Account & initiator) {
    require_auth(initiator.owner);
}

void refundreq::onApply(Account & initiator) {
    Request targetReq;
    assert(Requests::get(requestId, targetReq), "request not found");

    Offer assigneeOffer;
    assert(Offers::get(targetReq.offerId, assigneeOffer), "offer not found");


    assert ((AccountId)initiator.id == targetReq.id.accountId, "request can refunded only by creator");

    Requests::remove(targetReq);

    if (!targetReq.charged) {
        initiator.balance.quantity += assigneeOffer.price.quantity;
        initiator.openRequests--;
        Accounts::update(initiator);
    }

    Account offerCreator;
    Accounts::get((AccountIdI64)assigneeOffer.id.accountId, offerCreator);

    offerCreator.openOffers--;
    Accounts::update(offerCreator);
}

} //namespace booking


/**
 *  The init() and apply() methods must have C calling convention so that the blockchain can lookup and
 *  call these methods.
 */

#define APPLY(message) {\
    booking::Account initiator; \
    assert(booking::Accounts::get((booking::AccountIdI64)message.initiatorId, initiator), "initiator account not found"); \
    message.checkAuth(initiator); \
    message.onApply(initiator); \
}

extern "C" {

    /**
     *  This method is called once when the contract is published or updated.
     */
    void init()  {

        booking::Account account1 = { 1, N(inita), booking::Token(static_cast<uint64_t>(100)), 1 };
        booking::Account account2 = { 2, N(initb), booking::Token(static_cast<uint64_t>(100)) };

        booking::Accounts::store(account1);
        booking::Accounts::store(account2);

        eosio::print( "Init Booking!\n" );
    }

    /// The apply method implements the dispatch of events to this contract
    void apply( uint64_t code, uint64_t action ) {
        if (code == N(booking)) {
            switch (action) {
            case N(createoffer):
                APPLY(eosio::current_message<booking::createoffer>());
                break;
            case N(createreq):
                APPLY(eosio::current_message<booking::createreq>());
                break;
            case N(chargereq):
                APPLY(eosio::current_message<booking::chargereq>());
                break;
            case N(refundreq):
                APPLY(eosio::current_message<booking::refundreq>());
                break;
            default:
                eosio::print("unknown action: ", eosio::name(action), "\n");
            }
        }
        eosio::print( "apply: ", eosio::name(code), "->", eosio::name(action), "\n" );
    }

} // extern "C"
