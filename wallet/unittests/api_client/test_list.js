var net = require('net');

var client = new net.Socket();
client.connect(10000, '127.0.0.1', function() {
	console.log('Connected');
	client.write(JSON.stringify(
		{
			jsonrpc: '2.0',
			id: 123,
			method: 'tx_list',
			params: 
			{
				filter: {
					//asset_id: 0,
					//height: 313375,
					//status: 3
				},
				count: 10,
				skip: 0
			}
		}) + '\n');
});

var acc = '';

client.on('data', function(data) {

	acc += data;
	if(data.indexOf('\n') != -1)
	{

	    var res = JSON.parse(acc);

	    console.log("got:", res);

	    client.destroy(); // kill client after server's response
        }
});

client.on('close', function() {
	console.log('Connection closed');
});
