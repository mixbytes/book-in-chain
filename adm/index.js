'use strict';

const EOS = require('eosjs');

if (process.argv.length < 3) {
    console.error('usage: <action> <params> <accounts> <keys>\n' +
        'params in JSON \n' +
        'accounts,keys with \",\" separator \n ');
    process.exit(1);
}

let action = process.argv[2];
let params = process.argv.length > 3 ? JSON.parse(process.argv[3]) : null;

let auth = process.argv.length > 4 ? process.argv[4] : null;
let key = process.argv.length > 5 ? process.argv[5] : null;


EOS.Localnet({ keyProvider: key })
    .contract('booking')
    .then(booking => {
        booking[action](params, {
            authorization: auth + '@owner',
            scope: 'booking'
        })
            .then(result => {
                console.log('Success:');
                console.dir(result, {depth: null, colors: true});
                process.exit(0);
            })
            .catch(error => {
                console.error('Fail:');
                console.dir(JSON.parse(error), {depth: null, colors: true});
                process.exit(1);
            })
    });