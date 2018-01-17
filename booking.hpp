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

struct Id
{
    account_name owner;
    uint64_t number;

    void print() {
        eosio::print(   "{ owner: ", owner,
                        ", number: ", number, " }");
    }
};


//@abi table i128i128
struct Hotel
{
    Id id;
    uint128_t id2;  //megakostyl, can't create table with one i128 idx

    Token balance;

    uint64_t openOffers = 0u;
    uint64_t totalOffers = 0u;

    uint64_t openRequests = 0u;
    uint64_t totalRequests = 0u;

    void print() {
        eosio::print(   "{ id: ", id,
                        ", balance: ", balance,
                        ", openOffers: ", openOffers,
                        ", totalOffers: ", totalOffers,
                        ", openRequests: ", openRequests,
                        ", totalRequests: ", totalRequests, " }");
    }
};

using Hotels = eosio::table<N(booking), N(booking), N(Hotels), Hotel, Id, uint128_t>;
using HotelsById = Hotels::primary_index;


//@abi table i128i128
struct Offer
{
    Id id;
    Id hotelId;

    eosio::string roomInfo;
    time arrivalDate;
    Token price;

    void print() {
        eosio::print(   "{ id: ", id,
                        ", hotelId: ", hotelId,
                        ", roomInfo: ", roomInfo,
                        ", arrivalDate: ", arrivalDate,
                        ", price: ", price, " }");
    }
};

using Offers = eosio::table<N(booking), N(booking), N(Offers), Offer, Id, Id>;
using OffersById = Offers::primary_index;
using OffersByHotelId = Offers::secondary_index;


//@abi table i128i128
struct Request {
    Id id;
    Id offerId;

    eosio::string pubKey;
    uint8_t charged = false;
    eosio::string chargeData;

    void print() {
        eosio::print(   "{ id: ", id,
                        ", offerId: ", offerId,
                        ", pub_key: ", pubKey,
                        ", charged: ", charged ? "true" : "false",
                        ", charge_data: ", chargeData, " }");
    }
};

using Requests = eosio::table<N(booking), N(booking), N(Requests), Request, Id, Id>;
using RequestsById = Requests::primary_index;
using RequestsByHotelId = Requests::secondary_index;



/** Operations **/

struct Operation
{
    Id initiatorHotel;

    virtual void apply() = 0;
};

//@abi action CreateOffer
struct CreateOffer : public Operation
{
    fixed_string32 roomInfo;
    time arrivalDate;
    Token price;

    void apply() override;
};

//@abi action CreateReq
struct CreateReq : public Operation
{
    Id offerId;
    public_key pubKey;

    void apply() override;
};

//@abi action ChargeRequest
struct ChargeReq : public Operation
{
    Id requestId;
    fixed_string32 chargeData;

    void apply() override;
};

//@abi action RefundRequest
struct RefundReq : public Operation
{
    Id requestId;

    void apply() override;
};

} // namespace booking
