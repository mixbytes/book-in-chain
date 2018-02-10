/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eoslib/eos.hpp>
#include <eoslib/db.hpp>
#include <eoslib/token.hpp>
#include <eoslib/string.hpp>


namespace booking {


using Token = eosio::token<uint64_t, N(Books)>;

using AccountId = uint32_t;
using AccountIdI64 = uint64_t;

struct Id
{
    AccountId accountId;
    uint32_t number;

    void print() {
        eosio::print(   "{ accountId: ", accountId,
                        ", number: ", number, " }");
    }
};


//@abi table i64
struct Account
{
    AccountIdI64 id;

    account_name owner;
    Token balance;

    uint8_t isHotel = 0;

    uint32_t openOffers = 0u;
    uint32_t totalOffers = 0u;

    uint32_t openRequests = 0u;
    uint32_t totalRequests = 0u;

    void print() {
        eosio::print(   "{ id: ", id,
                        ", owner: ", owner,
                        ", balance: ", balance,
                        ", isHotel: ", isHotel ? "true" : "false",
                        ", openOffers: ", openOffers,
                        ", totalOffers: ", totalOffers,
                        ", openRequests: ", openRequests,
                        ", totalRequests: ", totalRequests, " }");
    }
};

using Accounts = eosio::table<N(booking), N(booking), N(account), Account, AccountIdI64>;
using AccountsById = Accounts::primary_index;


//@abi table i64
struct Offer
{
    Id id;

    eosio::string roomInfo;
    time arrivalDate;
    Token price;

    void print() {
        eosio::print(   "{ id: ", id,
                        ", roomInfo: ", roomInfo,
                        ", arrivalDate: ", arrivalDate,
                        ", price: ", price, " }");
    }
};

using Offers = eosio::table<N(booking), N(booking), N(offer), Offer, Id>;
using OffersById = Offers::primary_index;


//@abi table i64
struct Request {
    Id id;
    Id offerId;

    eosio::string pubKey;
    uint32_t charged = false;
    eosio::string chargeData;

    void print() {
        eosio::print(   "{ id: ", id,
                        ", offerId: ", offerId,
                        ", pub_key: ", pubKey,
                        ", charged: ", charged ? "true" : "false",
                        ", charge_data: ", chargeData, " }");
    }
};

using Requests = eosio::table<N(booking), N(booking), N(request), Request, Id>;
using RequestsById = Requests::primary_index;



/** Operations **/

struct Operation
{
    AccountId initiatorId;

    virtual void onApply(Account & initiator) {};
    virtual void checkAuth(Account & initiator) {};

    void apply() {
        Account initiator;
        assert(Accounts::get((AccountIdI64)initiatorId, initiator), "initiator account not found");

        checkAuth(initiator);
        onApply(initiator);
    }
};

//@abi action CreateOffer
struct CreateOffer : public Operation
{
    eosio::string roomInfo;
    time arrivalDate;
    Token price;

    void onApply(Account & initiator) override;
    void checkAuth(Account & initiator) override;
};

//@abi action CreateReq
struct CreateReq : public Operation
{
    Id offerId;
    eosio::string pubKey;

    void onApply(Account & initiator) override;
    void checkAuth(Account & initiator) override;
};

//@abi action ChargeRequest
struct ChargeReq : public Operation
{
    Id requestId;
    eosio::string chargeData;

    void onApply(Account & initiator) override;
    void checkAuth(Account & initiator) override;
};

//@abi action RefundRequest
struct RefundReq : public Operation
{
    Id requestId;

    void onApply(Account & initiator) override;
    void checkAuth(Account & initiator) override;
};

} // namespace booking
