var http = require("http");
const express = require('express');
const app = express();
const port = 8080;

const server = http.createServer(app)

// chamada de uma base de dados
var redis = require('./database.js');

listener = server.listen((process.env.PORT || port), (err) => {
  if (err) {
    return console.log('something bad happened', err);
  }

  console.log(`server is listening on ${listener.address().port}`)
})

app.get('/', function(req, res){
  res.send('Servidor rodando!');
});

app.post('/send', function(req, res, next){
	// dados de entrada, caso nao sejam enviados serao considerados como zero
	var Moment = require('moment');
	try {
		var accelx = req.body['accelx'];
	} catch (e) {
		var accelx = 0.0;
	}
	try {
		var accely = req.body['accely'];
	} catch (e) {
		var accely = 0.0;
	}
	try {
		var accelz = req.body['accelz'];
	} catch (e) {
		var accelz = 0.0
	}
	try {
		var gyrox = req.body['gyrox'];
	} catch (e) {
		var gyrox =0.0
	}
	try {
		var gyroy = req.body['gyroy'];
	} catch (e) {
		var gyroy = 0.0
	}
	try {
		var gyroz = req.body['gyroz'];
	} catch (e) {
		var gyroz = 0.0
	}
	try {
		var temp = req.body['temp'];
	} catch (e) {
		var temp = 0.0
	}

	// lets get the system time for registring the input times
	Moment.locale('pt-BR');
	var time = Moment().utc();
	console.log(time.format());
	res.send(time.format());

});
