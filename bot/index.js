'use strict';

const TelegramBot = require('node-telegram-bot-api');
const EOS = require('eosjs');
const sha256 = require('sha256');
const fs = require('fs');

const mongoClient = require("mongodb").MongoClient;

const { conf } = require('./config.js');

const bot = new TelegramBot(conf.botToken, {polling: true});

const sessions = new Map();
const methods = new Map();

let logFile = null;


function printToLogFile(text) {
    if (!logFile) {
        fs.open(conf.logFile, 'a', (err, fd) => {
            logFile = fd;
            fs.write(logFile, text, err => {
                if (err) {
                    console.error('Log error: ' + err.message);
                    logFile = null;
                }
            })
        });
    }
    else
        fs.write(logFile, text, err => {
            if (err) {
                console.error('Log error: ' + err.message);
                logFile = null;
            }
        })
}

const log =  {
    d: text => printToLogFile('[DEBUG] ' + new Date().toISOString() + ' : ' + text + '\n'),
    i: text => printToLogFile('[INFO] ' +new Date().toISOString() + ' : ' + text + '\n'),
    e: text => printToLogFile('[ERROR] ' + new Date().toISOString() + ' : ' + text + '\n'),
    a: text => printToLogFile('[ALARM] ' + new Date().toISOString() + ' : ' + text + '\n'),
};

function db() {
    return new Promise((resolve, reject) => {
        mongoClient.connect(conf.mongoUrl, function(err, client) {
            if (err)
                reject(err);
            else
                resolve(client.db(conf.mongoDbName));
        })
    });
}

function replaceKey(acc) {
    return new Promise((resolve, reject) => {
        if (!EOS.modules.ecc.isValidPrivate(acc.privateKey))
            reject(new Error("invalid wif"));

        let user = {
            chatId: +acc.chatId,
            accountId: +acc.accountId,
            owner: acc.owner,
            privateKey: acc.privateKey,
            notifyEnabled: acc.notifyEnabled
        };

        db().then(db => {
                db.collection('users').replaceOne({chatId: user.chatId}, user, {upsert: true})
                    .then(resolve)
                    .catch(reject)
            })
    });
}

function getKey(chatId) {
    return new Promise((resolve, reject) => {
        db().then(db => {
                db.collection('users').findOne({chatId: chatId}, ((err, result) => {
                    if (result)
                        resolve(result);
                    else
                        reject(new Error('Not found'));
                }))
            })
    });
}

function getBookingAccount(accountId) {
    return new Promise((resolve, reject)=> {
        EOS.Localnet({httpEndpoint: conf.eosHttp}).getTableRows({
            json: true,
            table_key: "id",
            scope: "booking",
            code: "booking",
            table: "account",
            lower_bound: accountId,
            upper_bound: accountId + 1,
            limit: 1
        }).then(result => {
            if (result.rows.length)
                resolve(result.rows[0]);
            reject(new Error("not found"));
        }).catch(error => { reject(error) });
    });
}

function getNewAccountId() {
    return new Promise((resolve, reject)=> {
        EOS.Localnet({httpEndpoint: conf.eosHttp}).getTableRows({
            json: true,
            table_key: "id",
            scope: "booking",
            code: "booking",
            table: "account",
            limit: 1000
        })
            .then(result => resolve(result.rows.length))
            .catch(error => reject(error));
    });
}

function bookingAction(action, key, params) {
    params.initiatorId = +key.accountId;

    return new Promise((resolve, reject) => {
        EOS.Localnet({keyProvider: key.privateKey, httpEndpoint: conf.eosHttp})
            .contract('booking')
            .then(booking => {
                booking[action](params, {
                    authorization: key.owner + '@owner',
                    scope: 'booking'
                })
                    .then(res => resolve(res))
                    .catch(error => reject(JSON.parse(error)))
            });
    })
}

function createOffer(key, params) {
    return bookingAction('createoffer', key, params);
}

