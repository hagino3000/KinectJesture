var http = require('http'),
    io = require('socket.io'),
    zmq = require('zeromq'),
    json = JSON.stringify;


var server = http.createServer();
server.listen(8080);

var webSocket = io.listen(server);
webSocket.on('connection', function(client) {
  console.info('browser connect!!!');
  client.on('disconnect', function() {
    console.info('disconnect');
  });
});

var zsocket = zmq.createSocket('sub');
zsocket.on('message', function(event, data) {
  console.info(data.toString());
  webSocket.broadcast(data);
});
zsocket.on('error', function(err) {
  console.info('error');
  console.info(err.toString('utf8'));
});
zsocket.connect('tcp://127.0.0.1:14444');
zsocket.subscribe('event');


