# Software
This is a general repository for our software/sensors. If it is helpful feel free to make a new repo. 

Base Sketch for Mines Wildfires & Water Quality Project Board

The board used is a EnviroDIY Mayfly board that is configured with the Arduino IDE.
This code aims to integrate multiple water quality sensors together to provide the user with important data points about their water site.

Currently, the sketch is programmed to measure and log the Data Number, Temperature in degrees Celcius, Voltage for Turbidity, and pH value at a 15 second sample rate with a startup delay of 30 seconds to an inserted microSD drive. Sections are outlined in the comments of the code of where to include more sensors information and functions. 