function deleteOffer(key, params) {
    return bookingAction('deleteoffer', key, params);
}

function createRequest(key, params) {
    return bookingAction('createreq', key, params);
}

function chargeRequest(key, params) {
    return bookingAction('chargereq', key, params);
}

function refundRequest(key, params) {
    return bookingAction('refundreq', key, params);
}

function transferTokes(key, params) {
    return bookingAction('transfer', key, params);
}

function idToName(id) {
    let name = '';
    id = +id;
    while (id > 0) {
        let s = id % 10;
        id = (id / 10)|0;
        name += String.fromCharCode(97 + s);
    }
    return name;
}

function newAccount() {
    return new Promise((resolve, reject) => {
        getNewAccountId()
            .then(id => {
                let params = {
                    owner: idToName(id),
                    id: id
                };

                EOS.modules.ecc.randomKey()
                    .then(privateKey => {
                        EOS.Localnet({keyProvider: conf.bookingKey.privateKey, httpEndpoint: conf.eosHttp})
                            .newaccount({
                                creator: 'booking',
                                name: params.owner,
                                owner: EOS.modules.ecc.privateToPublic(privateKey),
                                active: EOS.modules.ecc.privateToPublic(privateKey),
                                recovery: 'booking',
                                deposit: '0 EOS'
                            })
                            .then(() => {
                                bookingAction('newaccount', conf.bookingKey, params)
                                    .then(() => {
                                        bookingAction('transfer', conf.bookingKey, {
                                            to: params.id,
                                            quantity: conf.startTokens
                                        })
                                            .then(() => resolve({
                                                accountId: +id,
                                                owner: params.owner,
                                                privateKey: privateKey
                                            }))
                                            .catch(e => reject(e))
                                    })
                                    .catch(e => reject(e))
                            })
                            .catch(e => reject(e))
                    })
                    .catch(e => reject(e))
            })
            .catch(e => reject(e))
    });
}

function getOffers() {
    return new Promise((resolve, reject)=> {
        EOS.Localnet({httpEndpoint: conf.eosHttp}).getTableRows({
            json: true,
            table_key: "id",
            scope: "booking",
            code: "booking",
            table: "offer",
            limit: 1000  //TODO normal fetching
        })
            .then(result => resolve(result.rows.filter(offer => !offer.deleted)))
            .catch(error => reject(error));
    });
}

function getOffer(hotelID, number) {
    return new Promise((resolve, reject)=> {
        getOffers()
            .then(offers => {
                let offer = offers.find(r => r.id.accountId === +hotelID && r.id.number === +number);
                if (offer)
                    resolve(offer);
                else
                    reject(new Error('not found'));
            })
    });
}

function getRequests() {
    return new Promise((resolve, reject)=> {
        EOS.Localnet({httpEndpoint: conf.eosHttp}).getTableRows({
            json: true,
            table_key: "id",
            scope: "booking",
            code: "booking",
            table: "request",
            limit: 1000 //TODO normal fetching
        })
            .then(result => resolve(result.rows))
            .catch(error => reject(error));
    });
}

function getRequest(hotelID, number) {
    return new Promise((resolve, reject)=> {
        getRequests()
            .then(requests => {
                let request = requests.find(r => r.id.accountId === +hotelID && r.id.number === +number);
                if (request)
                    resolve(request);
                else
                    reject(new Error('Not found'));
            })
    });
}

function printHelp() {
    let text = '';
    text += 'Available commands:\n';
    text += '/get_me - get your account\n';
    text += '/my_bookings - get your bookings\n';
    text += '/new_booking - creating new booking\n';
    text += '/remove_booking - remove booking\n';
    text += '/get_offers - get available offers\n';
    text += '/offer_info - get info about offer\n';

    text += '/my_offers - get your offers(only for hotels)\n';
    text += '/new_offer - creating new offer(only for hotels)\n';
    text += '/remove_offer - remove offer(only for hotels)\n';
    text += '/get_requests - get requests for booking (only for hotels)\n';
    text += '/charge_request - charge request(only for hotels)\n';

    text += '/transfer - token transfer\n';
    text += '/set_notify - set notifications\n';
    text += '/cancel - cancel current command\n';
    text += '/help - print all commands\n';

    return text;
}

