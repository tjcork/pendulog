var express = require('express');
var router = express.Router();
var archiver = require('archiver');
var p = require('path');
var fs = require('fs');
var db = require('../utils/db');
var stringify = require('csv-stringify');


router.get('/', function(req, res) {
	var stringifier = stringify();

	var archive = archiver('zip');

  archive.on('error', function(err) {
    res.status(500).send({error: err.message});
  });

  //on stream closed we can end the request
  archive.on('end', function() {
    console.log('Archive wrote %d bytes', archive.pointer());
  });

  //set the archive name
  res.attachment('PendulogData.zip');

  //this is the streaming magic
  archive.pipe(res);

	archive
			.append(db.get().query('SELECT * FROM data')
					.stream()
					.pipe(stringifier)	
			, { name: 'data.csv' })

	archive.finalize(function(err, bytes) {
	  if (err) {
		throw err;
	  }
	});
});


router.get('/dl', function(req, res) {
	//dl?start=1213123123&end=12341324123
	var starttime = new Date(0); // sets the date to the epoch
	starttime.setUTCMilliseconds(req.query.start);
	var endtime = new Date(0); // sets the date to the epoch
	endtime.setUTCMilliseconds(req.query.end);
	
	var stringifier = stringify({header:true});

	var archive = archiver('zip');

  archive.on('error', function(err) {
    res.status(500).send({error: err.message});
  });

  //on stream closed we can end the request
  archive.on('end', function() {
    console.log('Archive wrote %d bytes', archive.pointer());
  });

  //set the archive name
  res.attachment('PendulogData.zip');

  archive.pipe(res);

	try {
	archive.append(db.get().query('SELECT * FROM `data` WHERE `TM` BETWEEN ? AND ? ORDER BY ID DESC',[req.query.start,req.query.end])
					.stream()
					.pipe(stringifier)
				
		, { name: 'data.csv' })
	} catch (e) {
		console.log(e);
	}


	archive.finalize(function(err, bytes) {
	  if (err) {
		console.log(err);
		throw err;
	  }
	});

});

module.exports = router;
