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
    uint8_t deleted = 0;

    void print() {
        eosio::print(   "{ id: ", id,
                        ", roomInfo: ", roomInfo,
                        ", arrivalDate: ", arrivalDate,
                        ", price: ", price,
                        ", deleted: ", deleted ? "true" : "false", " }");
    }
};

using Offers = eosio::table<N(booking), N(booking), N(offer), Offer, Id>;
using OffersById = Offers::primary_index;


//@abi table i64
struct PACKED(Request)
{
    Id id;
    Id offerId;

    public_key pubKey;
    uint8_t charged = false;
    uint128_t chargeData;

    void print() {
        eosio::print(   "{ id: ", id,
                        ", offerId: ", offerId,
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

//@abi action newaccount
struct PACKED(newaccount) : public Operation
{
    account_name owner;
    AccountId id;

    void apply(Account & initiator);
    void auth(Account & initiator);
};

static_assert(sizeof(newaccount) == 16, "sizeof(newaccount) != 16");


//@abi action transfer
struct PACKED(transfer) : public Operation
{
    AccountId to;
    uint64_t quantity;

    void apply(Account & initiator);
    void auth(Account & initiator);
};

static_assert(sizeof(transfer) == 16, "sizeof(transfer) != 16");


//@abi action sethotel
struct PACKED(sethotel) : public Operation
{
    AccountId id;
    uint8_t set;

    void apply(Account & initiator);
    void auth(Account & initiator);
};

static_assert(sizeof(sethotel) == 9, "sizeof(sethotel) != 9");


//@abi action createoffer
struct PACKED(createoffer) : public Operation
{
    checksum roomInfo;
    time arrivalDate;
    uint64_t price;

    void apply(Account & initiator);
    void auth(Account & initiator);
};

static_assert(sizeof(createoffer) == 4 + 32 + 4 + 8, "sizeof(createoffer) != 24");


//@abi action deleteoffer
struct PACKED(deleteoffer) : public Operation
{
    Id id;

    void apply(Account & initiator);
    void auth(Account & initiator);
};

static_assert(sizeof(deleteoffer) == 4 + 8, "sizeof(deleteoffer) != 12");


//@abi action createreq
struct PACKED(createreq) : public Operation
{
    Id offerId;
    public_key pubKey;

    void apply(Account & initiator);
    void auth(Account & initiator);
};

static_assert(sizeof(createreq) == 4 + 8 + 33, "sizeof(createreq) != 45");


//@abi action chargereq
struct PACKED(chargereq) : public Operation
{
    Id requestId;
    uint128_t chargeData;

    void apply(Account & initiator);
    void auth(Account & initiator);
};

static_assert(sizeof(chargereq) == 4 + 16 + 8, "sizeof(chargereq) != 20");


//@abi action refundreq
struct PACKED(refundreq) : public Operation
{
    Id requestId;

    void apply(Account & initiator);
    void auth(Account & initiator);
};

static_assert(sizeof(refundreq) == 4 + 8, "sizeof(refundreq) != 12");

} // namespace booking
