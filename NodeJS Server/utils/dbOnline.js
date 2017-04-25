var mysql = require('mysql')
  , async = require('async')

var PRODUCTION_DB = 'pendulog'
  , TEST_DB = 'pendulog'
  
exports.MODE_TEST = 'mode_test'
exports.MODE_PRODUCTION = 'mode_production'


var state = {
  pool: null,
  mode: null,
}

  
exports.connect = function(mode, done) {
  state.pool = mysql.createPool({
	connectionLimit		: 10,
    host				: 'localhost',
    user				: 'tom',
    password			: XXXXX,
    database			: mode === exports.MODE_PRODUCTION ? PRODUCTION_DB : TEST_DB
  })
  state.mode = mode
  done()
}


exports.get = function() {
  return state.pool
}

exports.initialise = function () {
	state.pool.query('CREATE TABLE IF NOT EXISTS `data` (`ID` BIGINT UNSIGNED PRIMARY KEY,`TM` BIGINT UNSIGNED,`YW` DECIMAL(6,3),`PT` DECIMAL(6,3),`RL` DECIMAL(6,3),`RL` DECIMAL(6,3),`AX` DECIMAL(6,3),`AY` DECIMAL(6,3),`AZ` DECIMAL(6,3),`QW` DECIMAL(6,3),`QX` DECIMAL(6,3),`QY` DECIMAL(6,3),`QZ` DECIMAL(6,3)),`MX` DECIMAL(6,3),`MY` DECIMAL(6,3),`MZ` DECIMAL(6,3)', function (error, results, fields) {
	  if (error)
		  console.log(error.code);
	});
}


//'ALTER TABLE `data` MODIFY `YAW`,`PITCH`,`ROLL` DECIMAL(3,X)'



exports.lastID = function(done) {
	state.pool.query('SELECT `ID` FROM `data` ORDER BY `ID` DESC LIMIT 1', function (err, id) {
		if (err) return done(err)
		if (id.length > 0)
			return done(null, id[0].ID)
		else 
			return done(null, 0);
	})
}

exports.drop = function(tables, done) {
  var pool = state.pool
  if (!pool) return done(new Error('Missing database connection.'))

  async.each(tables, function(name, cb) {
    pool.query('DELETE * FROM ' + name, cb)
  }, done)
}


exports.fixtures = function(data) {
  var pool = state.pool
  if (!pool) return done(new Error('Missing database connection.'))

  var names = Object.keys(data.tables)
  async.each(names, function(name, cb) {
    async.each(data.tables[name], function(row, cb) {
      var keys = Object.keys(row)
        , values = keys.map(function(key) { return "'" + row[key] + "'" })

      pool.query('INSERT INTO ' + name + ' (' + keys.join(',') + ') VALUES (' + values.join(',') + ')', cb)
    }, cb)
  }, done)
}






