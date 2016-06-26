/*
 * Student: Peter Chu
 * Advisor: Dr Bridget Benson
 * 
 * Senior Project: musicLED
 * Description:
 *  This project visualizes a strip of 60 RGB LEDs (APA102C)
 *  to music information delivered by the accompanying 
 *  Processing app. It uses Blynk on iOS/Andorid to adjust
 *  the various modes and settings of the application.
 *
 *  This program analyzes sound from the default recording device of the machine.
 *  It uses the Fast Fourier Transform to see audio activity in certain frequency bands.
 */

/**
  * An FFT object is used to convert an audio signal into its frequency domain representation. This representation
  * lets you see how much of each frequency is contained in an audio signal. Sometimes you might not want to 
  * work with the entire spectrum, so it's possible to have the FFT object calculate average frequency bands by 
  * simply averaging the values of adjacent frequency bands in the full spectrum. There are two different ways 
  * these can be calculated: Linearly, by grouping equal numbers of adjacent frequency bands, or 
  * Logarithmically, by grouping frequency bands by octave, which is more akin to how humans hear sound.
  * 
  * This sketch uses the logarithmic average because its application deals with music and its visualization. 
  *
  * Moving the mouse across the sketch will display the center frequency of that band.
  * For more information about Minim and additional features, visit http://code.compartmental.net/minim/
  */

import processing.net.*;
import processing.serial.*;
import ddf.minim.analysis.*;
import ddf.minim.*;
import papaya.*;

// minim
Minim minim;  
AudioInput in;
FFT fftLin;
FFT fftLog;

// constants defines
final int avgSens = 75;         // number of averages for calculating running average
final int maxSens = 300;        // number of averages for calculating running maximums
final float spectrumScale = 2;  // scalar to make bars more visible
final int timeSens = 250;       // ms to wait until next beat is valid
final float avgMult = 1.8;      // scales the threshold
final float avgOffset = 4;      // higher number prevents noisy detections
final int bufferSize = 1024;    // for the FFT analysis

// FUTURE WORK
final int spectrumAvgSens = 5;   // number of averages for calculating running spectrum avg
final int spectrumMaxSens = 400; // number of averages for calculating running spectrum max
// FUTURE WORK

// beat finding variables
float runAvgs[][] = new float[60][avgSens];  // array holds ith bin time average value for calculating threshold
float runMaxs[][] = new float[60][maxSens];  // array holds ith bin time average value for calculating max
int timer[] = new int[60];      // holds the last time a beat was detected for the ith freq band
int counter;                    // used for knowing how many iterations draw has run
byte beats[] = new byte[60];    // array holds beats (info)

// draw variables
float height3;
float height23;
PFont font;

// FUTURE WORK
float spectrumSum = 0;
float spectrumAvg = 0;
float spectrumAvgMaxRatio = 0;
float runSpectrumAvgs[] = new float[spectrumAvgSens];
float runSpectrumMaxs[] = new float[spectrumMaxSens];
// FUTURE WORK


// network variables
Server myServer;

void setup()
{
  // canvas
  size(512, 512);
  height3 = height/3;
  height23 = 2*height/3;
  
  // start myServer on port 5204 
  myServer = new Server(this, 5204);

  // minim
  minim = new Minim(this);
  in = minim.getLineIn(Minim.STEREO, bufferSize);
  
  // create an FFT object for calculating logarithmically spaced averages
  // note that bufferSize needs to be a power of two
  fftLog = new FFT(in.bufferSize(), in.sampleRate());
  
  // calculate averages based on a miminum octave width of 22 Hz
  // split each octave into 6 bands
  // this should result in 60 averages
  fftLog.logAverages(22, 6);
  
  rectMode(CORNERS);
  font = loadFont("ArialMT-12.vlw");
}

