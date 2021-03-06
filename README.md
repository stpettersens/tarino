### tarino
> :package: A tar implementation with Node.js.

[![Build Status](https://travis-ci.org/stpettersens/tarino.png?branch=master)](https://travis-ci.org/stpettersens/tarino)
[![Build status](https://ci.appveyor.com/api/projects/status/d46htijspcy1j2ln?svg=true)](https://ci.appveyor.com/project/stpettersens/tarino)
[![js-standard-style](https://img.shields.io/badge/code%20style-standard-brightgreen.svg)](https://github.com/feross/standard)
[![npm version](https://badge.fury.io/js/ssp-tarino.svg)](http://npmjs.com/package/ssp-tarino)
[![Dependency Status](https://david-dm.org/stpettersens/tarino.png?theme=shields.io)](https://david-dm.org/stpettersens/tarino) [![Development Dependency Status](https://david-dm.org/stpettersens/tarino/dev-status.png?theme=shields.io)](https://david-dm.org/stpettersens/tarino#info=devDependencies)

##### Install

- `npm install ssp-tarino --production`

##### Tests

If you want to run the tests, install without `--production` flag
and then run:

- `npm test`

##### Usage

```js
'use strict'

const tarino = require('ssp-tarino')
tarino.createTar('control.tar', 'control') // --> Create regular Tar.
tarino.extractTar('control.tar') // --> Extract regular Tar.

tarino.createTarGz('control.tar.gz', 'control') // --> Create gzipped Tar.
tarino.extractTarGz('control.tar.gz') // --> Extract gzipped Tar.
```

##### Other requirements

If you wish to make use of the native-addon. Your system will need:

* [node-gyp](https://github.com/nodejs/node-gyp)
* Python 2.7+ (not 3.0+)
* A C++11 compiler (e.g. g++ 4.8+)

##### License

Tarino is dual licensed under the GNU General Public License and MIT licenses respectively.
