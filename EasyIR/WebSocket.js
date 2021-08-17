alert('Hello World')

var led_on = true;
var connection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
connection.onopen = function()
{
	connection.send('Connect '+new Date());
};

connection.onerror = function (error) 
{
	console.log('Server: ', e.data);
};

connection.onclose = function()
{
	console.log('WebSocket connection closed');
};

function toggleled()
{
	if(led_on)
	{
		connection.send("0");
		led_on = 0;
		document.getElementById('toggleLED').style.backgroundColor = '#00878F';
	}
	else
	{
		connection.send("1");
		led_on = 1;
		document.getElementById('toggleLED').style.backgroundColor = '#999';
	}
}

function getLEDdutycycle()
{
} 

