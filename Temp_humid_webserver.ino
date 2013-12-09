/* YourDuino Ethernet Temperature/humidity Web Server
 Reads a DHT11 Sensor
 Outputs a web page with Temperature and Humidity
 terry@yourduino.com 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * DHT11 Sensor connected to Pin 2
 
 Based on code by David A. Mellis & Tom Igoe
 
 */
/*-----( Import needed libraries )-----*/
#include <SPI.h>
#include <Ethernet.h>
#include <dht11.h>
#include <Wire.h>

/*-----( Declare Constants and Pin Numbers )-----*/
#define DHT11PIN 2  // The Temperature/Humidity sensor

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

/*-----( Declare objects )-----*/
IPAddress ip(192,168,1, 120);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(8082);

dht11 DHT11;  //The Sensor Object
/*-----( Declare Variables )-----*/



void setup()   /****** SETUP: RUNS ONCE ******/ 
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}//--(end setup )---


void loop()   /*----( LOOP: RUNS OVER AND OVER AGAIN )----*/
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connnection: close");
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // add a meta refresh tag, so the browser pulls again every 5 seconds:
          client.println("<meta http-equiv=\"refresh\" content=\"5\">");
          client.println("<center>");
          client.print("<strong>Brad's Shed Temperature</strong>");   
          client.println("<br />");    

          /*----(Get sensor reading, calculate and print results )-----------------*/

          int chk = DHT11.read(DHT11PIN);

          Serial.print("Read sensor: ");
          switch (chk)
          {
          case 0: 
            Serial.println("OK"); 
            break;
          case -1: 
            Serial.println("Checksum error"); 
            break;
          case -2: 
            Serial.println("Time out error"); 
            break;
          default: 
            Serial.println("Unknown error"); 
            break;
          }  

          client.print("Temperature (C): ");
          client.println((float)DHT11.temperature, 1);  
          client.println("<br />");  
          
          //client.print("Temperature (F): ");
          //client.println(Fahrenheit(DHT11.temperature), 1);
          //client.println("<br />");  
          
          client.print("Humidity (%): ");
          client.println((float)DHT11.humidity, 0);  
          client.println("<br />");   

          //client.print("Temperature (K): ");
          //client.println(Kelvin(DHT11.temperature), 1);
          //client.println("<br />");   

          client.print("Dew Point (C): ");
          client.println(dewPoint(DHT11.temperature, DHT11.humidity));
          client.println("<br />");   

          client.print("Dew PointFast (C): ");
          client.println(dewPointFast(DHT11.temperature, DHT11.humidity));
          client.println("<br />");   


          /*--------( End Sensor Read )--------------------------------*/
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
} // END Loop

/*-----( Declare User-written Functions )-----*/
//
//Celsius to Fahrenheit conversion
double Fahrenheit(double celsius)
{
  return 1.8 * celsius + 32;
}

//Celsius to Kelvin conversion
double Kelvin(double celsius)
{
  return celsius + 273.15;
}

// dewPoint function NOAA
// reference: http://wahiduddin.net/calc/density_algorithms.htm 
double dewPoint(double celsius, double humidity)
{
  double A0= 373.15/(273.15 + celsius);
  double SUM = -7.90298 * (A0-1);
  SUM += 5.02808 * log10(A0);
  SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
  SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
  SUM += log10(1013.246);
  double VP = pow(10, SUM-3) * humidity;
  double T = log(VP/0.61078);   // temp var
  return (241.88 * T) / (17.558-T);
}

// delta max = 0.6544 wrt dewPoint()
// 5x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity/100);
  double Td = (b * temp) / (a - temp);
  return Td;
}
/* ( THE END ) */

