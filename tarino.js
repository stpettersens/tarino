/*
  tarino: a tar implementation with Node.js.
  Copyright 2016 Sam Saint-Pettersen.

  Dual licensed under the GPL and MIT licenses;
  see GPL-LICENSE and MIT-LICENSE respectively.
*/

'use strict'

const fs = require('fs')
// const path = require('path')
const zlib = require('zlib')

const NC = String.fromCharCode(0)
const EOF_PADDING = 1029

function decToPaddedOctal (num, length) {
  let octal = parseInt(num, 10).toString(8)
  let padding = ''
  for (let i = 0; i < (length - octal.length); i++) {
    padding += '0'
  }
  return padding + octal
}

function padData (length) {
  let padding = ''
  for (let i = 1; i < length; i++) {
    padding += NC
  }
  return padding
}

function calcChecksum (header) {
  let checksum = 0
  for (let i = 0; i < header.length; i++) {
    checksum += header.charCodeAt(i)
  }
  return decToPaddedOctal(checksum - 64, 6)
}

function getContents (srcpath) {
  return fs.readdirSync(srcpath)
}

function writeTarEntry (tarname, filename, callback) {
  let contents = fs.readFileSync(filename, 'ascii').toString()
  let stats = fs.statSync(filename)
  let size = decToPaddedOctal(stats['size'], 11)
  let modified = decToPaddedOctal(Date.parse(stats['mtime']) / 1000, 0)
  let type = 0
  /**
    * TAR FORMAT SPECIFICATION
    * (a) File name (0-)
    * (b) File mode (100; 8)
    * (c) Owner's numeric user ID (108; 8)
    * (d) Group's numeric user ID (116; 8)
    * (e) File size in bytes (octal) (124; 12)
    * (f) Last modification time in numeric Unix time format (octal) (136; 12)
    * (g) Checksum for header record (148; 8)
    * (h) Link indicator (file type) (156; 1)
    * (i) UStar indicator (257; 6)
  */
  let tar = fs.createWriteStream(tarname, {start: 0, flags: 'w'})
  let header = `${filename}${padData(101 - filename.length)}` // (a)
  header += `0100777${NC}0000000${NC}0000000${NC}${size}${NC}${modified}${NC}` // (b, c, d, e, f)
  header += `000000${NC} ${type}${padData(257 - 156)}ustar${NC}00${padData(512 - 264)}` // (g, h, i)
  let data = `${contents}${padData(320)}` // !!! was 512, 323, 238, 512
  tar.write(header + data)
  tar.close()
  callback(header)
}

function writeChecksum (tarname, header, callback) {
  let tar = fs.createWriteStream(tarname, {start: 148, flags: 'r+'})
  tar.write(calcChecksum(header))
  tar.close()
  fs.readFile(tarname, function (err, data) {
    if (err) {
      console.warn('tarino: Error writing checksum on archive.')
      console.log(err)
    }
    return callback(data)
  })
}

function writeTarEntries (tarname, entries, callback) {
  for (let i = 0; i < entries.length; i++) {
    writeTarEntry(entries[i].part, entries[i].file, function (header) {
      writeChecksum(entries[i].part, header, function (data) {
        fs.appendFileSync(tarname, data.toString())
        if (i === entries.length - 1) {
          fs.appendFileSync(tarname, padData(EOF_PADDING))
        }
      })
    })
  }
  entries.map(function (entry) {
    fs.unlinkSync(entry.part)
  })
}

module.exports.createTar = function (tarname, filename, options) {
  if (options && options.flat) {
    let fns = filename.split(/\//)
    filename = fns[fns.length - 1]
  }

  if (options && options.root) {
    console.log(options.root)
  }

  if (options && options.folder) {
    console.log(getContents(options.root))
  }

  try {
    if (Array.isArray(filename)) {
      let i = 0
      let entries = filename.map(function (fn) {
        if (fn.length < 100) {
          return {part: tarname.replace(/.tar$/, `.${++i}`), file: fn}
        } else {
          throw Error
        }
      })
      fs.closeSync(fs.openSync(tarname, 'w'))
      writeTarEntries(tarname, entries)
    } else {
      if (filename.length < 100) {
        writeTarEntry(tarname, filename, function (header) {
          writeChecksum(tarname, header, function () {
            fs.appendFileSync(tarname, padData(EOF_PADDING))
          })
        })
      } else {
        throw Error
      }
    }
  } catch (e) {
    console.log(e.stack)
  }
}

module.exports.createTarGz = function (tarnamegz, filename, options) {
  let tarname = tarnamegz.split(/\.gz$/)[0]
  this.createTar(tarname, filename, options)
  let gzip = fs.createWriteStream(tarnamegz)
  let tar = fs.createReadStream(tarname)
  try {
    tar
    .pipe(zlib.createGzip())
    .pipe(gzip)
  } catch (e) {
    console.warn(`tarino: Error creating ${tarnamegz}`)
    console.log(e)
  }
}
