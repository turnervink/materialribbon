var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
	console.log("Opening request");
  xhr.open(type, url);
	xhr.timeout = 10000;
	xhr.ontimeout = function() {
		Pebble.sendAppMessage({'0': 404
		}, function(e) {
				console.log('Sent timeout message!');
		}, function(e) {
				console.log('Failed to send timeout message');
		});
	};
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=2874bea34ea1f91820fa07af69939eea';
  
  console.log("Lat is " + pos.coords.latitude);
  console.log("Lon is " + pos.coords.longitude);
  console.log('URL is ' + url);

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      console.log("Parsing JSON");
      
      var json = JSON.parse(responseText); // Parse JSON response
      console.log(JSON.parse(responseText));

      var temperature = Math.round(((json.main.temp - 273.15) * 1.8) + 32); // Convert from Kelvin to Fahrenheit
      console.log("Temperature in Fahrenheit is " + temperature);
      
      var temperaturec = Math.round(json.main.temp - 273.15); // Convert from Kelvin to Celsius
      console.log("Temperature in Celsius is " + temperaturec);

      // Conditions
      var id = json.weather[0].id;      
      console.log("Weather ID is " + id);
      
      // Assemble weather info into dictionary
      var dictionary = {
        "KEY_TEMP": temperature,
        "KEY_TEMPC": temperaturec,
        "KEY_WEATHERID": id,
      };

      // Send dictionary to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
					console.log(e);
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
					console.log(e);
        }
      );
    }      
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS Ready!');

  Pebble.sendAppMessage({'0': 0
	}, function(e) {
      console.log('Sent ready message!');
  }, function(e) {
      console.log('Failed to send ready message');
			console.log(e);
  });
});

Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    console.log('Message contents: ' + JSON.stringify(e.payload));
		console.log(e.payload.KEY_WEATHERID);

    if (e.payload.KEY_WEATHERID === 0) { // If KEY_CONDITIONS exists in the appmessage
      console.log('Fetching weather');
      getWeather(); // Fetch the weather
    }
  }                     
);

//===== Config =====//

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://4512fc45.ngrok.io';

  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));

  console.log('Configuration page returned: ' + JSON.stringify(configData));

  if (configData.useCelsius >= 0) { // If we have received the correct data (not sure why we wouldn't, but who knows?)
    // Send all keys to Pebble
		console.log("Sending config dict");
		console.log("showWeather: " + configData.showWeather);
    Pebble.sendAppMessage({
      useCelsius: configData.useCelsius ? 1 : 0,
			showWeather: configData.showWeather ? 1 : 0,
      vibeDisconnect: configData.vibeDisconnect ? 1 : 0,
      vibeConnect: configData.vibeConnect ? 1 : 0,
      langSel: configData.langSel,
			colourScheme: parseInt(configData.colourScheme),
			//colourScheme: 5
			updateTime: parseInt(configData.updateTime)
    }, function(e) {
      console.log('Send successful!');
			console.log(e);
    }, function(e) {
      console.log('Send failed!');
			console.log(e);
    });
  }
});