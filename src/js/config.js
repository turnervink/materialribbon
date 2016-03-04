module.exports = [
	
	{
		"type": "section",
		"items": [
			{
				"type": "heading",
				"id": "localization-heading",
				"defaultValue": "Localization",
				"size": 5
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
				"size": 5
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
			}
		]
	},
			
	{
  "type": "submit",
  "defaultValue": "Save"
	}
	
];