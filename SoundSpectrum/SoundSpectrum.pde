/**
  * An FFT object is used to convert an audio signal into its frequency domain representation. This representation
  * lets you see how much of each frequency is contained in an audio signal. Sometimes you might not want to 
  * work with the entire spectrum, so it's possible to have the FFT object calculate average frequency bands by 
  * simply averaging the values of adjacent frequency bands in the full spectrum. There are two different ways 
  * these can be calculated: <b>Linearly</b>, by grouping equal numbers of adjacent frequency bands, or 
  * <b>Logarithmically</b>, by grouping frequency bands by <i>octave</i>, which is more akin to how humans hear sound.
  * <br/>
  * This sketch illustrates the difference between viewing the full spectrum, 
  * linearly spaced averaged bands, and logarithmically spaced averaged bands.
  * <p>
  * From top to bottom:
  * <ul>
  *  <li>The full spectrum.</li>
  *  <li>The spectrum grouped into 30 linearly spaced averages.</li>
  *  <li>The spectrum grouped logarithmically into 10 octaves, each split into 3 bands.</li>
  * </ul>
  *
  * Moving the mouse across the sketch will highlight a band in each spectrum and display what the center 
  * frequency of that band is. The averaged bands are drawn so that they line up with full spectrum bands they 
  * are averages of. In this way, you can clearly see how logarithmic averages differ from linear averages.
  * <p>
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
final int avgSens = 75;  // number of averages for calculating running average
final int maxSens = 400; // number of averages for calculating running maximums
final int spectrumSens = 400; // number of averages for calculating running spectrum max
final float spectrumScale = 2;  // scalar to make bars more visible
final int timeSens = 300;  // ms to wait until next beat is valid
final float avgMult = 1.8;  // scales the threshold
final float avgOffset = 4;   // higher number prevents noisy detections
final int bufferSize = 1024; 

// draw variables
float height3;
float height23;
PFont font;

// beat finding variables
float runAvgs[][] = new float[30][avgSens];
float runMaxs[][] = new float[30][maxSens];
float avgs[] = new float[30];
float maxs[] = new float[30];
float sums[] = new float[30];
float runSpectrumAvgs[] = new float[spectrumSens];
float runSPectrumMaxs[] = new float[spectrumSens];
int timer[] = new int[30];
int counter;
byte absPwr[] = new byte[30];
byte avgPwr[] = new byte[30];
byte beats[] = new byte[30];
float spectrumSum = 0;
float spectrumAvg = 0;
float spectrumAvgMaxRatio = 0;

// network variables
Server myServer;

void setup()
{
  // canvas
  size(512, 512);
  height3 = height/3;
  height23 = 2*height/3;
  
  // server
  myServer = new Server(this, 5204);

  // minim
  minim = new Minim(this);
  in = minim.getLineIn(Minim.STEREO, bufferSize);
  
  // create an FFT object that has a time-domain buffer the same size as the input's sample buffer
  // note that this needs to be a power of two 
  // and that it means the size of the spectrum will be
  fftLin = new FFT( in.bufferSize(), in.sampleRate() );
  println(in.bufferSize() + " " + in.sampleRate());
  
  // calculate the averages by grouping frequency bands linearly. use 30 averages.
  fftLin.linAverages( 30 );
  
  // create an FFT object for calculating logarithmically spaced averages
  fftLog = new FFT( in.bufferSize(), in.sampleRate() );
  
  // calculate averages based on a miminum octave width of 22 Hz
  // split each octave into three bands
  // this should result in 30 averages
  fftLog.logAverages( 22, 3 );
  
  rectMode(CORNERS);
  font = loadFont("ArialMT-12.vlw");
}

void draw()
{
  background(0);
  
  textFont(font);
  textSize( 18 );
 
  float centerFrequency = 0;
  
  // perform a forward FFT on the samples in jingle's mix buffer
  // note that if jingle were a MONO file, this would be the same as using jingle.left or jingle.right
  fftLin.forward( in.mix );
  fftLog.forward( in.mix );
 
  // draw the full spectrum
  {
    noFill();
    for(int i = 0; i < fftLin.specSize(); i++)
    {
      // if the mouse is over the spectrum value we're about to draw
      // set the stroke color to red
      if ( i == mouseX )
      {
        centerFrequency = fftLin.indexToFreq(i);
        stroke(255, 0, 0);
      }
      else
      {
          stroke(255);
      }
      //line(i, height3, i, height3 - fftLin.getBand(i)*spectrumScale);
    }
    
    fill(255, 128);
    //text("Spectrum Center Frequency: " + centerFrequency, 5, height3 - 25);
  }
  
  // no more outline, we'll be doing filled rectangles from now
  noStroke();
  
  // draw the linear averages
  {
    // since linear averages group equal numbers of adjacent frequency bands
    // we can simply precalculate how many pixel wide each average's 
    // rectangle should be.
    int w = int( width/fftLin.avgSize() );
    for(int i = 0; i < fftLin.avgSize(); i++)
    {
      // if the mouse is inside the bounds of this average,
      // print the center frequency and fill in the rectangle with red
      if ( mouseX >= i*w && mouseX < i*w + w )
      {
        centerFrequency = fftLin.getAverageCenterFrequency(i);
        
        fill(255, 128);
        //text("Linear Average Center Frequency: " + centerFrequency, 5, height23 - 25);
        
        fill(255, 0, 0);
      }
      else
      {
          fill(255);
      }
      // draw a rectangle for each average, multiply the value by spectrumScale so we can see it better
      //rect(i*w, height23, i*w + w, height23 - fftLin.getAvg(i)*spectrumScale);
      
      /*
      // running average power
      sums[i] = 0;
      
      //if ( i == 0 )
      //{
        runAvgs[i][counter % runAvgs[i].length] = (int)fftLin.getAvg(i)*spectrumScale;
        for (int j = 0; j < runAvgs[i].length; j++)
        {
          sums[i] += runAvgs[i][j];
        }
        avgs[i] = sums[i] / runAvgs[i].length;
        
        // draw the avg rect
        fill(0, 0, 255);
        rect(i*w, height23, i*w + w, height23 - avgs[i]);
      //}
      
      //println("bin=" + i + " inst=" + fftLin.getAvg(i)*spectrumScale + " avg=" + avgs[i]);
      */
    }
    //counter++;
  }
  
  // draw the logarithmic averages
  {
    int w = int( width/fftLog.avgSize() );
    // since logarithmically spaced averages are not equally spaced
    // we can't precompute the width for all averages
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
      
      // freqToIndex converts a frequency in Hz to a spectrum band index
      // that can be passed to getBand. in this case, we simply use the 
      // index as coordinates for the rectangle we draw to represent
      // the average.
      int xl = (int)fftLog.freqToIndex(lowFreq);
      int xr = (int)fftLog.freqToIndex(highFreq);
      
      // if the mouse is inside of this average's rectangle
      // print the center frequency and set the fill color to red
      //if ( mouseX >= xl && mouseX < xr )
      if ( mouseX >= i * w && mouseX < i*w + w)
      {
        fill(255, 128);
        text("Log Avg Center Frequency: " + centerFrequency + " bin " + i, 5, height - 25);
        fill(255, 0, 0);
      }
      
      // running average, running maximum 
      sums[i] = 0;
      maxs[i] = 0;
      
      // store values into array
      runAvgs[i][counter % avgSens] = (int)fftLog.getAvg(i)*spectrumScale;
      runMaxs[i][counter % maxSens] = (int)fftLog.getAvg(i)*spectrumScale;
      for (int j = 0; j < avgSens; j++)
      {
        sums[i] += runAvgs[i][j];
      }
      avgs[i] = sums[i] / avgSens;
      for (int j = 0; j < maxSens; j++)
      {
        sums[i] += runMaxs[i][j];
      }
      maxs[i] = sums[i] / maxSens;
      //println("bin " + i + " stddev: " + Descriptive.std(runAvgs[i], true));
      //println("bin " + i + " moving max: " + maxs[i] + 3 * Descriptive.std(runMaxs[i], true));
      
      // draw the maximum rect
      fill(0, 255, 0);
      rect(i*w, height, i*w + w, height - (maxs[i] + 3 * Descriptive.std(runMaxs[i], true)));
      
      // draw the threshold rect
      fill(0, 0, 255);
      //rect( xl, height, xr, height - avgs[i] * avgMult);
      //rect(i*w, height, i*w + w, height - avgs[i] * avgMult + avgOffset);
      rect(i*w, height, i*w + w, height - (avgs[i] + 1.5 * Descriptive.std(runAvgs[i], true)));
      
      //println("bin=" + i + " inst=" + fftLog.getAvg(i)*spectrumScale + " avg=" + avgs[i] * avgMult + 1);
      
      // if sensitivity timer is up AND inst pwr > threshold, draw beats
      //if (millis() > timer[i] + timeSens && fftLog.getAvg(i)*spectrumScale > avgs[i] * avgMult + avgOffset)
      if (fftLog.getAvg(i)*spectrumScale > avgs[i] +  1.5 * Descriptive.std(runAvgs[i], true) + avgOffset) {
        if (millis() > timer[i] + timeSens) {
          timer[i] = millis();
          fill(0, 255, 0);
          //rect( xl, height23, xr, height23 - fftLog.getAvg(i)*spectrumScale );
          //rect(i*w, height23, i*w + w, height23 - 30);
          beats[i] = (byte)i;
          myServer.write(beats[i]);
          //println("writing " + beats[i]);
        }
      }
      else
      {
        beats[i] = 31;
      }
      
      // calculate instant spectrum sum and avg
      spectrumSum += fftLog.getAvg(i) * spectrumScale;
      
      println("bin " + i + " lowFreq = "+ lowFreq + " hiFreq = " + highFreq);
      
      // draw a rectangle for each average, multiply the value by spectrumScale so we can see it better
      //rect( xl, height, xr, height - fftLog.getAvg(i)*spectrumScale );
      fill(255);
      rect(i*w, height, i*w + w, height - fftLog.getAvg(i)*spectrumScale);
    }
    counter++;
    
    spectrumAvg = spectrumSum / 30;
    runSpectrumAvgs[counter % spectrumSens] = spectrumAvg;
    spectrumAvgMaxRatio = spectrumAvg / (6 * Descriptive.std(runSpectrumAvgs, true)) * 255;
    // draw spectrum max
    fill(255, 0, 0);
    rect(29*w, height, 30*w, height - (6 * Descriptive.std(runSpectrumAvgs, true)));
    // draw spectrum avg
    fill(255, 255, 0);
    rect(29*w, height, 30*w, height - spectrumAvg);
    
    
    // proportion to 255 for LED brightness
    if (spectrumAvgMaxRatio > 255) {
      println("spectrum avg/max ratio = 255");
      //myServer.write(255);
    }
    else {
      println("spectrum avg/max ratio = " + spectrumAvgMaxRatio);
      //myServer.write((byte)spectrumAvgMaxRatio);
    }
    
    spectrumSum = 0;
    
    // delay to let ESP run
    delay(15);
  }
}

void serverEvent(Server someServer, Client someClient) {
  println("We have a new client: " + someClient.ip());
}