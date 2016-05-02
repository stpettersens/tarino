/*
  Test tarino 2/2.
*/

/* global describe it */
'use strict'

const tarino = require('./tarino')

describe('Test tarino 2/2:', function () {
  it('Should list contents of archive (tar) using native implementation.', function (done) {
    tarino.listTar('tarino.tar', {native: true, verbose: false})
    done()
  })

  it('Should list contents of archive (tar) using pure JS implementation.', function (done) {
    tarino.listTar('tarino.tar', {native: false, verbose: true})
    done()
  })
})
