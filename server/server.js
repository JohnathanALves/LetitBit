const express = require('express');
const app = express();
const port = 3000;

app.listen(port, function () {
  console.log('to ouvindo! hahahah!');
});

app.get('/', function(req, res){
  res.send('aiaiaiaiai');
});
