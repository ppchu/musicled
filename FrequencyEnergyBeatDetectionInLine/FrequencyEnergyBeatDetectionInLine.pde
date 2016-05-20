import processing.net.*;
import ddf.minim.*;
import ddf.minim.analysis.*;
import processing.serial.*;

// minim variables
Minim minim;
AudioPlayer song;
AudioInput in;
BeatDetect beat;
BeatListener bl;
int bufferSize = 1024;
int sampleRate = 48000;

// network variables
Server myServer;
int val = 0;

class BeatListener implements AudioListener
{
  private BeatDetect beat;
  private AudioInput source;
  
  BeatListener(BeatDetect beat, AudioInput source)
  {
    this.source = source;
    this.source.addListener(this);
    this.beat = beat;
  }
  
  void samples(float[] samps)
  {
    beat.detect(source.mix);
  }
  
  void samples(float[] sampsL, float[] sampsR)
  {
    beat.detect(source.mix);
  }
}

void setup() {
  size(512, 200, P3D);
  
  // Starts a myServer on port 5204
  myServer = new Server(this, 5204); 
  
  // Minim
  minim = new Minim(this);
  
  // use the getLineIn method of the Minim object to get an AudioInput
  // The default values are for a stereo input with a 
  // 1024 sample buffer sample rate of 44100 and bit depth of 16
  in = minim.getLineIn(Minim.STEREO, bufferSize, sampleRate);
  
  // a beat detection object that is FREQ_ENERGY mode that 
  // expects buffers the length of song's buffer size
  // and samples captured at songs's sample rate
  beat = new BeatDetect(bufferSize, sampleRate);

  // After a beat has been detected, the algorithm will wait for 300 milliseconds 
  // before allowing another beat to be reported. You can use this to dampen the 
  // algorithm if it is giving too many false-positives. The default value is 10, 
  // which is essentially no damping. If you try to set the sensitivity to a negative value, 
  // an error will be reported and it will be set to 10 instead. 
  beat.setSensitivity(200);  // sets the sensitivity to 200 ms
  
  // make a new beat listener, so that we won't miss any buffers for the analysis
  bl = new BeatListener(beat, in); 
}

void draw() {
  background(0);
  
  // draw a rectangle for every detect band
  // that had on onset this frame
  float rectW = width / beat.detectSize();
  //println(beat.detectSize());
  for(int i = 0; i < beat.detectSize(); ++i)
  {
    // test one frequency band for an onset
    if ( beat.isOnset(i) )
    {
      rect( i*rectW, 0, rectW, height);
    }
  }
  
  // draw an orange rectangle over the bands in 
  // the range we are querying
  int lowBand = 3;
  int highBand = 10;
  // at least this many bands must have an onset 
  // for isRange to return true
  int numberOfOnsetsThreshold = 1;
  if ( beat.isRange(lowBand, highBand, numberOfOnsetsThreshold) )
  {
    fill(232,179,2,200);
    rect(rectW*lowBand, 0, (highBand-lowBand)*rectW, height);
    
    //myServer.write('1');
    //println("1");
  }
  else if (beat.isRange(21, 26, 1))
  { 
    fill(232,179,2,200);
    rect(rectW*21, 0, (26-21)*rectW, height);
    
    //myServer.write('2');
    //println("2");
  }
  else
  {
    //myServer.write('a');
    //println("a");
  }
  
  fill(255);
  
  val = (val + 1) % 255;
  myServer.write(val);
  println(val);
  delay(10);
}

void serverEvent(Server someServer, Client someClient) {
  println("We have a new client: " + someClient.ip());
}