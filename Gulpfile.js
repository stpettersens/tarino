'use strict'

const gulp = require('gulp')
const mocha = require('gulp-mocha')
const wait = require('gulp-wait')
const sequence = require('gulp-sequence')
const standard = require('gulp-standard')
const clean = require('gulp-rimraf')

gulp.task('standard', function () {
  return gulp.src('*.js')
  .pipe(standard())
  .pipe(standard.reporter('default', {
    breakOnError: true
  }))
})

gulp.task('test1', function () {
  return gulp.src('tarino.test1.js')
  .pipe(mocha({reporter: 'min', timeout: 100000}))
  .pipe(wait(1500))
})

gulp.task('test2', function () {
  return gulp.src('tarino.test2.js')
  .pipe(mocha({reporter: 'min', timeout: 100000}))
})

gulp.task('clean', function () {
  return gulp.src('*.tar')
  .pipe(clean())
})

gulp.task('test', sequence('standard', 'test1', 'test2'))
