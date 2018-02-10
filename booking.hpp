/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#include <eoslib/eos.hpp>
#include <eoslib/db.hpp>
#include <eoslib/token.hpp>
#include <eoslib/string.hpp>


namespace booking {


using Token = eosio::token<uint64_t, N(books)>;

using AccountId = uint32_t;
using AccountIdI64 = uint64_t;

struct PACKED(Id)
{
    AccountId accountId;
    uint32_t number;

    void print() {
        eosio::print(   "{ accountId: ", accountId,
                        ", number: ", number, " }");
    }
};


//@abi table i64
struct PACKED(Account)
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
struct PACKED(Offer)
{
    Id id;

    uint64_t roomInfo;
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
struct PACKED(Request)
{
    Id id;
    Id offerId;

    eosio::string pubKey;
    uint8_t charged = false;
    uint128_t chargeData;

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
};

static_assert(sizeof(Operation) == 4, "sizeof(Operation) != 4");

//@abi action CreateOffer
struct PACKED(CreateOffer) : public Operation
{
    uint64_t roomInfo;
    time arrivalDate;
    uint64_t price;

    void onApply(Account & initiator);
    void checkAuth(Account & initiator);
};

static_assert(sizeof(CreateOffer) == 4 + 8 + 4 + 8, "sizeof(CreateOffer) != 24");

//@abi action CreateReq
struct PACKED(CreateReq) : public Operation
{
    Id offerId;
    public_key pubKey;

    void onApply(Account & initiator);
    void checkAuth(Account & initiator);
};

static_assert(sizeof(CreateReq) == 4 + 8 + 33, "sizeof(CreateReq) != 45");

//@abi action ChargeReq
struct PACKED(ChargeReq) : public Operation
{
    Id requestId;
    uint128_t chargeData;

    void onApply(Account & initiator);
    void checkAuth(Account & initiator);
};

static_assert(sizeof(ChargeReq) == 4 + 16 + 8, "sizeof(ChargeReq) != 20");

//@abi action RefundReq
struct PACKED(RefundReq) : public Operation
{
    Id requestId;

    void onApply(Account & initiator);
    void checkAuth(Account & initiator);
};

static_assert(sizeof(RefundReq) == 4 + 8, "sizeof(RefundReq) != 12");
} // namespace booking
