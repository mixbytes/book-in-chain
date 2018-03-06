/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <booking.hpp>

namespace booking {

void transfer::auth(Account & initiator)
{
    require_auth(initiator.owner);
}

void transfer::apply(Account & initiator)
{
    assert(initiator.balance >= Token(quantity), "need balance > quantity");

    Account toAcc;
    assert(Accounts::get((AccountIdI64)to, toAcc), "reciever not found");

    toAcc.balance += Token(quantity);
    initiator.balance -= Token(quantity);

    Accounts::store(toAcc);
    Accounts::store(initiator);
}

void sethotel::auth(Account & initiator)
{
    require_auth(initiator.owner);
    assert(initiator.owner == N(booking), "only booking can set hotel flag");
}

void sethotel::apply(Account & initiator)
{
    Account acc;
    assert(Accounts::get((AccountIdI64)id, acc), "account not found");
    assert(acc.isHotel != set, "account is alredy setted");

    acc.isHotel = (bool)set;
    Accounts::store(acc);
}

void newaccount::auth(Account & initiator)
{
    require_auth(initiator.owner);
    assert(initiator.owner == N(booking), "only booking can create account");
}

void newaccount::apply(Account & initiator)
{
    Account newAcc;
    assert(!Accounts::get(id, newAcc), "account already exist");

    newAcc.id = id;
    newAcc.owner = owner;

    Accounts::store(newAcc);
}

void createoffer::auth(Account & initiator)
{
    require_auth(initiator.owner);
    assert(initiator.isHotel, "only for hotels");
}

void createoffer::apply(Account & initiator)
{
    assert(arrivalDate > now(), "arrivalDate should be > now time");

    Offer newOffer;
    memcpy(&newOffer.roomInfo, &roomInfo, sizeof(roomInfo));

    newOffer.arrivalDate = arrivalDate;
    newOffer.price.quantity = price;

    newOffer.id = { (AccountId)initiator.id, initiator.totalOffers++ };
    initiator.openOffers++;

    Offers::store(newOffer);
    Accounts::store(initiator);

    eosio::print("createoffer: ", newOffer, "\n");
}


void createreq::auth(Account & initiator)
{
    require_auth(initiator.owner);
}

void createreq::apply(Account & initiator)
{
    Offer targetOffer;
    assert(Offers::get(offerId, targetOffer), "offer not found");

    assert(initiator.balance >= targetOffer.price, "not enough balance");

    Request newRequest;
    newRequest.offerId = targetOffer.id;
    memcpy(&newRequest.pubKey, &pubKey, sizeof(pubKey));

    newRequest.id = { (AccountId)initiator.id, initiator.totalRequests++ };
    initiator.openRequests++;

    Requests::store(newRequest);

    initiator.balance -= targetOffer.price;
    Accounts::store(initiator);

    eosio::print("createreq: ", newRequest, "\n");
}


void chargereq::auth(Account & initiator)
{
    require_auth(initiator.owner);
}

void chargereq::apply(Account & initiator)
{
    Request targetReq;
    assert(Requests::get(requestId, targetReq), "request not found");
    assert(!targetReq.charged, "request already charged");

    Offer assigneeOffer;
    assert(Offers::get(targetReq.offerId, assigneeOffer), "offer not found");

    assert(initiator.id == (AccountIdI64)assigneeOffer.id.accountId, "only assignee offer owner can charge request");

    targetReq.chargeData = chargeData;
    targetReq.charged = true;

    Requests::store(targetReq);

    initiator.balance += assigneeOffer.price;
    Accounts::store(initiator);
}


void refundreq::auth(Account & initiator)
{
    require_auth(initiator.owner);
}

void refundreq::apply(Account & initiator)
{
    Request targetReq;
    assert(Requests::get(requestId, targetReq), "request not found");

    Offer assigneeOffer;
    assert(Offers::get(targetReq.offerId, assigneeOffer), "offer not found");


    assert ((AccountId)initiator.id == targetReq.id.accountId, "request can refunded only by creator");

    Requests::remove(targetReq);

    if (!targetReq.charged)
        initiator.balance += assigneeOffer.price;

    initiator.openRequests--;
    Accounts::store(initiator);
}

} //namespace booking


/**
 *  The init() and apply() methods must have C calling convention so that the blockchain can lookup and
 *  call these methods.
 */

#define APPLY(message) {\
    booking::Account initiator; \
    assert(booking::Accounts::get((booking::AccountIdI64)message.initiatorId, initiator), "initiator account not found"); \
    message.auth(initiator); \
    message.apply(initiator); \
}

extern "C" {

    /**
     *  This method is called once when the contract is published or updated.
     */
    void init() {
        booking::Account booking;

        if (!booking::Accounts::get(0, booking))
            booking::Accounts::store({ 0, N(booking), booking::Token(1000ull * 1000ull * 1000ull) });

        eosio::print( "Init Booking!\n" );
    }

    /// The apply method implements the dispatch of events to this contract
    void apply( uint64_t code, uint64_t action ) {
        if (code == N(booking)) {
            switch (action) {
            case N(newaccount):
                APPLY(eosio::current_message<booking::newaccount>());
                break;
            case N(transfer):
                APPLY(eosio::current_message<booking::transfer>());
                break;
            case N(sethotel):
                APPLY(eosio::current_message<booking::sethotel>());
                break;
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
