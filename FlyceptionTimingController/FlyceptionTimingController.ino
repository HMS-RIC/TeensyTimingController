
// The following library can be downloaded from
// https://github.com/luni64/TeensyDelay
#include <TeensyDelay.h>

// define output pins
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
const unsigned long Flash_Duration_ms 	= 1;		 // 1 ms

bool flashTriggered = false;
bool flashAtNextTimeWidow = false;
elapsedMillis timeSinceFlash;

// Create IntervalTimer objects
IntervalTimer arenaViewInervalTimer;
IntervalTimer flyViewIntervalTimer;
IntervalTimer fluoViewIntervalTimer;

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

	// Set up delay timers to turn off pulses
	TeensyDelay::begin();
	TeensyDelay::addDelayChannel(ArenaViewOff, 0);  // Delay timer 0: Turn off ArenaView pulse
	TeensyDelay::addDelayChannel(FlyViewOff, 1);    // Delay timer 1: Turn off FlyView pulse
	TeensyDelay::addDelayChannel(FluoViewOff, 2);   // Delay timer 2: Turn off FluoView pulse
	TeensyDelay::addDelayChannel(FlashOff, 3);      // Delay timer 3: Turn off Flash pulse

	// Start up interval timers to generate periodic pulses
	arenaViewInervalTimer.begin(pulseArenaView, ArenaView_Period);
	flyViewIntervalTimer.begin(pulseFlyView, FlyView_Period);
	fluoViewIntervalTimer.begin(pulseFluoView, FluoView_Period);
}

// functions called by IntervalTimer should be short, run as quickly as
// possible, and should avoid calling other functions if possible.
void pulseArenaView() {
	digitalWriteFast(ArenaView_Pin, HIGH);
	TeensyDelay::trigger(ArenaView_Duration, 0);
	if (flashAtNextTimeWidow) {
		digitalWriteFast(Flash_Pin, LOW);
		flashAtNextTimeWidow = false;
    TeensyDelay::trigger(FluoView_Duration, 3);
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
	// Don't actually flash yet.
	// Instead set a flag to flash at the next arenaView/fluoView frame.
	flashAtNextTimeWidow = true;
}

void ArenaViewOff() {digitalWriteFast(ArenaView_Pin, LOW);}
void FlyViewOff() {digitalWriteFast(FlyView_Pin, LOW);}
void FluoViewOff() {digitalWriteFast(FluoView_Pin, LOW);}
void FlashOff() {digitalWriteFast(Flash_Pin, HIGH);}


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
