'use strict'

const tarino = require('./tarino')

tarino.createTar('tarino.tar', 'native', {folder: true})
// tarino.createTar('tarino.tar', [ 'README.md', 'package.json' ])
// tarino.createTar('tarino.tar', 'native', {folder: true})
// tarino.createTarGz('tarino.tar.gz', 'README.md')
