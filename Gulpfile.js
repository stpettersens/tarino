'use strict'
const gulp = require('gulp')
const mocha = require('gulp-mocha')
const wait = require('gulp-wait')
const sequence = require('gulp-sequence')

gulp.task('test1', function () {
  return gulp.src('tarino.test1.js')
  .pipe(mocha({reporter: 'min', timeout: 100000}))
})

gulp.task('test2', function () {
  return gulp.src('tarino.test2.js')
  .pipe(mocha({reporter: 'min', timeout: 100000}))
})

gulp.task('test', sequence('test1', 'test2'))
