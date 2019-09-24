
// The following library can be downloaded from
// https://github.com/luni64/TeensyDelay
#include <TeensyDelay.h>


// Create an IntervalTimer object
IntervalTimer myTimer1;
IntervalTimer myTimer2;
IntervalTimer myTimer3;


// 1kHz  fly-view camera
// 100 Hz  fluo-view camera
// 100 Hz  flash and arena-view cameras

const int ArenaView_Pin = 0;
const int FlyView_Pin 	= 1;
const int FluoView_Pin 	= 2;
const int Flash_Pin 	= 3;

// define periods in us
const unsigned long ArenaView_Period	= 10 * 1000; // 10 ms = 100Hz
const unsigned long FlyView_Period 		= 1000;		 //  1 ms = 1kHz
const unsigned long FluoView_Period 	= 10 * 1000; // 10 ms = 100Hz

// define durations in us
const unsigned long ArenaView_Duration	= 100;	// 100 us
const unsigned long FlyView_Duration 	= 100;	// 100 us
const unsigned long FluoView_Duration 	= 5000; //   5 ms

// Flash times in ms (not us)
const unsigned long Flash_Interval_ms 	= 60 * 1000; // 60 seconds
const unsigned long Flash_Duration_ms 	= 100;		 // 100 ms

bool flashTriggered = false;
bool flashAtNextTimeWidow = false;
elapsedMillis timeSinceFlash;

void setup() {
	pinMode(ArenaView_Pin, OUTPUT);
	pinMode(FlyView_Pin, OUTPUT);
	pinMode(FluoView_Pin, OUTPUT);
	pinMode(Flash_Pin, OUTPUT);

	digitalWrite(ArenaView_Pin, LOW);
	digitalWrite(FlyView_Pin, LOW);
	digitalWrite(FluoView_Pin, LOW);
	digitalWrite(Flash_Pin, HIGH);

	Serial.begin(9600);
	delay(200);
	Serial.println("Starting Flyception timing control...");
	Serial.println("");
	Serial.print("ArenaView: ");
	Serial.print(ArenaView_Duration/1000.0);
	Serial.print(" ms pulses at ");
	Serial.print(1000000.0/ArenaView_Period);
	Serial.println(" Hz.");
	Serial.print("FlyView: ");
	Serial.print(FlyView_Duration/1000.0);
	Serial.print(" ms pulses at ");
	Serial.print(1000000.0/FlyView_Period);
	Serial.println(" Hz.");
	Serial.print("FluoView: ");
	Serial.print(FluoView_Duration/1000.0);
	Serial.print(" ms pulses at ");
	Serial.print(1000000.0/FluoView_Period);
	Serial.println(" Hz.");

	Serial.println("");
	Serial.println("Press <Return> to trigger flash.");

	TeensyDelay::begin();                 // setup timer
	TeensyDelay::addDelayChannel(ArenaViewOff, 0);    // add a delay channel and attach callback function
	TeensyDelay::addDelayChannel(FlyViewOff, 1);    // add a delay channel and attach callback function
	TeensyDelay::addDelayChannel(FluoViewOff, 2);    // add a delay channel and attach callback function

	myTimer1.begin(pulseArenaView, ArenaView_Period);
	myTimer2.begin(pulseFlyView, FlyView_Period);
	myTimer3.begin(pulseFluoView, FluoView_Period);
}

// functions called by IntervalTimer should be short, run as quickly as
// possible, and should avoid calling other functions if possible.
void pulseArenaView() {
	digitalWriteFast(ArenaView_Pin, HIGH);
	TeensyDelay::trigger(ArenaView_Duration, 0);
	if (flashAtNextTimeWidow) {
		digitalWriteFast(Flash_Pin, LOW);
		flashAtNextTimeWidow = false;
	}
}
void pulseFlyView() {
	digitalWriteFast(FlyView_Pin, HIGH);
	TeensyDelay::trigger(FlyView_Duration, 1);
}
void pulseFluoView() {
	digitalWriteFast(FluoView_Pin, HIGH);
	TeensyDelay::trigger(FluoView_Duration, 2);
}
void pulseFlash() {
	// TeensyDelay won't work (without some tricks) for durations > 65ms
	// We will use regular 'delay()' here, even though we won't catch Serial input
	// until after function returns.
	flashAtNextTimeWidow = true;
	// digitalWriteFast(Flash_Pin, LOW);
	delay(Flash_Duration_ms);
	digitalWriteFast(Flash_Pin, HIGH);
}

void ArenaViewOff() {digitalWriteFast(ArenaView_Pin,LOW);}
void FlyViewOff() {digitalWriteFast(FlyView_Pin,LOW);}
void FluoViewOff() {digitalWriteFast(FluoView_Pin,LOW);}


// The main program will print the blink count
// to the Arduino Serial Monitor
void loop() {

	// Check for any incoming characters, if so trigger flash
	if (Serial.available() > 0) {

		// 1. Eat up all they bytes (we don't care about the content or how many bytes)
		while (Serial.available() > 0) {
			Serial.read();
		}

		// 2. generate flash and trigger future flash
		pulseFlash();
		flashTriggered = true;
		timeSinceFlash = 0;
	}

	// generate triggered flash, if needed
	if (flashTriggered && (timeSinceFlash >= Flash_Interval_ms)) {
		pulseFlash();
		flashTriggered = false;
	}

	delay(10);
}
