const express = require('express');
const jsonFile = require('jsonFile');
var bodyParser = require('body-parser');
const app = express();
const port = 3000;


app.listen(port, function () {
  console.log('to ouvindo! hahahah!');
});

app.get('/', function(req, res){
  res.send('aiaiaiaiai');
});

app.post('/data', function(req, res){
  //recebe os dados vindos do esp

});
