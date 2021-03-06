/*
  Test tarino 1/2.
*/

/* global describe it */
'use strict'

const tarino = require('./tarino')
const assert = require('chai').assert
const fs = require('fs-extra')
const os = require('os')

let archives = ['tarino_na.tar', 'tarino_js.tar']
let sources = ['tarino.js', 'tarino.test1.js', 'tarino.test2.js', 'Gulpfile.js']
let licenses = ['GPL-LICENSE', 'MIT-LICENSE']

let archivesGz = archives.map(function (archive) {
  return archive + '.gz'
})

describe('Test tarino 1/2:', function () {
  it('Should extract gzipped archive (tar.gz) using native implementation.', function (done) {
    console.log('')
    tarino.extractTarGz('tarino.tar.gz', {native: true, verbose: true, full: false})
    done()
  })

  /* it('Should extract gzipped archive (tar.gz) using pure JS implementation.', function (done) {
    console.log('')
    tarino.extractTarGz('tarino.tar.gz', {native: false, verbose: true, full: true})
    assert.equal(fs.existsSync(licenses[0]), true)
    assert.equal(fs.existsSync(licenses[1]), true)
    done()
  }) */

  it('Should create archive (tar) using native implementation.', function (done) {
    console.log('')
    tarino.createTar(archives[0], sources[0], {native: true, verbose: true})
    assert.equal(fs.existsSync(archives[0]), true)
    done()
  })

  it('Should create archive (tar) using pure JS implementation.', function (done) {
    console.log('')
    tarino.createTar(archives[1], sources, {native: false, verbose: true})
    assert.equal(fs.existsSync(archives[1]), true)
    done()
  })

  it('Should create gzipped archive (tar.gz) using native implementation.', function (done) {
    if (os.platform() !== 'win32') {
      console.log('')
      tarino.createTarGz(archivesGz[0], licenses, {native: true, verbose: true})
      assert.equal(fs.existsSync(archivesGz[0]), true)
    } // NEED TO FIX THIS.
    done()
  })

  it('Should create gzipped archive (tar.gz) using pure JS implementation.', function (done) {
    if (os.platform() !== 'win32') {
      console.log('')
      tarino.createTarGz(archivesGz[1], licenses, {native: false, verbose: true})
      assert.equal(fs.existsSync(archivesGz[1]), true)
    } else {
      console.info('\tSkipping this test on Windows:')
    }
    done()
  })
})
