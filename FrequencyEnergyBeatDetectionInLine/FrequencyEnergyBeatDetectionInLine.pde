import processing.net.*;
import ddf.minim.*;
import ddf.minim.analysis.*;
import processing.serial.*;

// minim variables
Minim minim;
AudioInput in;
BeatDetect beat;
BeatListener bl;
int bufferSize = 1024;
int sampleRate = 44100;

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
  beat.setSensitivity(100);  // sets the sensitivity to 200 ms
  
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
  
  // seperate into 3 blocks
  int loloBand = 0;
  int lohiBand = 10;
  int midloBand = 11;
  int midhiBand = 20;
  int hiloBand = 21;
  int hihiBand = 26;
  // at least this many bands must have an onset 
  // for isRange to return true
  int numberOfOnsetsThreshold = 3;
  if ( beat.isRange(loloBand, lohiBand, numberOfOnsetsThreshold) )
  {
    fill(0,255,0,200);
    rect(rectW*loloBand, 0, (lohiBand-loloBand)*rectW, height);
    
    myServer.write('1');
    println("1");
  }
  else if (beat.isRange(midloBand, midhiBand, numberOfOnsetsThreshold))
  { 
    fill(255,0,0,200);
    rect(rectW*midloBand, 0, (midhiBand-midloBand)*rectW, height);
    
    myServer.write('2');
    println("2");
  }
  else if (beat.isRange(hiloBand, hihiBand, 3))
  {
    fill(0,0,255,200);
    rect(rectW*hiloBand, 0, (hihiBand-hiloBand)*rectW, height);
    
    myServer.write('3');
    println("3");
  }
  else
  {
    myServer.write('0'); 
    println('0');
  }
  
  fill(255);
  
  delay(10);
}

void serverEvent(Server someServer, Client someClient) {
  println("We have a new client: " + someClient.ip());
}