function printBooking(request, isHotel = false) {
    let text = '';
    if (isHotel)
        text += 'Account ID: ' + request.id.accountId + '\n';
    text += 'ID: ' + request.id.number + '\n';

    if (!isHotel)
        text += 'Hotel ID: ' + request.offerId.accountId + '\n';
    text += 'Offer ID: ' + request.offerId.number + '\n';

    text += 'Status: ' + (request.charged ? '' : 'not ') + 'approved\n';
    if (request.charged)
        text += 'Secret: ' + request.chargeData + '\n';

    return text;
}

function printOffer(offer, my) {
    let text = '';
    if (!my)
        text += 'Hotel ID: ' + offer.id.accountId + '\n';
    text += 'Offer ID: ' + offer.id.number + '\n';
    text += 'Date: ' + new Date(offer.arrivalDate).toISOString().slice(0,10) + '\n';
    text += 'Room info: ' + offer.roomInfo + '\n';
    text += 'Price: ' + offer.price.quantity + ' BKG\n';

    return text;
}

function sendNotify(accountId, text) {
    db().then(db => {
        db.collection('users').findOne({accountId: accountId}, (err, result) => {
            if (result && result.notifyEnabled) {
                bot.sendMessage(result.chatId, text);
            }
        })
    })
}

function onMessage(msg) {
    let chatId = msg.chat.id;
    let session = sessions.get(chatId);

    if (session) {
        let method = methods.get(session.method);

        session.match.push(msg.text);

        if (method.stages === session.stage) {
            if (method.auth)
                getKey(chatId)
                    .then(key => method.cb(chatId, session.match, key))
                    .catch(() => bot.sendMessage(chatId, 'Need /start'));
            else
                method.cb(chatId, session.match);
            sessions.delete(chatId);
            return;
        }

        session.stage++;

        bot.sendMessage(chatId, method.texts[session.stage - 1]);
        sessions.set(chatId, session);
    }
    else
        bot.sendMessage(chatId, "Unknown command, enter /help to find out all commands");
}

function regMethod(method) {
    methods.set(method.name, method);

    bot.onText(new RegExp(method.name + '$'), msg => {
        sessions.set(msg.chat.id, {
            method: method.name,
            stage: 0,
            match: []
        });
        onMessage(msg);
    })
}

regMethod({
    name: '/new_offer',
    stages: 3,
    texts: [
        "Enter arrival date(YYYY-MM-DD), please",
        "Enter room info, please",
        "Enter price, please"
    ],
    auth: true,
    cb: (chatId, match, key) => {

        let params = {
            arrivalDate: (Date.parse(match[1]) / 1000) | 0,
            roomInfo: sha256(match[2]),
            price: +match[3]
        };

        createOffer(key, params)
            .then(() => {
                bot.sendMessage(chatId, "Offer created");
            })
            .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
    }
});

regMethod({
    name: '/remove_offer',
    stages: 2,
    texts: [
        "Enter offer ID, please",
        "Are you sure (Y/N) ?",
    ],
    auth: true,
    cb: (chatId, match, key) => {
        if (match[2] === 'y' || match[2] === 'Y') {
            let params = {
                id: {
                    accountId: key.accountId,
                    number: +match[1]
                }
            };

            deleteOffer(key, params)
                .then(() => {
                    bot.sendMessage(chatId, "Offer removed");
                })
                .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
        }
        else
            bot.sendMessage(chatId, "Removing cancel")
    }
});

