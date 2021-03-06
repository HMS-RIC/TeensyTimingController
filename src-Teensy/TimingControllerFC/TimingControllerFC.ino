
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
const unsigned long FlyView_Duration 	= 900;	// 900 us
const unsigned long FluoView_Duration 	= 5000; //   5 ms (duration doesn't matter; FluoView shutter is set in software)
const unsigned long Flash_Duration 	    = 1000;	//   1 ms (duration doesn't matter. Flash duration set on flash.)

// flash interval in ms (not us)
const unsigned long Default_Flash_Interval_ms 	= 17 * 1000; // 17 seconds
unsigned long Flash_Interval_ms = Default_Flash_Interval_ms;

// timing offsets
const unsigned long Fluo_Pre_Trig    = 3000; //  3 ms
const unsigned long FlyView_Pre_Trig 	    = 150; // 150 us

// pulse polarity
const char ArenaView_ON = HIGH;   const char ArenaView_OFF = LOW;
const char FlyView_ON   = LOW;    const char FlyView_OFF   = HIGH;
const char FluoView_ON  = HIGH;   const char FluoView_OFF  = LOW;
const char Flash_ON     = LOW;    const char Flash_OFF     = HIGH;

bool flashTriggered = false;
volatile bool flashAtNextTimeWidow = false;
elapsedMillis timeSinceFlash;

// Create IntervalTimer objects (up to 4 can be created on Teensy 3.2)
IntervalTimer arenaViewInervalTimer;
IntervalTimer flyViewIntervalTimer;
IntervalTimer fluoViewIntervalTimer;
IntervalTimer flashIntervalTimer;

void setup() {

	// reduce jitter??
	//FMC_PFAPR |= 0x00FF0000; // Disable prefetching
	SCB_SHPR3 = 0x20200000;  // Systick = priority 32 (defaults to zero)
	//

	pinMode(ArenaView_Pin, OUTPUT);
	pinMode(FlyView_Pin, OUTPUT);
	pinMode(FluoView_Pin, OUTPUT);
	pinMode(Flash_Pin, OUTPUT);

	digitalWrite(ArenaView_Pin, ArenaView_OFF);
	digitalWrite(FlyView_Pin, FlyView_OFF);
	digitalWrite(FluoView_Pin, FluoView_OFF);
	digitalWrite(Flash_Pin, Flash_OFF);

	Serial.begin(9600);
	Serial.setTimeout(10); // 10 ms timeout for parsing
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
	arenaViewInervalTimer.priority(0);
	flyViewIntervalTimer.priority(0);
	fluoViewIntervalTimer.priority(0);
	flashIntervalTimer.priority(0);

	fluoViewIntervalTimer.begin(pulseFluoView, FluoView_Period);
	delayMicroseconds(Fluo_Pre_Trig);
	flyViewIntervalTimer.begin(pulseFlyView, FlyView_Period);
	delayMicroseconds(FlyView_Pre_Trig);
	noInterrupts();
	arenaViewInervalTimer.begin(pulseArenaView, ArenaView_Period);
	flashIntervalTimer.begin(checkFlash, ArenaView_Period); // check for flash at AV interval
	interrupts();

	arenaViewInervalTimer.priority(0);
	flyViewIntervalTimer.priority(0);
	fluoViewIntervalTimer.priority(0);
	flashIntervalTimer.priority(0);
}

// functions called by IntervalTimer should be short, run as quickly as
// possible, and should avoid calling other functions if possible.
void pulseArenaView() {
	digitalWriteFast(ArenaView_Pin, ArenaView_ON);
	TeensyDelay::trigger(ArenaView_Duration, 0);
}
void pulseFlyView() {
	digitalWriteFast(FlyView_Pin, FlyView_ON);
	TeensyDelay::trigger(FlyView_Duration, 1);
}
void pulseFluoView() {
	digitalWriteFast(FluoView_Pin, FluoView_ON);
	TeensyDelay::trigger(FluoView_Duration, 2);
}
// void pulseFlash() {
// 	// Don't actually flash yet.
// 	// Instead set a flag to flash at the next arenaView/fluoView frame.
// 	flashAtNextTimeWidow = true;
// }
void checkFlash() {
	if (flashAtNextTimeWidow) {
		digitalWriteFast(Flash_Pin, Flash_ON);
		flashAtNextTimeWidow = false;
        TeensyDelay::trigger(Flash_Duration, 3);
	}
}

void ArenaViewOff() {digitalWriteFast(ArenaView_Pin, ArenaView_OFF);}
void FlyViewOff() {digitalWriteFast(FlyView_Pin, FlyView_OFF);}
void FluoViewOff() {digitalWriteFast(FluoView_Pin, FluoView_OFF);}
void FlashOff() {digitalWriteFast(Flash_Pin, Flash_OFF);}


void loop() {

	// Check for any incoming characters, if so trigger flash
	if (Serial.available() > 0) {

		// 1. Get delay value (if present)
		long delay = Serial.parseInt(); // has a 10ms timeout
		// Eat up any extra bytes
		while (Serial.available() > 0) {
			Serial.read();
		}

		// 2. generate flash and trigger future flash
		flashAtNextTimeWidow = false; // 2020-01-14 Do not fire immediate flash
		// pulseFlash();
		flashTriggered = true;
		timeSinceFlash = 0;
		Flash_Interval_ms = delay;
	}

	// generate triggered flash, if needed
	if (flashTriggered && (timeSinceFlash >= Flash_Interval_ms)) {
		flashAtNextTimeWidow = true;
		// pulseFlash();
		flashTriggered = false;
	}

	delay(10);
}
