#include "optfft.h"
#define IMAGE_CAPTURE_SAMPLES 256
#define IMAGE_CAPTURE_PIN 5
#define PWM_OUTPUT_PIN 23

signed int* samples;
signed int* zeros;

#define DEPLOY_RETRACT_TIME SECOND*10
#define DEFAULT_DUTY_CYCLE 0.1f
#define SECOND 1000000.0f
#define SAMPLING_FREQUENCY  7500.0f
#define PWM_PERIOD SECOND/2
#define PWM_COUNTER_PERIOD SECOND / 4
#define SAMPLING_DELAY  (1.0f/SAMPLING_FREQUENCY)*SECOND
int sampleIndex = 0;

boolean stopping = false;
void setup() {
  pinMode(PWM_OUTPUT_PIN, OUTPUT); //SETUP PWM OUTPUT SIGNAL
  // put your setup code here, to run once:
  samples = malloc (IMAGE_CAPTURE_SAMPLES * sizeof *samples);
  zeros = malloc (IMAGE_CAPTURE_SAMPLES * sizeof *samples);
  Serial.begin(9600);
  Serial.print("Start!: ");
  Serial.println(sampleIndex);
}

long systemTime() {
  return micros();
}

long nextRunTime = 0;
void recordMeasurement() {
  if(nextRunTime == 0 || systemTime() >= nextRunTime) {
    nextRunTime = systemTime() + SAMPLING_DELAY;
    //Check if we need to record smaples
    if(sampleIndex < IMAGE_CAPTURE_SAMPLES) {
      samples[sampleIndex] = analogRead(IMAGE_CAPTURE_PIN);
      zeros[sampleIndex] = 0;
      sampleIndex++;
    } else {
      Serial.print("Running Operation: ");
      signed int maxFrequency = optfft(samples, zeros);
      Serial.print("Max: ");
      Serial.println(samples[maxFrequency]);
      nextRunTime = 0;
      sampleIndex = 0;
    }
  }
}

boolean inc = false;
boolean dec = false;
boolean start = false;


void PWM() {
  static boolean isPWMOn = false;
  static long nextPWMRunTime = 0;
  static long nextCounterRunTime = 0;
  static float dutyCycle = DEFAULT_DUTY_CYCLE;
  static float elapsedTime = 0;
  boolean outputPWM = false;
  
  if(inc) {
    dutyCycle = min(0.9f, dutyCycle + 0.1f);
    inc = false;
  }
  if(dec) {
    dutyCycle = max(0.1f, dutyCycle - 0.1f);
    dec = false;
  }
  
  if(nextPWMRunTime == 0 || systemTime() >= nextPWMRunTime) {
    if(isPWMOn) {
      nextPWMRunTime = systemTime() + PWM_PERIOD * (1.0f - dutyCycle);
      outputPWM = false;
    } else {
      nextPWMRunTime = systemTime() + PWM_PERIOD * (dutyCycle);
      outputPWM = true;
    }
    isPWMOn = !isPWMOn;
    digitalWrite(PWM_OUTPUT_PIN, outputPWM ? HIGH : LOW);
    Serial.println(outputPWM ? "High" : "Low");
  }

  if(nextCounterRunTime == 0 || systemTime() >= nextCounterRunTime) {
    nextCounterRunTime = systemTime() + PWM_COUNTER_PERIOD;
    elapsedTime += PWM_COUNTER_PERIOD * (DEFAULT_DUTY_CYCLE + dutyCycle);
    if(elapsedTime >= DEPLOY_RETRACT_TIME) {
      //Interupt
      digitalWrite(PWM_OUTPUT_PIN, LOW);

      //Reset values
      isPWMOn = false;
      nextPWMRunTime = 0;
      nextCounterRunTime = 0;
      dutyCycle = DEFAULT_DUTY_CYCLE;
      elapsedTime = 0;

      //Send stop
      stopping = true;
    }
  }
}

void processesEarthInput(char in) {
  if(in == '1') {
    inc = true;
  } else if(in == '2') {
    dec = true;
  } else if(in == '3') {
    start = true;
  }
}


void loop() {
  while (Serial.available() > 0) {
    char input = Serial.read();
    processesEarthInput(input);
    
  }
  if(!stopping) {
    PWM();
  } else if(start){
    start = false;
    stopping = false;
  }
}
