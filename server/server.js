var http = require("http");
const express = require('express');
const app = express();
const port = 8080;

const server = http.createServer(app)

listener = server.listen((process.env.PORT || port), (err) => {
  if (err) {
    return console.log('something bad happened', err)
  }

  console.log(`server is listening on ${listener.address().port}`)
})

app.get('/', function(req, res){
  res.send('aiaiaiaiai');
});
