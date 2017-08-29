//arquivo de configuracao da base de dados Redis
var redis = require('redis');

module.exports = function (){
	console.log('entrei');
	var client = redis.createClient('12699', 'redis-12699.c14.us-east-1-2.ec2.cloud.redislabs.com'); //creates a new client

	client.on('connect', function() {
    	console.log('conectado na base de dados com sucesso!');
	});
};
