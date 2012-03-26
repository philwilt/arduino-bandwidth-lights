/**
  
  bandwidthMon.h
  
  This file is part of bandwidthMon
  
  Author: Phillip Wilt
  

  This is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

 **/

#ifndef BANDWIDTHMON_H_
#define BANDWIDTHMON_H_

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>

const char *NET_PATH ="/proc/net/dev"; //the path of the network file to read
const char *UPTIME_PATH = "/proc/uptime"; //path of the uptime file to read
const unsigned short NAME_LEN = 6; //default lenght of interface name
const unsigned int DEFAULT_DELAY = 500000; //our default delay in microseconds

//our data structure to hold a interface stats
typedef struct  {
	char* name;
	unsigned long rx_bytes;
	unsigned long tx_bytes;
}interface;

//updates the interface passed in
int update_interface(interface *inter);

//function to get the uptime from the system
//returns a -1 if we couldn't get uptime
float get_uptime();

//wait function
void wait ( int milliseconds );

#endif /* BANDWIDTHMON_H_ */
