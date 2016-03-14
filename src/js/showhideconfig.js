module.exports = function(minified) {
  var clayConfig = this;
	
	function weatherSection() {
		if (this.get()) {
			clayConfig.getItemByAppKey("useCelsius").show();
			clayConfig.getItemByAppKey("updateTime").show();
			clayConfig.getItemByAppKey("cityName").show();
		} else {
			clayConfig.getItemByAppKey("useCelsius").hide();
			clayConfig.getItemByAppKey("updateTime").hide();
			clayConfig.getItemByAppKey("cityName").hide();
		}
	}
	
	function healthSection() {
		if (this.get()) {
			clayConfig.getItemByAppKey("stepGoal").show();
		} else {
			clayConfig.getItemByAppKey("stepGoal").hide();
		}
	}

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    console.log("Built page");
		var _ = minified._;
  	var $ = minified.$;
  	var HTML = minified.HTML;
		
		var weatherToggle = clayConfig.getItemByAppKey("showWeather");
		weatherSection.call(weatherToggle);
		weatherToggle.on("change", weatherSection);
		
		var stepToggle = clayConfig.getItemByAppKey("showSteps");
		healthSection.call(stepToggle);
		stepToggle.on("change", healthSection);
  });

};