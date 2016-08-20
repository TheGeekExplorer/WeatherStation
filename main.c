
/** 
 * AirPressure, Altitude, and Temperature Sensor 
 */
  #include <SFE_BMP180.h>
  #include <Wire.h>

  SFE_BMP180 pressure;
  #define ALTITUDE 88.0 // Altitude of SparkFun's HQ in Boulder, CO. in meters
  double tc0,tf0,am,af,pmb,phg,prmb,prhg;
  
  /**
   * Function to read values
   */
  char * readAirPressure ()
  {
    char status;
    double T,P,p0;
    
    // If you want to measure altitude, and not pressure, you will instead need
    // to provide a known baseline pressure. This is shown at the end of the sketch.

    // You must first get a temperature measurement to perform a pressure reading.
    
    // Start a temperature measurement:
    // If request is successful, the number of ms to wait is returned.
    // If request is unsuccessful, 0 is returned.

    status = pressure.startTemperature();
    if (status != 0)
    {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Get the temperature in C and F
      tc0 = T;
      tf0 = ((9.0/5.0)*T+32.0);
      
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
      // Wait for the measurement to complete:
      delay(status);

      // Retrieve the completed pressure measurement:
      // Note that the measurement is stored in the variable P.
      // Note also that the function requires the previous temperature measurement (T).
      // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
      // Function returns 1 if successful, 0 if failure.

      status = pressure.getPressure(P,T);
      if (status != 0)
      {
        // Get Air Pressure
        pmb = P;          // Millibar
        phg = (P*0.0295333727); // HG

        // The pressure sensor returns abolute pressure, which varies with altitude.
        // To remove the effects of altitude, use the sealevel function and your current altitude.
        // This number is commonly used in weather reports.
        // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
        // Result: p0 = sea-level compensated pressure in mb

        p0 = pressure.sealevel(P,ALTITUDE); // we're at 1655 meters (Boulder, CO)
        prmb = p0;        // Millibar
        prhg = (p0*0.0295333727); // HG

        // On the other hand, if you want to determine your altitude from the pressure reading,
        // use the altitude function along with a baseline pressure (sea-level or other).
        // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
        // Result: a = altitude in m.

        am = pressure.altitude(P,p0);    // Altitude in Meters
        af = (am*3.28084);               // Altitude in Feet
        
      }
      else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
    }
    else Serial.println("error starting temperature measurement\n");
  }

  

/**
 * Thermistor
 */
  #include <math.h>
  // which analog pin to connect
  #define THERMISTORPIN 0         
  // resistance at 25 degrees C
  #define THERMISTORNOMINAL 15000      
  // temp. for nominal resistance (almost always 25 C)
  #define TEMPERATURENOMINAL 25   
  // how many samples to take and average, more takes longer
  // but is more 'smooth'
  #define NUMSAMPLES 5
  // The beta coefficient of the thermistor (usually 3000-4000)
  #define BCOEFFICIENT 3740
  // the value of the 'other' resistor
  #define SERIESRESISTOR 15000
  
  int samples[NUMSAMPLES];
  double tc1,tf1;
  
  // Read the Thermistor Temperature
  int readThermistor() {
    uint8_t i;
    float average;
   
    // take N samples in a row, with a slight delay
    for (i=0; i< NUMSAMPLES; i++) {
     samples[i] = analogRead(THERMISTORPIN);
     delay(10);
    }
   
    // average all the samples out
    average = 0;
    for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
    }
    average /= NUMSAMPLES;
   
    // convert the value to resistance
    average = 1023 / average - 1;
    average = SERIESRESISTOR / average;
   
    float steinhart;
    steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
    steinhart = log(steinhart);                  // ln(R/Ro)
    steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
    steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart;                 // Invert
    steinhart -= 273.15;                         // convert to C
   
    tc1 = steinhart;         // Temperature C
    tf1 = ((9.0/5.0)*tc1+32.0);      // Temperature F
  }

  
/**
 * Sunlight Sensor
 */
  #include <math.h>
  // which analog pin to connect
  #define SUNLIGHTPIN 1
  
  double sr,sd;
  
  // Read sunlight sensor
  int readSunlight ()
  {
  sr = analogRead(SUNLIGHTPIN);
  sd = ((sr/100)*1.0);
  }
  

  
