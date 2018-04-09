/* eslint-env mocha */
const assert = require('assert')

const Eos = require('eosjs')
const {ecc} = Eos.modules
const {Keystore} = require('eosjs-keygen')

describe('version', () => {
  it('exposes a version number', () => {
    assert.ok(Eos.version)
  })
})

describe('offline', () => {
  const headers = {
    expiration: new Date().toISOString().split('.')[0],
    region: 0,
    ref_block_num: 1,
    ref_block_prefix: 452435776,
    context_free_cpu_bandwidth: 0,
    packed_bandwidth_words: 0,
    context_free_actions: []
  }

  it('transaction', async function() {
    const privateKey = await ecc.unsafeRandomKey()

    const eos = Eos.Localnet({
      keyProvider: privateKey,
      httpEndpoint: 'https://doesnotexist.example.org',
      transactionHeaders: (expireInSeconds, callback) => {
        callback(null/*error*/, headers)
      },
      broadcast: false,
      sign: true
    })

    const memo = ''
    const trx = await eos.transfer('bankers', 'people', '1000000 EOS', memo)

    assert.deepEqual({
      expiration: trx.transaction.data.expiration,
      region: 0,
      ref_block_num: trx.transaction.data.ref_block_num,
      ref_block_prefix: trx.transaction.data.ref_block_prefix,
      context_free_cpu_bandwidth: 0,
      packed_bandwidth_words: 0,
      context_free_actions: []
    }, headers)

    assert.equal(trx.transaction.signatures.length, 1, 'expecting 1 signature')
  })
})

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