regMethod({
    name: '/my_offers',
    stages: 0,
    texts: [],
    auth: true,
    cb: (chatId, match, key) => {
        getOffers()
            .then(offers => {
                let myOffers = offers.filter(request => request.id.accountId === key.accountId);
                let text = '';

                if (!myOffers.length)
                    text = 'You have not actual offers';
                else
                    myOffers.forEach(offer => text += printOffer(offer, true) + '\n');

                bot.sendMessage(chatId, text);
            })
            .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
    }
});

regMethod({
    name: '/get_requests',
    stages: 0,
    texts: [],
    auth: true,
    cb: (chatId, match, key) => {
        getRequests()
            .then(requests => {
                let text = '';
                let requestsForMe = requests.filter(request => request.offerId.accountId === key.accountId);

                if (requestsForMe.length)
                    requestsForMe.forEach(request => text += printBooking(request, true) + '\n');
                else
                    text = 'You have not requests for booking';

                bot.sendMessage(chatId, text);
            })
            .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
    }
});

regMethod({
    name: '/charge_request',
    stages: 3,
    texts: [
        "Enter account ID, please",
        "Enter request ID, please",
        "Enter charge data, please"
    ],
    auth: true,
    cb: (chatId, match, key) => {

        let params = {
            requestId: {
                accountId: +match[1],
                number: +match[2]
            },
            chargeData: +match[3]
        };

        chargeRequest(key, params)
            .then(() => {
                bot.sendMessage(chatId, "Request charged");
                sendNotify(params.requestId.accountId, 'Your booking request has been approved, booking ID: ' +
                    params.requestId.number + ', ' +
                    'chargeData: ' + params.chargeData
                );
            })
            .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
    }
});

regMethod({
    name: '/get_offers',
    stages: 2,
    texts: [
        "Enter begin date(YYYY-MM-DD), please",
        "Enter end date(YYYY-MM-DD), please"
    ],
    cb: (chatId, match) => {

        let fromDate = Date.parse(match[1]);
        let toDate = 10**18;

        if (match[2].length)
            toDate = Date.parse(match[2]);

        getOffers()
            .then(offers => {
                offers = offers.filter(offer => {
                    return Date.parse(offer.arrivalDate) >= fromDate &&
                        Date.parse(offer.arrivalDate) <= toDate
                });

                let text = '';
                if (offers.length)
                    offers.forEach(offer => text += printOffer(offer) + '\n');
                else
                    text = 'No offers for this period';

                bot.sendMessage(chatId, text);
            })
            .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
    }
});

regMethod({
    name: '/new_booking',
    stages: 2,
    texts: [
        "Enter hotel ID, please",
        "Enter offer ID, please"
    ],
    auth: true,
    cb: (chatId, match, key) => {

        let params = {
            offerId: {
                accountId: +match[1],
                number: +match[2]
            }
        };

        params.pubKey = EOS.modules.ecc.privateToPublic(key.privateKey);
        createRequest(key, params)
            .then(() => {
                bot.sendMessage(chatId, "Booking created");
                sendNotify(params.offerId.accountId, 'You have new request for booking, offer ID: ' + params.offerId.number);
            })
            .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
    }
});

regMethod({
    name: '/my_bookings',
    stages: 0,
    texts: [],
    auth: true,
    cb: (chatId, match, key) => {
        getRequests()
            .then(requests => {
                let myRequests = requests.filter(request => request.id.accountId === key.accountId);
                let text = '';

                if (!myRequests.length)
                    text = 'You have not actual bookings';
                else
                    myRequests.forEach(request => {
                        text += printBooking(request) + '\n'
                    });

                bot.sendMessage(chatId, text);
            })
            .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
    }
});

regMethod({
    name: '/remove_booking',
    stages: 1,
    texts: [
        "Enter booking ID, please"
    ],
    auth: true,
    cb: (chatId, match, key) => {
        let params = {
            requestId: {
                number: +match[1]
            }
        };

        params.requestId.accountId = key.accountId;
        refundRequest(key, params)
            .then(() => bot.sendMessage(chatId, "Booking removed"))
            .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
    }
});


