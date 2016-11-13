var amqp = require('amqplib/callback_api');
var msgpack = require("msgpack-lite");

var args = process.argv.slice(2);
var broker = args[0];
var entity_name = args[1];

if (args.length == 0) {
  console.log("Usage: $ node client-find-entities.js <broker-address> <entity-name>");
  process.exit(1);
}

amqp.connect('amqp://admin:admin@'+broker+':5672/', function(err, conn) {
  conn.createChannel(function(err, ch) {
    //
    var entity   = 'is';
    var exchange = 'services';
    var topic    = entity + '.find_entities';

    ch.assertQueue('', {exclusive: true}, function(err, q) {
      
      ch.bindQueue(q.queue, exchange, q.queue);

      var corr = generateUuid();

      console.log(' [x] Requesting entity: %s', entity_name);

      ch.consume(q.queue, function(msg) {
        if (msg.properties.correlationId == corr) {
          var reply = msgpack.decode(Buffer(msg.content));
          console.log(reply);
          //console.log(' [.] Got %s', msg.content.toString());
          setTimeout(function() { conn.close(); process.exit(0) }, 1000);
        }
      }, {noAck: true});

      ch.publish(exchange, topic, new Buffer(msgpack.encode(entity_name)),
                                  { correlationId: corr, replyTo: q.queue, 
                                    contentType: "application/msgpack" });
    });
  });
});

function generateUuid() {
  return Math.random().toString() +
         Math.random().toString() +
         Math.random().toString();
}