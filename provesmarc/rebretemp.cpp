/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licensesd
 version 2 as published by the Free Software Foundation.

 03/17/2013 : Charles-Henri Hallard (http://hallard.me)
              Modified to use with Arduipi board http://hallard.me/arduipi
						  Changed to use modified bcm2835 and RF24 library
TMRh20 2014 - Updated to work with optimized RF24 Arduino library

 */

/**
 * Example RF Radio Ping Pair
 *
 * This is an example of how to use the RF24 class on RPi, communicating to an Arduino running
 * the GettingStarted sketch.
 */

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <RF24/RF24.h>
#include <fstream>

// Per a la data i hora
#include <stdio.h>
#include <time.h>

// Per a fer el "buscar y reemplazar" ("_" per "\t")
#include <algorithm> 

using namespace std;
//
// Hardware configuration
// Configure the appropriate pins for your connections

/****************** Raspberry Pi ***********************/

// Radio CE Pin, CSN Pin, SPI Speed
// See http://www.airspayce.com/mikem/bcm2835/group__constants.html#ga63c029bd6500167152db4e57736d0939 and the related enumerations for pin information.

// Setup for GPIO 22 CE and CE0 CSN with SPI Speed @ 4Mhz
//RF24 radio(RPI_V2_GPIO_P1_22, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_4MHZ);

// NEW: Setup for RPi B+
//RF24 radio(RPI_BPLUS_GPIO_J8_15,RPI_BPLUS_GPIO_J8_24, BCM2835_SPI_SPEED_8MHZ);

// Setup for GPIO 15 CE and CE0 CSN with SPI Speed @ 8Mhz
//RF24 radio(RPI_V2_GPIO_P1_15, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ);

// RPi generic:
RF24 radio(22,0);

/*** RPi Alternate ***/
//Note: Specify SPI BUS 0 or 1 instead of CS pin number.
// See http://tmrh20.github.io/RF24/RPi.html for more information on usage

//RPi Alternate, with MRAA
//RF24 radio(15,0);

//RPi Alternate, with SPIDEV - Note: Edit RF24/arch/BBB/spi.cpp and  set 'this->device = "/dev/spidev0.0";;' or as listed in /dev
//RF24 radio(22,0);


/****************** Linux (BBB,x86,etc) ***********************/

// See http://tmrh20.github.io/RF24/pages.html for more information on usage
// See http://iotdk.intel.com/docs/master/mraa/ for more information on MRAA
// See https://www.kernel.org/doc/Documentation/spi/spidev for more information on SPIDEV

// Setup for ARM(Linux) devices like BBB using spidev (default is "/dev/spidev1.0" )
//RF24 radio(115,0);

//BBB Alternate, with mraa
// CE pin = (Header P9, Pin 13) = 59 = 13 + 46 
//Note: Specify SPI BUS 0 or 1 instead of CS pin number. 
//RF24 radio(59,0);

/********** User Config *********/
// Assign a unique identifier for this node, 0 or 1
bool radioNumber = 1;

/********************************/

// Radio pipe addresses for the 2 nodes to communicate.
const uint8_t pipes[][6] = {"1Node","2Node"};


int main(int argc, char** argv){

int afegeix (string node, string dataihora, string dades);
std::string liniatext;
char info_rebuda[50]=""; //la informació a rebre sera una cadena de 50 bytes. Modificar al gust.

bool read( void* buf, uint8_t len );
bool write( const void* buf, uint8_t len );

  // Setup and configure rf radio
  radio.begin();
  printf("\n ************ Role Setup ***********\n");
  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  // Dump the configuration of the rf unit for debugging
  radio.printDetails();


/***********************************/
  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.

    if ( !radioNumber )    {
      radio.openWritingPipe(pipes[0]);
      radio.openReadingPipe(1,pipes[1]);
    } else {
      radio.openWritingPipe(pipes[1]);
      radio.openReadingPipe(1,pipes[0]);
    }
	
	radio.startListening();

	printf("\n ************ MODE RECEPCIÓ DE DADES ***********\n");
	// forever loop
	while (1)
	{

		//
		// Pong back role.  Receive each packet, dump it out, and send it back
		//

			
		// if there is data ready
		if ( radio.available() )
		{
			// Dump the payloads until we've gotten everything

			// Fetch the payload, and see if this was the last one.
			while(radio.available()){
				radio.read( &info_rebuda, sizeof(info_rebuda) );
			}
			/*radio.stopListening();
			
			radio.write( &info_rebuda, sizeof(info_rebuda) ); // crec que no caldria

			// Now, resume listening so we catch the next packets.
			radio.startListening();
			*/
			
			// Spew it
			printf("Número rebut (de mida %d): %s\n",sizeof(info_rebuda), info_rebuda);

			// Convertim unsigned long a string
			std::ostringstream ss;
			ss << info_rebuda;
                        std::string info_rebuda_str = ss.str();

			// Data i hora
			time_t rawtime;
			struct tm * timeinfo;
			char dataihora [80];
			      
			time (&rawtime);
			timeinfo = localtime (&rawtime);
			        
			strftime (dataihora,80,"%Y/%m/%d %H:%M:%S",timeinfo); // la variable dataihora es on esta la data i hora, obviament

			// unim les dades rebudes
			std::stringstream liniatext;
			liniatext << info_rebuda_str; //aquí ho podriem concatenar tot, si volguessim

			std::string liniatext_str = liniatext.str(); // ho convertim a string

			// Separem les dades rebudes amb tabuladors
  			std::replace( liniatext_str.begin(), liniatext_str.end(), '_', '\t'); // replace all '_' to 'TAB'
			
			// Mirem quin es el node (els tres primers caracters)
			std::string node = liniatext_str.substr (0,3);
			std::cout << "Dades rebudes del node: " << node << "\n";
			
			// Separem la resta
			liniatext_str = liniatext_str.substr (4); 

			// Ho escrivim a un fitxer
			afegeix ( node, dataihora, liniatext_str );
			
			
			delay(925); //Delay after payload responded to, minimize RPi CPU time
			
		}
		
        	usleep(400); //per a no saturar la CPU. Com més alt, menys consum, pero mes possibilitats que es perdi un paquet d'informacio.

	} // forever loop

  return 0;
}

// Afegim string a fitxer de text
int afegeix (std::string node, std::string dataihora, std::string dades)
{

	string fitxerdades;

	// En funció del node, ho posem en un fitxer diferent
	/*
	switch(str2int(node)) {
                case str2int("001") : fitxerdades = "temp.txt";
                case str2int("002") : fitxerdades = "dades.txt";
                case str2int("003") : fitxerdades = "energia.txt";
                default : fitxerdades = "dades.txt";
	}
	*/
	
	if ( node == "001" ) { fitxerdades = "/home/pi/gasoil.txt"; }
	if ( node == "002" ) { fitxerdades = "/home/pi/temp.txt"; }
	if ( node == "003" ) { fitxerdades = "/home/pi/energia.txt"; }
	if ( node == "005" ) { fitxerdades = "/home/pi/alarmafum.txt"; }
	if ( node == "006" ) { fitxerdades = "/home/pi/aquari.txt"; }
	
	
	// std::cout << "Ho desem al fitxer: " << fitxerdades << "\n";

	char *fitxerdades_char = &fitxerdades[0u]; // Ho convertim a char, sino al myfile.open no li agrada.

	ofstream myfile;
  	myfile.open (fitxerdades_char, std::ios_base::app);
  	myfile << dataihora << "\t" << dades << "\n" << std::flush;
  	myfile.close();
  	
  	if (node == "002") {
  	        system(("/home/pi/parsebarometre.sh "+dades).c_str());
        }
  	if (node == "003") {
  	        system(("/home/pi/parseimosquitto.sh "+dades).c_str());
        }
  	                                
  	if (node == "005") {
  	        system(("/home/pi/parsealarmafum.sh "+dades).c_str());
  	        //system(("ssh pi@192.168.1.8 'echo "+dataihora+" Alarma: "+dades+" | sendxmpp -t -u iwannaberich -o gmail.com marcpalaus@gmail.com -f /home/pi/.sendxmpprc'").c_str());
  	}
  	if (node == "006") {
  	        system(("/home/pi/parseaquari.sh "+dades).c_str());
        }
  	return 0;
}

