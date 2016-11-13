var amqp = require('amqplib/callback_api');
var http = require('http')
var msgpack = require("msgpack-lite");

var args = process.argv.slice(2);
var broker = args[0];

if (args.length == 0) {
  console.log('>> Usage: $ node service-find-entities.js <broker-address>');  
  process.exit(1); 
}

var username = 'admin'
var password = 'admin'
var auth = 'Basic ' + new Buffer(username + ':' + password).toString('base64')

var options = {
  hostname: broker,
  port: 15672,
  path: '/api/exchanges/%2F/services/bindings/source',
  method: 'GET',
  headers: {
    'Authorization': auth
  }
};

const onChannelCreated = (err, ch) => {
  var entity   = 'is';
  var exchange = 'services';
  var topic    = entity + '.find_entities'

  ch.assertExchange(exchange, 'topic', {durable: true});
  ch.assertQueue(topic, {durable: false, noAck: false, arguments: {'x-expires': 60000}});
  ch.prefetch(1);
  ch.bindQueue(topic, exchange, topic);

  console.log(' [x] waiting for requests');
  
  ch.consume(topic, function r(msg) {
    var cur_entity = msgpack.decode(Buffer(msg.content));        

    var request = http.request(options, (reply) => {
      console.log(" [.] %s", cur_entity);
      
      reply.setEncoding('utf8');
      var response = '';
      
      reply.on('data', (chunk) => {
        response += chunk;
      });
      
      reply.on('end', () => {
        var bindings = JSON.parse(response);
        var info_services = [];
        bindings.forEach((binding) => {
          var route = binding.routing_key
          var first = route.indexOf(cur_entity + ".")
          var last = route.indexOf(".info")
          if (first == 0 && last !== -1) {
            info_services.push({ topic: route })
          }
        });

        console.log(info_services)

        if (info_services.length != 0) {
            amqp.connect('amqp://admin:admin@'+broker+':5672/', function(err, conn2) {
            conn2.createChannel(function(err, ch2) {
              ch2.assertQueue('', {exclusive: true}, function(err, q) {
                ch2.bindQueue(q.queue, exchange, q.queue);

                var received = 0;
                ch2.consume(q.queue, function(msg2) {
                  var correlationId = msg2.properties.correlationId;
                  info_services.forEach((service) => {
                    if (service.correlationId == correlationId) {
                      ++received;
                      service.info = msgpack.decode(Buffer(msg2.content));
                    }
                  });

                  if(received == info_services.length) {
                    doReply(ch, msg, info_services);
                  }

                }, {noAck: true});
            
                info_services.forEach((service) => {
                  service.correlationId = generateUuid();
                  //console.log('published: %s',service.topic);
                  ch2.publish(exchange, service.topic, new Buffer(0),
                                  { correlationId: service.correlationId, 
                                    replyTo: q.queue,
                                    contentType: "application/msgpack" });
                });
              });
            });
          });
        } else {
          doReply(ch, msg, info_services);
        }
      });
    });

    request.write('');
    request.end();
  });

}

function doReply(ch, msg, info) {
  rep = [];
  info.forEach((r) => {
    rep.push(r.info);
  });
  var buffer = new Buffer(msgpack.encode(rep));

  ch.sendToQueue( msg.properties.replyTo,
                  buffer,
                  { correlationId: msg.properties.correlationId,
                    contentType: "application/msgpack" });
  ch.ack(msg);
}

amqp.connect('amqp://admin:admin@'+broker+':5672/', function(err, conn) {
  conn.createChannel(onChannelCreated);
});


function generateUuid() {
  return Math.random().toString() +
         Math.random().toString() +
         Math.random().toString();
}