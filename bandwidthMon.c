/**
  
  bandwidthMon.c 
  
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

#include <bandwidthMon.h>
#include <arduino-serial.c>

const char *PORT = "/dev/ttyUSB0";//the port of the controller
const unsigned short BAUD = 57600;//the baud rate on the controller
const unsigned short SCALE = 500; //the amount to scale the bandwidth by
char *INTER =  "eth0"; //our interface

//wait function
void wait ( int milliseconds )
{
	clock_t endwait;
	endwait = clock () + milliseconds * (CLOCKS_PER_SEC / 1000) ;
	while (clock() < endwait) {}
}

//updates the interface from the system
//returns -1 if failure, positive if success
int update_interface(interface *inter)
{
	FILE *fp;
	//open the file for reading
	if((fp = fopen(NET_PATH,"r"))== NULL)
	{
		perror("Lame!");
		printf("errno = %d.\n", errno);
		return -1;
	}

	char chrBuff[512]; //tmp buffer
	char tmp_inter[NAME_LEN]; //storage for our interface names

	//get and throw away first two lines
	fgets(chrBuff, sizeof(chrBuff), fp);
	fgets(chrBuff, sizeof(chrBuff), fp);

	//read until end of file
	while(fgets(chrBuff, sizeof(chrBuff), fp) != NULL)
	{
		//printf("%s\n", chrBuff);

		//grab data from the stream
		sscanf(chrBuff, "%s %*u %*u %*i %*i %*i %*i %*i %*i %*u %*i "
				"%*i %*i %*i %*i %*i %*i", tmp_inter);

		//removing the ':' from interface name
		tmp_inter[strlen(tmp_inter) - 1] = '\0';

		//we found our interface
		if(strcmp(tmp_inter,inter->name) == 0)
		{
			//read in stats
			sscanf(chrBuff, "%s %lu %*u %*i %*i %*i %*i %*i %*i %lu %*i "
					"%*i %*i %*i %*i %*i %*i",
					tmp_inter,&inter->rx_bytes, &inter->tx_bytes);

			//done
			fclose(fp);
			return;
		}

	}
	//in case we didn't find our interface
	//still need to close the file
	fclose(fp);
	return 1;


}

//function to get the uptime from the system
//returns a -1 if we couldn't get uptime
float get_uptime()
{
	FILE *fp; //our file ptr
	char chrBuff[20]; //buffer to hold our read

	float uptime;

	//open the file for reading
	if((fp = fopen(UPTIME_PATH,"r"))== NULL)
	{
		perror("Lame!"); //in case of failure
		printf("errno = %d.\n", errno);
		return -1;
	}

	while(fgets(chrBuff, sizeof(chrBuff), fp) != NULL)
	{
		sscanf(chrBuff,"%f %*d",&uptime); //read in our uptime
	}
	fclose(fp);
	return uptime;
}

int main() {

	//our interface to keep track of transfer stats
	interface myInter;
	myInter.name = INTER;
	myInter.rx_bytes = 0;
	myInter.tx_bytes = 0;

	//our serial port
	int fd;

	//to hold the old data
	unsigned long old_rx_bytes = 0;
	unsigned long old_tx_bytes = 0;

	float start = get_uptime(); //start time
	float finish = 0; //finish time

	//vars to hold bytes transferred since last read
	unsigned int bw_rx = 0;
	unsigned int bw_tx = 0;
	//char *test = "8\n";
	int kBps_rx = 0; //our transfer in KiloBytes per second
	int kBps_tx = 0;
	char writeString[4];

	float time_xfer;

	if((fd = serialport_init(PORT, BAUD)) == -1){
		perror("Couldn't open port.");
		printf("Coudn't open port. Error: %d\n", errno);
		return -1;
	}
	usleep(DEFAULT_DELAY);

	//Keep doing this
	while(1)
	{
		system("clear");

		//keep track of the old data
		old_rx_bytes = myInter.rx_bytes;
		old_tx_bytes = myInter.tx_bytes;

		//update our interface
		update_interface(&myInter);

		//done timing
		finish = get_uptime();
		time_xfer = ((float)(finish - start));

		//get our bytes transfered since last read
		bw_rx = (myInter.rx_bytes - old_rx_bytes);
		bw_tx = (myInter.tx_bytes - old_tx_bytes);
		//calc kilobytes per second
		kBps_rx = (int)(bw_rx/(time_xfer*1000));
		kBps_tx = (int)(bw_tx/(time_xfer*1000));

		//save as a string scaled
		sprintf(writeString, "%u\n",kBps_rx/SCALE);

		//write it to the arduino
		if(serialport_write(fd,  writeString) == -1)
		{
			perror("Couldn't write to port.");
			printf("Coudn't write to port. Error: %d\n", errno);
			return -1;
		}

		//output it to ourselves to watch the bytes fly
		printf("The stats for %s:\nRX: %6u KB/s\nTX: %6u KB/s\n", myInter.name,
				kBps_rx ,kBps_tx);
		printf("Delay: %4.3f seconds\n",time_xfer);

		//start the timer until we read stats again
		start = get_uptime();

		//delay
		usleep(DEFAULT_DELAY);

	}
	return 0;
}
