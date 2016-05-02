/*
  Test tarino.
*/

/* global describe it */
'use strict'

const tarino = require('./tarino')
const assert = require('chai').assert
const fs = require('fs-extra')
const os = require('os')
const _exec = require('child_process').exec

let archives = ['tarino_na.tar', 'tarino_js.tar']
let sources = ['tarino.js', 'tarino.test.js']
let licenses = ['GPL-LICENSE', 'MIT-LICENSE']

let archivesGz = archives.map(function (archive) {
  return archive + '.gz'
})

describe('Test tarino:', function () {
  it('Test code conforms to JS Standard Style (http://standardjs.com).', function (done) {
    _exec(`standard ${sources.join(' ')}`, function (err, stdout, stderr) {
      let passed = true
      if (err || stderr.length > 0) {
        console.log('\n' + stderr)
        console.log(stdout)
        passed = false
      }
      assert.equal(passed, true)
      done()
    })
  })

  it('Should extract gzipped archive (tar.gz) using native implementation.', function (done) {
    tarino.extractTarGz('tarino.tar.gz', {native: true, verbose: true})
    assert.equal(fs.existsSync('tarino.tar'), true)
    done()
  })

  it('Should extract gzipped archive (tar.gz) using pure JS implementation.', function (done) {
    tarino.extractTarGz('tarino.tar.gz', {native: false, verbose: true})
    assert.equal(fs.existsSync('tarino.tar'), true)
    done()
  })

  it('Should list contents of archive (tar) using native implementation.', function (done) {
    tarino.listTar('tarino.tar', {native: true, verbose: true})
    done()
  })

  it('Should list contents of archive (tar) using pure JS implementation.', function (done) {
    tarino.listTar('tarino.tar', {native: false, verbose: true})
    done()
  })

  it('Should create archive (tar) using native implementation.', function (done) {
    tarino.createTar(archives[0], sources[0], {native: true, verbose: true})
    if (!fs.existsSync(archives[0])) {
      throw Error
    }
    done()
  })

  it('Should create archive (tar) using pure JS implementation.', function (done) {
    if (os.platform() !== 'win32') {
      tarino.createTar(archives[1], sources, {native: false, verbose: true})
      if (!fs.existsSync(archives[1])) {
        throw Error
      }
    } else {
      console.info('\tSkipping this test on Windows:')
    }
    done()
  })

  it('Should create gzipped archive (tar.gz) using native implementation.', function (done) {
    tarino.createTarGz(archivesGz[0], licenses, {native: true, verbose: true})
    if (!fs.existsSync(archivesGz[0])) {
      throw Error
    }
    done()
  })

  it('Should create gzipped archive (tar.gz) using pure JS implementation.', function (done) {
    if (os.platform() !== 'win32') {
      tarino.createTarGz(archivesGz[1], licenses, {native: false, verbose: true})
      if (!fs.existsSync(archivesGz[1])) {
        throw Error
      }
    } else {
      console.info('\tSkipping this test on Windows:')
    }
    done()
  })

  it('Archives created by native and pure JS implementations should be equal.', function (done) {
    let stats = []
    if (os.platform() !== 'win32') {
      archives.map(function (archive) {
        fs.lstat(archive, function (err, stat) {
          if (err) {
            throw Error
          }
          stats.push(stat.size)
          if (stats.length === 2) {
            assert.equal(stats[0], stats[1])
          }
        })
      })
      archivesGz.map(function (archiveGz) {
        fs.lstat(archiveGz, function (err, stat) {
          if (err) {
            throw Error
          }
          stats.push(stat.size)
          if (stats.length === 2) {
            assert.equal(stats[2], stats[3])
          }
        })
      })
    } else {
      console.info('\tSkipping this test on Windows:')
    }
    done()
  })
})
