'use strict'

const tarino = require('./tarino')

// tarino.createTar('tarino.tar', 'tarino.cc', {root: 'native', flat: true, native: true})
// tarino.createTar('tarino.tar', [ 'README.md', 'package.json' ])
// tarino.createTar('tarino.tar', 'native', {folder: true})
// tarino.extractTar('__tarino.tar__', {native: true, verbose: true})
// tarino.createTarGz('tarino.tar.gz', 'native', {folder: true, native: true})
// tarino.createTarGz('tarino.tar.gz', ['GPL-LICENSE','MIT-LICENSE'])
tarino.extractTarGz('tarino.tar.gz', {native: true, verbose: true})
