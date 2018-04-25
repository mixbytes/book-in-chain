/* eslint-env mocha */
const assert = require('assert');
const fs = require('fs');
const binaryen = require('binaryen');

const EOS = require('eosjs');
const {ecc} = EOS.modules;
const {Keystore} = require('eosjs-keygen');

const { conf } = require('./test.config.js');


const initaPrivate = '5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3';
const initaPublic = 'EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV';


describe('test of booking', () => {

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

if (true) {
  	it('deploy contract', async function() {
		const bookingPrivate = ecc.seedPrivate('booking');
		const bookingPublic = ecc.privateToPublic(bookingPrivate);
		
 		const eos = await EOS.Localnet({
			keyProvider: initaPrivate,
			httpEndpoint: 'http://127.0.0.1:8888', //conf.eosHttp,
			broadcast: false,
			sign: false,
			debug: false,
			binaryen: binaryen // !!!!!
			// mockTransactions: () => 'pass', // or 'fail'
			//  transactionHeaders: (expireInSeconds, callback) => {
			//	    callback(null/*error*/, headers)
			//	  },
			//	  expireInSeconds: 60,
		});

		//eos.binaryen = require("binaryen")

		let tx_acc = await eos.newaccount({
  			creator: 'inita',
  			name: 'booking',
  			owner: bookingPublic,
  			active: bookingPublic,
  			recovery: 'inita'
		});


		// await eos.transfer('inita', 'initb', '1 EOS', '');


		// DEPLOY CONTRACT

		// wast_body must be a Buffer, JS: Buffer.isBuffer(wast_body) == true;
		let wast_body = await fs.readFileSync(`booking.wast`);
		let abi_body = await fs.readFileSync(`booking.abi`);

		// hz defference between two variants:
		// - eos.setcode('booking', 0, 0, wast_body);
		// - eos.setabi('booking', JSON.parse(abi_body))
		// + eos.setcode('booking', 0, 0, wast_body, abi_body);
		//let tx = await eos.setcode('booking', 0, 0, wast_body, abi_body);
		//await console.log(tx);
		// await sleep(1900);

		let booking = null;
		booking = await eos.contract('booking', {authorization: 'booking'});
		await console.log(booking);

	

	

		/*
		const msg = { "offer_id": 27 };
		let res = await eos.transaction({
			scope: ['inita', 'booking', 'booking'],
			messages: [
			  {
				code: 'booking',
				type: 'createoffer',
				authorization: [{
				  account: 'booking',
				  permission: 'active'
				}],
				data: msg
			  }
			]
		  });

		*/

  		/*
		await eos.transaction({
    			scope: ['booking'],
    			messages: [{
				      code: 'booking',
      				  type: 'createoffer',
      				  data: {
        				"offer_id": 22
      					}
    			}]
  			});
		*/

		// Publish contract to the blockchain



		//booking.issue('inita', {authorization: 'booking'});

		// await booking.createoffer({});	

	})
}

});