regMethod({
    name: '/offer_info',
    stages: 2,
    texts: [
        "Enter hotel ID, please",
        "Enter offer ID, please"
    ],
    cb: (chatId, match) => {

        let hotelID = match[1];
        let number = match[2];

        getOffer(hotelID, number)
            .then(offer => bot.sendMessage(chatId, printOffer(offer)))
            .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
    }
});

regMethod({
    name: '/transfer',
    stages: 2,
    texts: [
        "Enter receiver account ID, please",
        "Enter the transfer amount, please"
    ],
    auth: true,
    cb: (chatId, match, key) => {

        let params = {
            to: +match[1],
            quantity: +match[2]
        };

        transferTokes(key, params)
            .then(() => {
                bot.sendMessage(chatId, "Success transfer");
                sendNotify(params.to, "You received a transfer, from: " + key.accountId, + ' amount: ' + params.quantity + ' BKG');
            })
            .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
    }
});

regMethod({
    name: '/get_me',
    stages: 0,
    texts: [],
    auth: true,
    cb: (chatId, match, key) => {
        getBookingAccount(key.accountId)
            .then(acc => {
                let text = '';
                text += 'ID: ' + acc.id + '\n';
                text += 'Balance: ' + acc.balance.quantity + ' BKG\n';
                text += 'Active bookings: ' + acc.openRequests + '\n';
                text += 'Total bookings: ' + acc.totalRequests + '\n';
                if (acc.isHotel) {
                    text += 'Open Offers: ' + acc.openOffers + '\n';
                    text += 'Total Offers: ' + acc.totalOffers + '\n';
                }
                bot.sendMessage(chatId, text);
            })
            .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
    }
});

regMethod({
    name: '/set_notify',
    stages: 1,
    texts: ['Enter 0 for disable, 1 - enable'],
    auth: true,
    cb: (chatId, match, key) => {
        key.notifyEnabled = !(+match[1] === 0);
        replaceKey(key)
            .then(() => bot.sendMessage(chatId, 'Notifications ' + (key.notifyEnabled ? 'enabled' : 'disabled')))
            .catch(e => bot.sendMessage(chatId, "Fail: " + e.message))
    }
});

regMethod({
    name: '/start',
    stages: 0,
    texts: [],
    cb: (chatId) => {
        getKey(chatId)
            .then(() => {
                let text = 'Welcome back !\n' + printHelp();
                bot.sendMessage(chatId, text);
            })
            .catch(e => {
                if (e.message === 'Not found')
                    newAccount()
                        .then(acc => {
                            acc.chatId = +chatId;
                            replaceKey({
                                chatId: chatId,
                                accountId: acc.accountId,
                                owner: acc.owner,
                                privateKey: acc.privateKey,
                                notifyEnabled: true
                            })
                                .then(() => {
                                    let text = 'Welcome, we give you ' + conf.startTokens + ' BKG !\n';
                                    text += printHelp();
                                    bot.sendMessage(chatId, text);
                                })
                                .catch( e=> bot.sendMessage(chatId, "Fail: " + e.message))
                        })
            });
    }
});

regMethod({
    name: '/help',
    stages: 0,
    texts: [],
    cb: (chatId) => {
        bot.sendMessage(chatId, printHelp());
    }
});

bot.on('message', msg => {
    if (!methods.has(msg.text)) {
        if (msg.text === '/cancel'){
            let session = sessions.get(msg.chat.id);

            if (session) {
                sessions.delete(msg.chat.id);
                bot.sendMessage(msg.chat.id, 'The command ' + session.method + ' has been cancelled');
            }
            else
                bot.sendMessage(msg.chat.id, 'No active command to cancel');
        }
        else
            onMessage(msg);
    }

    log.i('New message[' +
        'chatId: ' + msg.chat.id + ', ' +
        'username: ' + msg.chat.username + ', ' +
        'text:' + msg.text + ']');
});
