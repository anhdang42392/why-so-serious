/*
 * NMEA.c
 *
 *  Created on: 25-Feb-2022
 *      Author: controllerstech.com
 */


#include "NMEAbackup.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"


int GMT = +7;



int inx = 0;
int hr=0,min=0,day=0,mon=0,yr=0;
int posFix = 0;
int daychange = 0;

/* Decodes the GGA Data
   @GGAbuffer is the buffer which stores the GGA Data
   @GGASTRUCT is the pointer to the GGA Structure (in the GPS Structure)
   @Returns 0 on success
   @ returns 1, 2 depending on where the return statement is excuted, check function for more details
*/

int decodeGGA (char *GGAbuffer, GGASTRUCT *gga)
{
	inx = 0;
	char buffer[12];
	int i = 0;
	while (GGAbuffer[inx] != ',') inx++;  // 1st ','
	inx++;
	while (GGAbuffer[inx] != ',') inx++;  // After time ','
	inx++;
	while (GGAbuffer[inx] != ',') inx++;  // after latitude ','
	inx++;
	while (GGAbuffer[inx] != ',') inx++;  // after NS ','
	inx++;
	while (GGAbuffer[inx] != ',') inx++;  // after longitude ','
	inx++;
	while (GGAbuffer[inx] != ',') inx++;  // after EW ','
	inx++;  // reached the character to identify the fix
	while (GGAbuffer[inx] != ',') inx++; // after position ','
	inx++;
	while(GGAbuffer[inx] != ',') inx++;   // after  number of gga->numofggagps->numofsat ','
	inx++;
	if ((GGAbuffer[inx] == '1') || (GGAbuffer[inx] == '2') || (GGAbuffer[inx] == '6') ||  (GGAbuffer[inx] == '8'))   // 0 indicates no fix yet
	{
		gga->isfixValid = 1;   // fix available
		inx = 0;   // reset the index. We will start from the inx=0 and extract information now
	}
	else
	{
		gga->isfixValid = 0;   // If the fix is not available
		return 1;  // return error
	}
	while (GGAbuffer[inx] != ',') inx++;  // 1st ','


/*********************** Get TIME ***************************/
//(Update the GMT Offset at the top of this file)

	inx++;   // reach the first number in time
	memset(buffer, '\0', 12);
	i=0;
	while (GGAbuffer[inx] != ',')  // copy upto the we reach the after time ','
	{
		buffer[i] = GGAbuffer[inx];
		i++;
		inx++;
	}

	hr = (atoi(buffer)/10000) + GMT/100;   // get the hours from the 6 digit number

	min = ((atoi(buffer)/100)%100) + GMT%100;  // get the minutes from the 6 digit number

	// adjust time.. This part still needs to be tested
	if (min > 59)
	{
		min = min-60;
		hr++;
	}
	if (hr<0)
	{
		hr=24+hr;
		daychange--;
	}
	if (hr>=24)
	{
		hr=hr-24;
		daychange++;
	}

	// Store the time in the GGA structure
	gga->tim.hour = hr;
	gga->tim.min = min;
	gga->tim.sec = atoi(buffer)%100;

/***************** Get LATITUDE  **********************/
	inx++;   // Reach the first number in the lattitude
	memset(buffer, '\0', 12);
	i=0;
	while (GGAbuffer[inx] != ',')   // copy upto the we reach the after lattitude ','
	{
		buffer[i] = GGAbuffer[inx];
		i++;
		inx++;
	}
	if (strlen(buffer) < 6) return 2;  // If the buffer length is not appropriate, return error
	int16_t num = (atoi(buffer));   // change the buffer to the number. It will only convert upto decimal
	int j = 0;
	while (buffer[j] != '.') j++;   // Figure out how many digits before the decimal
	j++;
	int declen = (strlen(buffer))-j;  // calculate the number of digit after decimal
	int dec = atoi ((char *) buffer+j);  // conver the decimal part a a separate number
	float lat = (num/100.0) + (dec/pow(10, (declen+2)));  // 1234.56789 = 12.3456789
	gga->lcation.latitude = lat/60.0;  // save the lattitude data into the strucure
	inx++;
	gga->lcation.NS = GGAbuffer[inx];  // save the N/S into the structure


/***********************  GET LONGITUDE **********************/
	inx++;  // ',' after NS character
	inx++;  // Reach the first number in the longitude
	memset(buffer, '\0', 12);
	i=0;
	while (GGAbuffer[inx] != ',')  // copy upto the we reach the after longitude ','
	{
		buffer[i] = GGAbuffer[inx];
		i++;
		inx++;
	}
	num = (atoi(buffer));  // change the buffer to the number. It will only convert upto decimal
	j = 0;
	while (buffer[j] != '.') j++;  // Figure out how many digits before the decimal
	j++;
	declen = (strlen(buffer))-j;  // calculate the number of digit after decimal
	dec = atoi ((char *) buffer+j);  // conver the decimal part a a separate number
	lat = (num/100.0) + (dec/pow(10, (declen+2)));  // 1234.56789 = 12.3456789
	gga->lcation.longitude = lat/60.0;  // save the longitude data into the strucure
	inx++;
	gga->lcation.EW = GGAbuffer[inx];  // save the E/W into the structure

/**************************************************/
	// skip positition fix
		inx++;   // ',' after E/W
		memset(buffer, '\0', 12);
		i=0;
		while (GGAbuffer[inx] != ',')  // copy upto the we reach the after time ','
		{
			buffer[i] = GGAbuffer[inx];
			i++;
			inx++;
		}

        posFix = 1;
        gga->fixIndicator = posFix;




   // ',' after position fix;

	// number of gga->numofggagps->numofsattelites
	inx++;  // Reach the first number in the gga->numofggagps->numofsatellites
	memset(buffer, '\0', 12);
	i=0;
	while (GGAbuffer[inx] != ',')  // copy upto the ',' after number of gga->numofggagps->numofsatellites
	{
		buffer[i] = GGAbuffer[inx];
		i++;
		inx++;
	}
	gga->numofsat = atoi(buffer);   // convert the buffer to number and save into the structure



		inx++;
		memset(buffer, '\0', 12);
		i=0;
		while (GGAbuffer[inx] != ',')
		{
			buffer[i] = GGAbuffer[inx];
			i++;
			inx++;
		}
		gga->HDOP = (atof(buffer));   // convert the buffer to number and save into the structure

	/*************** Altitude calculation ********************/
	inx++;
	memset(buffer, '\0', 12);
	i=0;
	while (GGAbuffer[inx] != ',')
	{
		buffer[i] = GGAbuffer[inx];
		i++;
		inx++;
	}
	num = (atoi(buffer));
	j = 0;
	while (buffer[j] != '.') j++;
	j++;
	declen = (strlen(buffer))-j;
	dec = atoi ((char *) buffer+j);
	lat = (num) + (dec/pow(10, (declen)));
	gga->alt.altitude = lat;

	inx++;
	gga->alt.unit = GGAbuffer[inx];

	return 0;

}

