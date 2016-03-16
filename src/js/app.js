var Clay = require('clay');
var clayConfig = require('config');
var customClay = require('custom-clay');
var clay = new Clay(clayConfig, customClay, {AutoHandleEvents: false});
var city = "";

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
	var url;
	
	if (pos !== undefined) {
		console.log("Using pos to create URL");
		// Construct URL
		url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=2874bea34ea1f91820fa07af69939eea';

		console.log("Lat is " + pos.coords.latitude);
		console.log("Lon is " + pos.coords.longitude);
		console.log('URL is ' + url);
	} else {
		console.log("Using city to create URL");
		// Construct URL
  	url = 'http://api.openweathermap.org/data/2.5/weather?&q=' + city + '&appid=2874bea34ea1f91820fa07af69939eea';
	
		console.log('URL is ' + url);
	}
	
  

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET',
    function(responseText) {
      console.log("Parsing JSON");

      var json = JSON.parse(responseText); // Parse JSON response
      console.log(JSON.parse(responseText));
			
			var cityinresponse = json.name;
			console.log("City in response is " + cityinresponse);

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
	if (city === "") {
		console.log("No city defined, using GPS position");
		navigator.geolocation.getCurrentPosition(
			locationSuccess,
			locationError,
			{timeout: 15000, maximumAge: 60000}
		);
	} else {
		console.log("Fetching weather for " + city);
		locationSuccess();
	}
}

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS Ready!');
	city = localStorage.city;

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

    if (e.payload.KEY_WEATHERID === 0) { // If KEY_CONDITIONS exists in the appmessage
      console.log('Fetching weather');
      getWeather(); // Fetch the weather
    }
  }
);

//===== Config =====//

Pebble.addEventListener('showConfiguration', function(e) {
	console.log("Showing configuration page");
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e && !e.response) { 
		console.log("No response from config page!");
    return; 
  }
	
	console.log("Configuration page returned: " + e.response);

  // Get the keys and values from each config item
  var dict = clay.getSettings(e.response);
	var response = JSON.parse(e.response);
	console.log(dict);
	
	city = response.cityName.value;
	console.log("Entered city is " + city);
	localStorage.city = city;

  // Send settings values to watch side
  Pebble.sendAppMessage(dict, function(e) {
    console.log('Sent config data to Pebble');
  }, function(e) {
    console.log('Failed to send config data!');
    console.log(JSON.stringify(e));
  });
});

//===== Old Config=====//

/*Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://turnervink.github.io/materialribbon-config/';

  console.log('Showing configuration page: ' + url);

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));

  console.log('Configuration page returned: ' + JSON.stringify(configData));

  if (configData.useCelsius >= 0) { // If we have received the correct data (not sure why we wouldn't, but who knows?)
    // Send all keys to Pebble
		console.log("Sending config settings");
    Pebble.sendAppMessage({
      useCelsius: configData.useCelsius ? 1 : 0,
			showWeather: configData.showWeather ? 1 : 0,
      vibeDisconnect: configData.vibeDisconnect ? 1 : 0,
      vibeConnect: configData.vibeConnect ? 1 : 0,
      langSel: configData.langSel,
			colourScheme: parseInt(configData.colourScheme),
			updateTime: parseInt(configData.updateTime),
			battNumber: parseInt(configData.battNumber),
			showSteps: configData.showSteps ? 1 : 0,
			stepGoal: parseInt(configData.stepGoal)
    }, function(e) {
      console.log('Send successful!');
			console.log(e);
    }, function(e) {
      console.log('Send failed!');
			console.log(e);
    });
  }
});*/
