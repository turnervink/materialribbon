module.exports = [
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"id": "main-heading",
				"defaultValue": "Material Ribbon",
				"size": 1
			},
			{
				"type": "heading",
				"id": "dev-heading",
				"defaultValue": "<p style='text-align:center;'>by Turner Vink<br> <a href='http://www.turnervink.com' style='color:#FF4500; text-decoration:none;'>www.turnervink.com</a></p>",
				"size": 6
			},
		]
	},
	
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"id": "localization-heading",
				"defaultValue": "Localization",
				"size": 4
			},
			{
				"type": "select",
				"appKey": "langSel",
				"defaultValue": "en",
				"label": "Language",
				"options": [
					{
						"label": "English",
						"value": "en"
					},
					{
						"label": "Français",
						"value": "fr"
					},
					{
						"label": "Español",
						"value": "es"
					},
					{
						"label": "Deutsch",
						"value": "de"
					},
					{
						"label": "русский язык",
						"value": "ru"
					},
					{
						"label": "Italiano",
						"value": "it"
					},
					{
						"label": "Português",
						"value": "po"
					},
					{
						"label": "український",
						"value": "uk"
					}
				]
			}
		]
	},
	
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"id": "appearance-heading",
				"defaultValue": "Appearance",
				"size": 4
			},
			{
				"type": "select",
				"appKey": "colourScheme",
				"defaultValue": "classic",
				"label": "Colour Scheme",
				"options": [
					{
						"label": "Classic",
						"value": "classic"
					},
					{
						"label": "Pop Tart",
						"value": "poptart"
					},
					{
						"label": "Lemon Splash",
						"value": "lemonsplash"
					},
					{
						"label": "Frozen Yoghurt",
						"value": "froyo"
					},
					{
						"label": "Watermelon",
						"value": "watermelon"
					},
					{
						"label": "Popsicle",
						"value": "popsicle"
					}
				]
			},
			{
				"type": "select",
				"appKey": "battNumber",
				"defaultValue": "icon",
				"label": "Battery Level",
				"options": [
					{
						"label": "As an icon",
						"value": "icon"
					},
					{
						"label": "As a number",
						"value": "number"
					}
				]
			}
		]
	},
	
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"id": "weather-heading",
				"defaultValue": "Weather",
				"size": 4
			},
			{
				"type": "toggle",
				"appKey": "showWeather",
				"label": "Show Weather",
				"defaultValue": true
			},
			{
				"type": "toggle",
				"appKey": "useCelsius",
				"label": "Use Celsius",
				"defaultValue": false
			},
			{
				"type": "select",
				"appKey": "updateTime",
				"defaultValue": "60",
				"label": "Update Frequency",
				"options": [
					{
						"label": "60 minutes",
						"value": "60"
					},
					{
						"label": "30 minutes",
						"value": "30"
					}
				]
			},
			{
				"type": "input",
				"appKey": "cityName",
				"defaultValue": "",
				"label": "City",
				"description": "Enter a city to use for weather updates, or leave blank to use GPS location.",
				"attributes": {
					"placeholder": "",
			}
			}
		]
	},
	
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"id": "health-heading",
				"defaultValue": "Health",
				"size": 4
			},
			{
				"type": "toggle",
				"appKey": "showSteps",
				"label": "Show step goal",
				"defaultValue": true
			},
			{
				"type": "slider",
				"appKey": "stepGoal",
				"label":"Step goal",
				"min": 1000,
				"max": 10000,
				"step": 1000,
				"description": "Step goal is represented by the shadow of the horizontal bar"
			}
		]
	},
	
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"id": "bluetooth-heading",
				"defaultValue": "Bluetooth Status",
				"size": 4
			},
			{
				"type": "toggle",
				"appKey": "vibeConnect",
				"label": "Vibrate on Bluetooth Reconnect",
				"defaultValue": true
			},
			{
				"type": "toggle",
				"appKey": "vibeDisconnect",
				"label": "Vibrate on Bluetooth Disconnect",
				"defaultValue": true
			},
		]
	},
			
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"id": "paypal-heading",
				"defaultValue": "<h4 style='text-align:center;'>Before you tap save, consider donating a dollar or two through PayPal by tapping below.</h4>",
				"size": 4
			},
			{
			"type": "button",
			"defaultValue": "<a style='color:white; text-decoration:none;' href='https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=HHTTNFJDU82TC'>Donate</a>"
			}
		]
	},
			
	{
  "type": "submit",
  "defaultValue": "Save"
	}
	
];