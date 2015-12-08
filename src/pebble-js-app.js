var current_settings;

/*  ****************************************** Weather Section **************************************************** */

function getWeather(woeid) {  
  
  var temperature;
  var icon;
  var city;
  
   var query = 'select item.condition, location.city from weather.forecast where woeid =  ' + woeid + ' and u="' + (current_settings.temperatureFormat === 0? 'f' : 'c') + '"';
  //console.log ("\n++++ I am inside of 'getWeather()' preparing query:" + query);
  var url = 'https://query.yahooapis.com/v1/public/yql?q=' + encodeURIComponent(query) + '&format=json&env=store://datatables.org/alltableswithkeys';
  //console.log ("\n++++ I am inside of 'getWeather()' preparing url:" + url);
  // Send request to Yahoo
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    var json = JSON.parse(this.responseText);
    temperature = parseInt(json.query.results.channel.item.condition.temp);
    //console.log  ("\n++++ I am inside of 'getWeather()' callback. Temperature is " + temperature);
    
    icon = parseInt(json.query.results.channel.item.condition.code);
    //console.log  ("\n++++ I am inside of 'getWeather()' callback. Icon code: " + icon);
    
    city = json.query.results.channel.location.city;
   
    
    var dictionary = {
      'KEY_WEATHER_CODE': icon,
      'KEY_WEATHER_TEMP': temperature,
      'KEY_CITY_NAME': city
    };
    
    // Send to Pebble
    //console.log  ("\n++++ I am inside of 'getWeather()' callback. About to send message to Pebble");
    Pebble.sendAppMessage(dictionary,
    function(e) {
      //console.log ("\n++++ I am inside of 'Pebble.sendAppMessage()' callback. Weather info sent to Pebble successfully!");
    },
    function(e) {
      //console.log ("\n++++ I am inside of 'Pebble.sendAppMessage()' callback. Error sending weather info to Pebble! = " + JSON.stringify(e));
    }
    );
  };
  xhr.open('GET', url);
  xhr.send();
}



// on location success querying woeid and getting weather
function locationSuccess(pos) {
  // We neeed to get the Yahoo woeid first
  var woeid;
 
  
  var query = 'select * from geo.placefinder where text="' +
    pos.coords.latitude + ',' + pos.coords.longitude + '" and gflags="R"';
  //console.log ("\n++++ I am inside of 'locationSuccess()' preparing query:" + query);
  var url = 'https://query.yahooapis.com/v1/public/yql?q=' + encodeURIComponent(query) + '&format=json';
  //console.log ("\n++++ I am inside of 'locationSuccess()' preparing URL: " + url);
  // Send request to Yahoo
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    var json = JSON.parse(this.responseText);
    woeid = json.query.results.Result.woeid;
    //console.log ("\n++++ I am inside of 'locationSuccess()', woeid received:" + woeid);
    getWeather(woeid);
  };
  xhr.open('GET', url);
  xhr.send();

}




function locationError(err) {
  //console.log ("\n++++ I am inside of 'locationError: Error requesting location!");
}


// Get Location lat+lon
function getLocation() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}


// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
      
    //reading current stored settings
    try {
       current_settings = JSON.parse(localStorage.getItem('current_settings'));
    } catch(ex) {
       current_settings = null; 
    }  
    
     if (current_settings === null) {
         current_settings = {
             temperatureFormat: 0,
             mainClock: 0,
             bluetoothAlert: 2,
             secondHand: 1,
             locationService: 0,
             woeid: 0,
             secondaryInfoType: 1,
             timeZoneName: '',
             timeSeparator: 0 
         };
     }
    
    //console.log ("\n++++ I am inside of 'Pebble.addEventListener('ready'): PebbleKit JS ready!");
    var dictionary = {
      "KEY_JSREADY": 1
    };

    // Send to Pebble, so we can load units variable and send it back
    setTimeout(function() { // doing this on TIMEOUT! allowing JS to get ready
        //console.log ("\n++++ I am inside of 'Pebble.addEventListener('ready') about to send Ready message to phone");
        Pebble.sendAppMessage(dictionary,
          function(e) {
            //console.log ("\n++++ I am inside of 'Pebble.sendAppMessage() callback: Ready notice sent to phone successfully!");
          },
          function(e) {
            //console.log ("\n++++ I am inside of 'Pebble.sendAppMessage() callback: Error ready notice to Pebble! = " +  JSON.stringify(e));
          }
        ); 
    }, 2000); 
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    //console.log ("\n++++ I am inside of 'Pebble.addEventListener('appmessage'): AppMessage received");
    
    if (current_settings.locationService == 1) { // for manual location - request weather right away
        //console.log ("\n++++ I am inside of 'Pebble.addEventListener('appmessage'): Requesting weather by WOEID");
        getWeather(current_settings.woeid);
    } else {
       //console.log ("\n++++ I am inside of 'Pebble.addEventListener('appmessage'): Requesting automatic location");
       getLocation();  // for automatic location - get location
    }
    
  }                     
);

/*    ******************************************************************** Config Section ****************************************** */ 


Pebble.addEventListener("showConfiguration",
  function(e) {
   
    //Load the remote config page
    Pebble.openURL("http://codecorner.galanter.net/pebble/pebstyle_config.html");
    
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    
    if (e.response !== '' && e.respnse != 'undefined' && e.response != 'CANCELLED') {
      
      //console.log('resonse: ' + decodeURIComponent(e.response));
      
      //Get JSON dictionary
      var settings = JSON.parse(decodeURIComponent(e.response));
      
      //console.log(settings);
      
            
      
      //Send to Pebble
      var app_message_json = {};

      // preparing app message
      app_message_json.KEY_MAIN_CLOCK = settings.mainClock;
      app_message_json.KEY_SECOND_HAND = settings.secondHand;
      app_message_json.KEY_BLUETOOTH_ALERT = settings.bluetoothAlert;
      app_message_json.KEY_LOCATION_SERVICE = settings.locationService;
      app_message_json.KEY_WEATHER_INTERVAL = settings.weatherInterval;
      app_message_json.KEY_SECONDARY_INFO_TYPE = settings.secondaryInfoType;
      app_message_json.KEY_TIMEZONE_NAME = settings.timeZoneName;
      app_message_json.KEY_TIME_SEPARATOR = settings.timeSeparator;
      app_message_json.KEY_JS_TIMEZONE_OFFSET = new Date().getTimezoneOffset(); // for SDK2 - add current timezone offset since C's time() returns local
      
            
      // only storing and passing to pebble temperature format if it changed, because it will cause Pebble to reissue weather AJAX
      if (current_settings.temperatureFormat != settings.temperatureFormat) {
        app_message_json.KEY_TEMPERATURE_FORMAT = settings.temperatureFormat;
      }
      
      // storing new settings
      localStorage.setItem('current_settings', JSON.stringify(settings));
      current_settings = settings;
      
      //console.log ("\n++++ I am inside of 'Pebble.addEventListener(webviewclosed). About to send settings to the phone");
      Pebble.sendAppMessage(app_message_json,
        function(e) {
          //console.log ("\n++++ I am inside of 'Pebble.addEventListener(webviewclosed) callback' Data sent to phone successfully!");
        },
        function(e) {
          //console.log ("\n++++ I am inside of 'Pebble.addEventListener(webviewclosed) callback' Data sent to phone failed! = " +  JSON.stringify(e));
        }
     );
      
    }
  }
);