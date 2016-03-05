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
				"defaultValue": "by Turner Vink",
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
						"label": "French",
						"value": "fr"
					},
					{
						"label": "Spanish",
						"value": "es"
					},
					{
						"label": "German",
						"value": "de"
					},
					{
						"label": "Russian",
						"value": "ru"
					},
					{
						"label": "Italian",
						"value": "it"
					},
					{
						"label": "Portuguese",
						"value": "po"
					},
					{
						"label": "Ukrainian",
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
				"type": "select",
				"appKey": "stepGoal",
				"defaultValue": "10000",
				"label": "Step goal",
				"options": [
					{
						"label": "10,000",
						"value": "10000"
					},
					{
						"label": "9,000",
						"value": "9000"
					},
					{
						"label": "8,000",
						"value": "8000"
					},
					{
						"label": "7,000",
						"value": "7000"
					},
					{
						"label": "6,000",
						"value": "6000"
					},
					{
						"label": "5,000",
						"value": "5000"
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
  "type": "submit",
  "defaultValue": "Save"
	}
	
];