'use strict'

const tarino = require('./tarino')

tarino.createTarGz('tarino.tar.gz', 'tarino.cc', 
{root: 'native', flat: true, native: true})
// tarino.createTar('tarino.tar', [ 'README.md', 'package.json' ])
// tarino.createTar('tarino.tar', 'native', {folder: true})
// tarino.createTarGz('tarino.tar.gz', 'native', {folder: true, native: true})