int accurateSignal(GGASTRUCT *ggagps){

	if(ggagps->fixIndicator == 0)
	{
		return 0;
	}
	if(ggagps->fixIndicator != 0)
	{if((ggagps->numofsat>=3)&&(ggagps->numofsat<5))
    {
        if(ggagps->HDOP > 7.0) return 1;
        else if((ggagps->HDOP <= 7.0)&&(ggagps->HDOP >= 6.0)) return 1;
        else if((ggagps->HDOP < 6.0)&&(ggagps->HDOP >= 5.0)) return 1;
        else if((ggagps->HDOP < 5.0)&&(ggagps->HDOP >= 4.0)) return 1;
        else if((ggagps->HDOP < 4.0)&&(ggagps->HDOP >= 3.0)) return 1;
        else if((ggagps->HDOP < 3.0)&&(ggagps->HDOP >= 2.0)) return 1;
        else if((ggagps->HDOP < 2.0)&&(ggagps->HDOP >= 1.0)) return 2;
        else if(ggagps->HDOP < 1.0) return 2;
    }
    else if((ggagps->numofsat>=5)&&(ggagps->numofsat<7))
    {
        if(ggagps->HDOP > 7.0) return 2;
        else if((ggagps->HDOP <= 7.0)&&(ggagps->HDOP >= 6.0)) return 2;
        else if((ggagps->HDOP < 6.0)&&(ggagps->HDOP >= 5.0)) return 2;
        else if((ggagps->HDOP < 5.0)&&(ggagps->HDOP >= 4.0)) return 2;
        else if((ggagps->HDOP < 4.0)&&(ggagps->HDOP >= 3.0)) return 2;
        else if((ggagps->HDOP < 3.0)&&(ggagps->HDOP >= 2.0)) return 2;
        else if((ggagps->HDOP < 2.0)&&(ggagps->HDOP >= 1.0)) return 3;
        else if(ggagps->HDOP < 1.0) return 3;
    }

    else if((ggagps->numofsat>=7)&&(ggagps->numofsat<9))
    {
        if(ggagps->HDOP > 7.0) return 3;
        else if((ggagps->HDOP <= 7.0)&&(ggagps->HDOP >= 6.0)) return 3;
        else if((ggagps->HDOP < 6.0)&&(ggagps->HDOP >= 5.0)) return 3;
        else if((ggagps->HDOP < 5.0)&&(ggagps->HDOP >= 4.0)) return 3;
        else if((ggagps->HDOP < 4.0)&&(ggagps->HDOP >= 3.0)) return 3;
        else if((ggagps->HDOP < 3.0)&&(ggagps->HDOP >= 2.0)) return 3;
        else if((ggagps->HDOP < 2.0)&&(ggagps->HDOP >= 1.0)) return 4;
        else if(ggagps->HDOP < 1.0) return 4;
    }

    else if((ggagps->numofsat>=9)&&(ggagps->numofsat<11))
    {
        if(ggagps->HDOP > 7.0) return 4;
        else if((ggagps->HDOP <= 7.0)&&(ggagps->HDOP >= 6.0)) return 4;
        else if((ggagps->HDOP < 6.0)&&(ggagps->HDOP >= 5.0)) return 4;
        else if((ggagps->HDOP < 5.0)&&(ggagps->HDOP >= 4.0)) return 4;
        else if((ggagps->HDOP < 4.0)&&(ggagps->HDOP >= 3.0)) return 4;
        else if((ggagps->HDOP < 3.0)&&(ggagps->HDOP >= 2.0)) return 4;
        else if((ggagps->HDOP < 2.0)&&(ggagps->HDOP >= 1.0)) return 5;
        else if(ggagps->HDOP < 1.0) return 5;
    }

    else if((ggagps->numofsat>=11)&&(ggagps->numofsat<13))
    {
        if(ggagps->HDOP > 7.0) return 2;
        else if((ggagps->HDOP <= 7.0)&&(ggagps->HDOP >= 6.0)) return 5;
        else if((ggagps->HDOP < 6.0)&&(ggagps->HDOP >= 5.0)) return 5;
        else if((ggagps->HDOP < 5.0)&&(ggagps->HDOP >= 4.0)) return 5;
        else if((ggagps->HDOP < 4.0)&&(ggagps->HDOP >= 3.0)) return 5;
        else if((ggagps->HDOP < 3.0)&&(ggagps->HDOP >= 2.0)) return 5;
        else if((ggagps->HDOP < 2.0)&&(ggagps->HDOP >= 1.0)) return 6;
        else if(ggagps->HDOP < 1.0) return 6;
    }

    else if(ggagps->numofsat>= 13)
    {
        if(ggagps->HDOP > 7.0) return 6;
        else if((ggagps->HDOP <= 7.0)&&(ggagps->HDOP >= 6.0)) return 6;
        else if((ggagps->HDOP < 6.0)&&(ggagps->HDOP >= 5.0)) return 6;
        else if((ggagps->HDOP < 5.0)&&(ggagps->HDOP >= 4.0)) return 6;
        else if((ggagps->HDOP < 4.0)&&(ggagps->HDOP >= 3.0)) return 7;
        else if((ggagps->HDOP < 3.0)&&(ggagps->HDOP >= 2.0)) return 7;
        else if((ggagps->HDOP < 2.0)&&(ggagps->HDOP >= 1.0)) return 7;
        else if(ggagps->HDOP < 1.0) return 8;
    }
	}
}











