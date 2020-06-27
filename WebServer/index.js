var http = require('http');
var fs = require('fs');
var express = require('express');
var SerialPort = require('serialport');
var serialport = new SerialPort("COM5", 9600);

serialport.on('data', function (data){
	console.log(data.toString());
})

var app = express();
app.use(express.static(__dirname));

app.get('/', function(req,res){
	res.sendFile('index.html');
})

app.post('/gameover', function(req,res){
	serialport.write("F",function(err){
		console.log(err);
	});
	res.end("It worked");
})

app.listen(3000);