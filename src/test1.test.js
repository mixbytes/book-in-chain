/* eslint-env mocha */
const assert = require('assert')

const EOS = require('eosjs');
const {ecc} = EOS.modules;
const {Keystore} = require('eosjs-keygen');

const { conf } = require('./test.config.js');





async function bookingAction(action, key, params) {
    params.initiatorId = +key.accountId;

	const eos = EOS.Localnet({
			keyProvider: key.privateKey,
			httpEndpoint: 'http://127.0.0.1:8888', //conf.eosHttp,
			broadcast: false,
			sign: true
	});

    return eos.contract('booking')
				.then(booking => {
         			return booking[action](params, { authorization: key.owner + '@owner',
 					scope: 'booking'
            	})
            });

}


describe('test of booking', () => {


  	it('transaction', async function() {
    	const privateKey = await ecc.unsafeRandomKey();

		let res = await bookingAction('createoffer', privateKey, {});

	})
});

//Eos = require('eosjs') 

// eos = Eos.Localnet() // 127.0.0.1:8888
//eos = Eos.Testnet() // testnet at eos.io

// All API methods print help when called with no-arguments.
//eos.getBlock()

// Next, your going to need eosd running on localhost:8888

// If a callback is not provided, a Promise is returned
//eos.getBlock(1).then(result => {console.log(result)})

// Parameters can be sequential or an object
//eos.getBlock({block_num_or_id: 1}).then(result => console.log(result))

// Callbacks are similar

//callback = (err, res) => {err ? console.error(err) : console.log(res)}
//eos.getBlock(1, callback)
//eos.getBlock({block_num_or_id: 1}, callback)

// Provide an empty object or a callback if an API call has no arguments

//eos.getInfo({}).then(result => {console.log(result)})

