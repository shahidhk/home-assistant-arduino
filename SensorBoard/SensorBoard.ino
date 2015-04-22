/*
 * TMRh20 2014
 * 
 * RF24Ethernet simple web client example
 *
 * RF24Ethernet uses the fine uIP stack by Adam Dunkels <adam@sics.se>
 *
 * In order to minimize memory use and program space:
 * 1. Open the RF24Network library folder
 * 2. Edit the RF24Networl_config.h file
 * 3. Un-comment #define DISABLE_USER_PAYLOADS
 *
 * This example connects to google and downloads the index page
 */



#include <RF24.h>
#include <SPI.h>
#include <RF24Mesh.h>
#include <RF24Network.h>
//#include <printf.h>
#include <RF24Ethernet.h>
#if !defined __arm__ && !defined __ARDUINO_X86__
  #include <EEPROM.h>
#endif

#include <Servo.h>

/*** Configure the radio CE & CS pins ***/
RF24 radio(7,8);
RF24Network network(radio);
RF24Mesh mesh(radio,network);
RF24EthernetClass RF24Ethernet(radio,network);

boolean Z1detected = false;
boolean Z2detected = false;



int pirPin =6;

Servo arm;
Servo base;

EthernetClient client;

void setup() {
  
  Serial.begin(115200);
 // printf_begin();
  Serial.println("Start");
  
  pinMode(pirPin, INPUT);
  // This step is very important. The address of the node needs to be set both
  // on the radio and in the UIP layer
  // This is the RF24Network address and needs to be configured accordingly if
  // using more than 4 nodes with the master node. Otherwise, 01-04 can be used.
  mesh.setNodeID(3);
  mesh.begin();
  Serial.println(mesh.mesh_address,OCT);
  
  // Set the IP address we'll be using. The last octet mast match the nodeID (9)
  IPAddress myIP(192,168,2,3);
  Ethernet.begin(myIP);
  
  // If you'll be making outgoing connections from the Arduino to the rest of
  // the world, you'll need a gateway set up.
  IPAddress gwIP(192,168,2,2);
  Ethernet.set_gateway(gwIP);  
  
  // Initialise servos
  arm.attach(4);
  base.attach(5);
}

uint32_t counter = 0;
uint32_t reqTimer = 0;

uint32_t mesh_timer = 0;

void loop() {

  // Optional: If the node needs to move around physically, or using failover nodes etc.,
  // enable address renewal
  if(millis()-mesh_timer > 30000){ //Every 30 seconds, test mesh connectivity
    mesh_timer = millis();
    if( ! mesh.checkConnection() ){
        //refresh the network address        
        mesh.renewAddress();
     }
  }

size_t size;

if(size = client.available() > 0){
    char c = client.read();
    Serial.print(c);
    // Sends a line-break every 150 characters, comment out if not connecting to google
    //if(counter > 150){ Serial.println(""); counter=0;}
    counter++;
}

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println(F("Disconnect. Waiting for disconnect timeout"));
    client.stop();
    
    // Need to add servo code for zone 1 and 2
    
    // ALGO: 
    // GOTO Z1
    // DONE - Read PIR with .5 sec delay for 10 times
    // DONE - set Z1detect to true if PIR high >= 5 times
    // DONE - set Z1detect to false if PIR high < 5 times
    delay(1000); // For PIR to be stable
    Z1detected = getPIR(10);
    // DONE - Send data
    
    // GOTO Z2
    // DONE - read PIR with .5 sec delay for 10 times
    // DONE - set Z2detect to true if PIR high >= 5 times
    // DONE - set Z2detect to false if PIR high < 5 times
    delay(1000); 
    Z2detected = getPIR(10);
    // DONE - Send data
    
   
    // Wait 5 seconds between requests, extra delay will be above
    reqTimer = millis();
    while(millis() - reqTimer < 5000 && !client.available() ){ }    
    connect();
  
  }
  // We can do other things in the loop, but be aware that the loop will
  // briefly pause while IP data is being processed.
}

void connect(){
    Serial.println(F("connecting"));
    //IPAddress goog(74,125,224,87);
    //IPAddress pizza(94,199,58,243);
    IPAddress rpi(192,168,2,2);
    if (client.connect(rpi, 8080)) {
      Serial.println(F("connected"));
      
      // Make an HTTP request:
      if(Z1detected && !Z2detected)
        client.write("GET /status/1/0 HTTP/1.1\n");
      else if (Z1detected && Z2detected)
        client.write("GET /status/1/1 HTTP/1.1\n");
      else if (!Z1detected && Z2detected)
        client.write("GET /status/0/1 HTTP/1.1\n");
      else if (!Z1detected && !Z2detected)
        client.write("GET /status/0/0 HTTP/1.1\n");
      //client.write("GET / HTTP/1.1\n");
      
      client.write("Host: 192.168.2.2:8080\n");
      //client.write("Host: www.google.ca\n");
      
      client.write("Connection: close\n\n");   
    
    }else{
      // if you didn't get a connection to the server:
      Serial.println(F("connection failed"));
    }
}


boolean getPIR(int times){
  int highCount = 0;
  for(int i = 0; i < times; i++){
    if(digitalRead(pirPin)){
      highCount++;
      delay(500);
    }
  }
  if(highCount >= 5){
    return true;
  } else {
    return false;
  }
}
