var WebSocket = require('ws');
var net = require('net');


var ws = new WebSocket('http://big.cs.bris.ac.uk:3000');
var client = new net.Socket();

var server = net.createServer();  

server.listen(4014, function() {  
  console.log('server listening to %j', server.address());
});




ws.on('open', function open() {
	console.log('connected');
	
	var buffer = [];
	sampleFrequency=20;
	 		
	ws.on('message', function incoming(msg, flags) {
		data=JSON.parse(msg);
		if (data.type=='chunk') {
			for (var i = 0; i < data.numChunks; i++) {
				buffer.push(data.data[i]);
			}
			if (buffer.length > 30) buffer = buffer.slice(-10);
		}  
	});
	
	
server.on('connection', function(conn) {
	dataupdate = setInterval( function() {
		if (buffer.length > 0) {
			var data = buffer.shift();
			conn.write(data.TM + '\t' + data.YW + '\t' + data.PT + '\t' + data.RL + '\t\n');
		}
	},1000/sampleFrequency);
	
	conn.once('close', function() {
		clearInterval(dataupdate);
	});
});

});

ws.on('close', function close() {
	clearInterval(dataupdate);
	console.log('disconnected');
});


//SERIAL PORT VERSION
// REQUIRES VIRTUAL SERIAL PORT EMULATOR

/*
var SerialPort = require('serialport');
var WebSocket = require('ws');

var ws = new WebSocket('http://big.cs.bris.ac.uk:3000');



ws.on('open', function open() {
	console.log('connected');
	
	var buffer = [];
	sampleFrequency=20;
	 		
	ws.on('message', function incoming(msg, flags) {
		data=JSON.parse(msg);
		if (data.type=='chunk') {
			for (var i = 0; i < data.numChunks; i++) {
				buffer.push(data.data[i]);
			}
		}  
	});
  
  
	var port = new SerialPort('COM21');
	
	port.on('open', function() {
		dataupdate = setInterval( function() {
			if (buffer.length > 0) {
				var data = buffer.shift();
				port.write(data.TM + '\t' + data.YW + '\t' + data.PT + '\t' + data.RL + '\t\n');
			}
		},1000/sampleFrequency);
	});


	// open errors will be emitted as an error event
	port.on('error', function(err) {
	  console.log('Error: ', err.message);
	})
  
});

ws.on('close', function close() {
	clearInterval(dataupdate);
  console.log('disconnected');
});

*/
