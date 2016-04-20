/*
  tarino: a tar implementation with Node.js.
  Copyright 2016 Sam Saint-Pettersen.

  Dual licensed under the GPL and MIT licenses;
  see GPL-LICENSE and MIT-LICENSE respectively.
*/

'use strict'

const NC = String.fromCharCode(0)
const EOF_PADDING = 512

let USE_NATIVE = false

const fs = require('fs')
const conv = require('binstring')
const zlib = require('zlib')
const os = require('os')
const wrench = require('wrench')

let native = null
let rdir = null

try {
  native = require('./build/Release/tarino')
  USE_NATIVE = true
} catch (e) {
  USE_NATIVE = false
}

function getStats (filename) {
  let stats = fs.lstatSync(filename)
  let size = stats['size']
  let modified = Date.parse(stats['mtime']) / 1000
  let type = 0

  if (stats.isDirectory()) {
    type = 5
  }

  return [size, modified, type]
}

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

function writePaddedData (data) {
  let eof = 0
  let m = 1
  while (eof < data.length) {
    eof = EOF_PADDING * m
    if (data.length <= eof) break
    m++
  }
  return data + padData(eof - (data.length - 1))
}

function toBuffer (data) {
  return conv(data, {out: 'buffer'})
}

function calcChecksum (header) {
  let checksum = 0
  for (let i = 0; i < header.length; i++) {
    checksum += header.charCodeAt(i)
  }
  return decToPaddedOctal(checksum - 64, 6)
}

function getContents (srcpath, folder) {
  let path = fs.readdirSync(srcpath)
  let files = path
  let filesr = []
  if (Array.isArray(path)) {
    files = path.map(function (f) {
      if (getStats(`${srcpath}/${f}`)[2] === 5) {
        filesr.push(getContents(`${srcpath}/${f}`))
      }
      return `${srcpath}/${f}`
    })
    if (folder) {
      files.unshift(srcpath + '/')
    }
  }
  filesr.map(function (filesrr) {
    filesrr.map(function (f) {
      files.push(f)
    })
  })
  return files
}

function writeTarEntry (tarname, filename, callback) {
  let contents = ''
  let bits = null
  let stats = fs.lstatSync(filename)
  let size = decToPaddedOctal(stats['size'], 11)
  let modified = decToPaddedOctal(Date.parse(stats['mtime']) / 1000, 0)
  let type = 0
  let fm = '0100777'
  if (stats.isDirectory()) {
    type = 5
    fm = '0040777'
  } else {
    contents = fs.readFileSync(filename, 'ascii').toString()
  }
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
  let tar = fs.createWriteStream(tarname, {start: 0, flags: 'w', encoding: 'binary'})
  let header = `${filename}${padData(101 - filename.length)}` // (a)
  header += `${fm}${NC}0000000${NC}0000000${NC}${size}${NC}${modified}${NC}` // (b, c, d, e, f)
  header += `000000${NC} ${type}${padData(101)}ustar${NC}00${padData(248)}` // (g, h, i)
  let data = []
  data.push(toBuffer(header))
  if (type === 0) {
    data.push(toBuffer(writePaddedData(contents)))
    tar.write(Buffer.concat(data))
  } else {
    data[0] = toBuffer(header.replace(/010/, '000'))
    tar.write(Buffer.concat(data))
  }
  tar.close()
  return callback(header)
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

function writeTarEntries (tarname, entries) {
  if (USE_NATIVE) {
    let manifest = tarname + '.entries'
    fs.writeFileSync(manifest, '')
    entries.map(function (entry) {
      fs.appendFileSync(manifest,
      `${entry.part}:${entry.file}:${entry.size}:${entry.modified}:${entry.etype}\n`)
    })
    native.write_tar_entries(tarname, manifest)
  } else {
    for (let i = 0; i < entries.length; i++) {
      writeTarEntry(entries[i].part, entries[i].file, function (header) {
        writeChecksum(entries[i].part, header, function (data) {
          fs.appendFileSync(tarname, data.toString())
          fs.unlinkSync(entries[i].part)
          if (i === entries.length - 1) {
            finalizeTar(tarname)
          }
        })
      })
    }
  }
}

function finalizeTar (tarname) {
  fs.appendFileSync(tarname, padData((EOF_PADDING * 3) + 1))
}

function truncateNew (tarname, entries) {
  if (fs.existsSync(tarname)) {
    fs.unlinkSync(tarname)
  }
  fs.closeSync(fs.openSync(tarname, 'w'))
  if (entries !== null) {
    for (let i = 0; i < entries.length; i++) {
      fs.closeSync(fs.openSync(entries[i].part, 'w'))
    }
  }
}

module.exports.createTar = function (tarname, filename, options) {
  if (options && options.native !== undefined) {
    USE_NATIVE = options.native

    if (native == null && USE_NATIVE) {
      USE_NATIVE = false
      console.warn(
      'tarino: Falling back to pure JS implementation ( native: ', USE_NATIVE, ')')
      if (os.platform() === 'win32') {
        console.warn('There is known problems with the non-native impl. on Windows!')
      }
    }
  }

  if (options && options.root) {
    filename = `${options.root}/${filename}`
  }

  if (options && options.flat) {
    let fns = filename.split(/\//)
    let flat_fn = fns[fns.length - 1]
    fs.writeFileSync(flat_fn, fs.readFileSync(filename))
    filename = flat_fn
  }

  if (options && options.folder) {
    if (options.root !== undefined) {
      rdir = filename.split(options.root + '/')[1]
      wrench.copyDirSyncRecursive(filename, rdir)
      filename = rdir
    }
    filename = getContents(filename, true)
  }

  try {
    if (Array.isArray(filename)) {
      let i = 0
      let entries = filename.map(function (fn) {
        if (fn.length < 100) {
          let attribs = getStats(fn)
          return {
            part: tarname.replace(/.tar$/, `.${++i}`),
            file: fn,
            size: attribs[0],
            modified: attribs[1],
            etype: attribs[2]
          }
        } else {
          throw Error
        }
      })
      if (!USE_NATIVE) {
        truncateNew(tarname, entries)
      }
      if (options && options.verbose) {
        console.log(entries)
      }
      writeTarEntries(tarname, entries)
    } else {
      if (USE_NATIVE && filename.length < 100) {
        let attribs = getStats(filename)
        native.write_tar_entry(tarname, filename, attribs[0], attribs[1], attribs[2])
      } else {
        if (filename.length < 100) {
          truncateNew(tarname, null)
          writeTarEntry(tarname, filename, function (header) {
            writeChecksum(tarname, header, function () {
              finalizeTar(tarname)
            })
          })
        } else {
          throw Error
        }
      }
    }
  } catch (e) {
    console.warn(`tarino: Error creating ${tarname}`)
    console.log(e.stack)
  } finally {
    if (options && options.flat && fs.existsSync(filename)) {
      fs.unlinkSync(filename)
    } else if (options && options.folder && options.root && rdir !== null) {
      wrench.rmdirSyncRecursive(rdir)
    }
  }
}

module.exports.createTarGz = function (tarnamegz, filename, options) {
  if (/\.tar$/.test(tarnamegz)) {
    console.warn('tarino: You are trying to create a gzipped tar with a *.tar extension.')
    console.warn('This will not work. Please use *.tar.gz.')
    process.exit(-1)
  }
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
  } finally {
    if (fs.existsSync(tarname)) {
      fs.unlinkSync(tarname)
    }
  }
}
