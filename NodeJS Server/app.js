var express = require('express');
var http = require('http');
var url = require('url');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');
var WebSocket = require('ws');
var passport = require('passport');
var Strategy = require('passport-local').Strategy;


var db = require('./utils/db')
var index = require('./routes/index');
var users = require('./routes/users');
var download = require('./routes/download');
//var login = require('./routes/login');
//var config = require('./routes/config');

var app = express();

// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');

app.use(favicon(path.join(__dirname, 'public', 'favicon.ico')));
app.use(logger('dev'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

app.use(require('express-session')({ secret: 'asdfasdfasdf', resave: false, saveUninitialized: false }));


var usr = require('./routes/users');

passport.use(new Strategy(
  function(username, password, cb) {
    usr.findByUsername(username, function(err, user) {
      if (err) { return cb(err); }
      if (!user) { return cb(null, false); }
      if (user.password != password) { return cb(null, false); }
      return cb(null, user);
    });
  }));

passport.serializeUser(function(user, cb) {
  cb(null, user.id);
});

passport.deserializeUser(function(id, cb) {
  usr.findById(id, function (err, user) {
    if (err) { return cb(err); }
    cb(null, user);
  });
});

app.use(passport.initialize());
app.use(passport.session());




app.get('/', function(req,res) {
  path.join(__dirname, '../public', 'index.html')
  res.sendFile('public/index.html');
});

app.get('/debug', function(req,res) {
  res.sendFile('public/debug.html' , { root : __dirname});
});

app.get('/graph', function(req,res) {
  res.sendFile('public/basegraph.html' , { root : __dirname});
});


app.get('/config',
  require('connect-ensure-login').ensureLoggedIn('/login'),
  function(req, res) {
    res.sendFile('views/config.html' , { root : __dirname});;
  });

app.post('/config',
  require('connect-ensure-login').ensureLoggedIn('/login'),
  function(req, res) {
	res.sendFile('views/success.html' , { root : __dirname});;
	req.body.type="newconfig";
	wss.clients.forEach(function each(client) {
		if (client.readyState === WebSocket.OPEN) {
			client.send(JSON.stringify(req.body));
		}
	});
  });  
  

app.get('/login', function(req, res) {
  res.sendFile('public/login.html' , { root : __dirname});
});

app.post('/login', passport.authenticate('local', { successReturnToOrRedirect: '/', failureRedirect: '/login' }));
  
  
app.use('/download',download);


// catch 404 and forward to error handler
app.use(function(req, res, next) {
  var err = new Error('Not Found');
  err.status = 404;
  next(err);
});

// error handler
app.use(function(err, req, res, next) {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get('env') === 'development' ? err : {};

  // render the error page
  res.status(err.status || 500);
  res.render('error');
});


var server = http.createServer(app);

var wss = new WebSocket.Server({ server });

var nextID=1;
var clients = [];
var isInitData = false;
var socketCount = 0;
var initialDataCount=50;
var initialBufferSize=5;


db.connect(db.MODE_TEST, function(err) {
	if (err) {
		console.log('Unable to connect to MySQL.');
		process.exit(1);
	} else {
		db.initialise();
		db.lastID(function(err,result) {
			if(err) 
				console.log(err);
			console.log(result);
			nextID=result+1;
		});
		server.listen(3000, "0.0.0.0");
	}
});





wss.on('connection', function connection(ws) {
	const location = url.parse(ws.upgradeReq.url, true);
	console.log((new Date()) + ' Connection from origin ' + location.origin + '.');
    //var connection = ws.accept(null, ws.origin); 
    console.log((new Date()) + ' Connection accepted.');

    //var index = clients.push(connection) - 1;
	// You might use location.query.access_token to authenticate or share sessions
	// or ws.upgradeReq.headers.cookie (see http://stackoverflow.com/a/16395220/151312)
	
	var points = [];
	socketCount++;
	try {
		var msgOUT={type:'users connected',socketconn:socketCount};
		wss.clients.forEach(function each(client) {
			if (client.readyState === WebSocket.OPEN) {
				client.send(JSON.stringify(msgOUT));
			}
		});
	} catch (e) {return console.error(e);}
	ws.onmessage = function(event) {
		try {
			var msg = JSON.parse(event.data);
		} catch (e) {
			return console.error('Could not parse ' + event.data);
		}
		try {
			switch (msg.type) {
			case 'getconfig':
				db.lastID(function(err,result) {
					if(err) 
						console.log(err);
					nextID=result+1;
					msgOUT = {	type:'config',
						time: (new Date).getTime().toString(),
						id: nextID,
						schema: ['0','1','2','3','4','9','10','11']			//ID TM YW PT RL AX AY AZ
					};
					ws.send(JSON.stringify(msgOUT));
				});
				
				break;
			case 'chunk':
				//console.log(msg.data);
				//forward to all connected clients
				wss.clients.forEach(function each(client) {
					if (client !== ws && client.readyState === WebSocket.OPEN) {
						client.send(JSON.stringify(msg));
					}
				});
				//push into sql
				for(i=0; i < msg.numChunks; i++) {
					try {
						db.get().query('INSERT IGNORE INTO `data` SET ?', msg.data[i], function (error, results, fields) {
							if (error) throw error; 
						});
					} catch (e) {
						console.log(e.code);
						console.log(msg);
						if (e.code=='ER_DUP_ENTRY') {
							console.error(msg);
							console.log('Duplicate index for position ' + i + ' missing');
							console.log('error with ID: ' + msg.data[i].ID) ;
							console.log(msg.data[i]);
						}
					}
				}
				break;
			}
		} catch (e) {
			return console.error(e);
		}
	}	
	// Initial app start, run db query
			// Push results onto the points array
	db.get().query('SELECT * FROM data ORDER BY id DESC LIMIT ?',initialDataCount+initialBufferSize)
		.on('result', function(data){
			points.push(data);
		})		
		.on('end', function(){
			// Only emit points after query has been completed
			msgOUT = {type:'initial points',data:points, buffer:initialBufferSize, schema:['0','1','2','3','4']};
			ws.send(JSON.stringify(msgOUT));
		})
});


module.exports = app;

