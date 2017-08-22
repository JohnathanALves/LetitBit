const express = require('express');
const app = express();
const port = 80;

app.listen(port, function () {
  console.log('to ouvindo! hahahah!');
});

app.get('/', function(req, res){
  res.send('aiaiaiaiai');
});
