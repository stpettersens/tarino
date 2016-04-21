/*
  Test tarino.
*/

/* global describe it */
'use strict'

const tarino = require('./tarino')
const assert = require('chai').assert
const fs = require('fs')
const os = require('os')
const _exec = require('child_process').exec

let archives = ['tarino_na.tar', 'tarino_js.tar']
let sources = ['tarino.js', 'tarino.test.js']

let archives_gz = archives.map(function (archive) {
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

  it('Should create archive (tar) using native implementation.', function (done) {
    tarino.createTar(archives[0], sources, {native: true, verbose: true})
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
    tarino.createTarGz(archives_gz[0], sources, {native: true, verbose: true})
    if (!fs.existsSync(archives_gz[0])) {
      throw Error
    }
    done()
  })

  it('Should create gzipped archive (tar.gz) using pure JS implementation.', function (done) {
    if (os.platform() === 'win32') {
      tarino.createTarGz(archives_gz[1], sources, {native: false, verbose: true})
      if (!fs.existsSync(archives_gz[1])) {
        throw Error
      }
    } else {
      console.info('\tSkipping this test on Windows:')
    }
    done()
  })

  it('Archives created by native and pure JS implementations should be equal', function (done) {
    let stats = []
    if (os.platform() !== 'win32') {
      archives.map(function (archive) {
        fs.lstat(archive, function (err, stat) {
          if (err) {
            throw Error
          }
          stats.push(stat)
          if (stats.length === 2) {
            console.log(stats)
            assert.equal(stats[0]['size'], stats[1]['size'])
          } 
        })
      })
      archives_gz.map(function (archive_gz) {
        fs.lstat(archive_gz, function (err, stat) {
          if (err) {
            throw Error
          }
          stats.push(stat)
          if (stats.length === 2) {
            console.log(stats)
            assert.equal(stats[2]['size'], stats[3]['size'])
          }
        })
      })
    } else {
      console.info('\tSkipping this test on Windows:')
    }
    done()
  })
})