void draw()
{
  background(0);
  
  textFont(font);
  textSize( 18 );
 
  float centerFrequency = 0;
  
  // perform a forward FFT on the AudioInput in
  fftLog.forward(in.mix);
  
  // draw the logarithmic averages
  int w = int( width/fftLog.avgSize() );  // indicates where to draw rectangles

  for(int i = 0; i < fftLog.avgSize(); i++)
  {
    centerFrequency    = fftLog.getAverageCenterFrequency(i);
    // how wide is this average in Hz?
    float averageWidth = fftLog.getAverageBandWidth(i);   
    
    // we calculate the lowest and highest frequencies
    // contained in this average using the center frequency
    // and bandwidth of this average.
    float lowFreq  = centerFrequency - averageWidth/2;
    float highFreq = centerFrequency + averageWidth/2;
    
    // store values into array of -Sens size
    // altering -Sens values alters the sensitivity of the running average & max 
    // by controlling the number of values used for calculating the running average & max
    runAvgs[i][counter % avgSens] = (int)fftLog.getAvg(i)*spectrumScale;
    runMaxs[i][counter % maxSens] = (int)fftLog.getAvg(i)*spectrumScale;
    
    // draw the maximum rect
    // maximum = mean + 3 standard deviations
    fill(0, 255, 0);
    rect(i*w, height, i*w + w, height - (Descriptive.mean(runMaxs[i]) + 3 * Descriptive.std(runMaxs[i], true)));
    
    // draw the threshold rect
    // instantaneous value in ith bin must exceed the threshold value to count as a beat
    // treshold = mean + 1 standard deviation
    fill(0, 0, 255);
    rect(i*w, height, i*w + w, height - (Descriptive.mean(runAvgs[i]) + 1 * Descriptive.std(runAvgs[i], true)));
  
    // if ((inst pwr > threshold) AND (sensitivity timer is up)), send beats to client
    //if (fftLog.getAvg(i)*spectrumScale > avgs[i] +  1 * Descriptive.std(runAvgs[i], true) + avgOffset) {
    if (fftLog.getAvg(i)*spectrumScale > Descriptive.mean(runAvgs[i]) +  1 * Descriptive.std(runAvgs[i], true) + avgOffset) {
      if (millis() > timer[i] + timeSens) {
        timer[i] = millis();      // reset timer
        beats[i] = (byte)i;       // mark beat in ith frequency band
        myServer.write(beats[i]);  
      }
    }
    
    // if the mouse is inside of this average's rectangle
    if ( mouseX >= i * w && mouseX < i*w + w)
    {
      fill(255, 255, 0);
      text("Log Avg Center Frequency: " + centerFrequency + " bin " + i, 5, 0.1 * height);
    }
    
    // draw a rectangle for each average, multiply the value by spectrumScale so we can see it better
    fill(255);
    rect(i*w, height, i*w + w, height - fftLog.getAvg(i)*spectrumScale);
    
    
    
    
    
    
    
    // FUTURE WORK -- calculate instant spectrum sum and avg
    spectrumSum += fftLog.getAvg(i) * spectrumScale;
    //println("bin " + i + " lowFreq = "+ lowFreq + " hiFreq = " + highFreq);
    // FUTURE WORK
  }
 
 
  
  
  // FUTURE WORK -- spectrum-wide analysis
  // proportion to 255 for LED brightness
  spectrumAvg = spectrumSum / 50;
  runSpectrumAvgs[counter % spectrumAvgSens] = spectrumAvg;
  runSpectrumMaxs[counter % spectrumMaxSens] = spectrumAvg;
  spectrumAvgMaxRatio = Descriptive.mean(runSpectrumAvgs) / (Descriptive.mean(runSpectrumMaxs) + 3 * Descriptive.std(runSpectrumMaxs, true)) * 255;
  // draw spectrum max
  fill(255, 0, 0);
  rect(59*w, height, 60*w, height - (Descriptive.mean(runSpectrumMaxs) + 3 * Descriptive.std(runSpectrumMaxs, true)));
  
  // draw spectrum avg
  fill(255, 255, 0);
  rect(59*w, height, 60*w, height - Descriptive.mean(runSpectrumAvgs));
  if (spectrumAvgMaxRatio > 255) {
    //println("spectrum avg/max ratio = 255");
    //myServer.write(255);
  }
  else {
    //println("spectrum avg/max ratio = " + spectrumAvgMaxRatio);
    //myServer.write((byte)spectrumAvgMaxRatio);
  }
  spectrumSum = 0;
  // FUTURE WORK
  
  
  
  
  
  counter++;  // marks another iteration through draw
  delay(15);  // delay to let ESP run
}

void serverEvent(Server someServer, Client someClient) {
  println("We have a new client: " + someClient.ip());
}