/**
 * Web Client
 */
	#include <SPI.h>
	#include <Ethernet.h>

	// Enter a MAC address for your controller below.
	// Newer Ethernet shields have a MAC address printed on a sticker on the shield
	byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
	// if you don't want to use DNS (and reduce your sketch size)
	// use the numeric IP instead of the name for the server:
	//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
	char server[] = "weatherannounce.sourceflare.uk";    // name address for Google (using DNS)

	// Set the static IP address to use if the DHCP fails to assign
	IPAddress ip(192, 168, 1, 170);

	// Initialize the Ethernet client library
	// with the IP address and port of the server
	// that you want to connect to (port 80 is default for HTTP):
	EthernetClient client;

	/** 
	 * Send data to Web Server
	 */
	int sendData ()
	{
	
		// if the server's disconnected, stop the client:
		client.stop();
		
		// give the Ethernet shield a second to initialize:
		Serial.println("connecting...");

		// if you get a connection, report back via serial:
		if (client.connect(server, 80)) {
			Serial.println("connected");
			// Make a HTTP request:
			client.print("GET /?status=1");
			
			// Print readings into query stringtc0
			client.print("&tc0="); client.print(tc0);
			client.print("&tc1="); client.print(tc1);
			client.print("&tf0="); client.print(tf0);
			client.print("&tf1="); client.print(tf1);
			client.print("&pmb="); client.print(pmb);
			client.print("&phg="); client.print(phg);
			client.print("&prmb="); client.print(prmb);
			client.print("&am="); client.print(am);
			client.print("&af="); client.print(af);
			client.print("&sr="); client.print(sr);
			client.print("&sd="); client.print(sd);
			
			client.println(" HTTP/1.1");
			client.println("Host: weatherannounce.sourceflare.uk");
			client.println("Connection: close");
			client.println();
		} else {
			// if you didn't get a connection to the server:
			Serial.println("connection failed");
		}
	}
  
  
/**
 * Setup
 */
  void setup()
  {
    Serial.begin(9600);

    // BMP180 Init
    pressure.begin();
	
	// start the Ethernet connection:
	if (Ethernet.begin(mac) == 0) {
		Serial.println("Failed to configure Ethernet using DHCP");
		// try to congifure using IP address instead of DHCP:
		Ethernet.begin(mac, ip);
	}
  }

  
/**
 * Main loop
 */
  void loop()
  {
    // Read sensors!
    readAirPressure();
    readThermistor();
    readSunlight();
    
/**	
    Serial.println("--------------------------");
    Serial.println("----- Weather Report -----");
    Serial.println("--------------------------");
    Serial.println();
    
    // Temperature in C from two sources
    Serial.println(" ----- Temperature -----"); 
    Serial.print(tc0); Serial.print("C"); Serial.print(" / "); Serial.print(tc1); Serial.println("C");
    Serial.print(tf0); Serial.print("F"); Serial.print(" / "); Serial.print(tf1); Serial.println("F");
    Serial.println();
    
    // Air Pressure
    Serial.println(" ----- Air Pressure -----"); 
    Serial.print("Calculated: "); Serial.print(pmb); Serial.print("mb"); Serial.print(" / "); Serial.print(phg); Serial.println("hg");
    Serial.print("Relative Sea-Level: "); Serial.print(prmb); Serial.print("mb"); Serial.print(" / "); Serial.print(prhg); Serial.println("hg");
    Serial.println();
    
    // Sunlight
    Serial.println(" ----- Sunlight -----"); 
    Serial.print("Raw: "); Serial.println(sr);
    Serial.print("Decimal: "); Serial.println(sd);
    Serial.println();
    
    //New line spacer
    Serial.println();
**/
	
	
	// Send Data to Web Server
	sendData();
	delay(5000);

	
/**
	int lk=0;
	for (lk=0; lk<500; lk++) {
		if (client.available()) {
			char c = client.read();
			Serial.print(c);
		}
	}
**/
	
	client.stop();
    delay(60000);
  }
