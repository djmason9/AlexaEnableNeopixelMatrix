#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

const uint8_t kMatrixWidth  = 16;
const uint8_t kMatrixHeight = 16;
const bool    kMatrixSerpentineLayout = false;

// Fixed definitions cannot change on the fly.
#define LED_DT 5 // Data pin to connect to the strip.
#define READY_PIN 2 // Data pin to connect to the ready LED.
#define VIRTUAL_PIN V0 //Blynk's Virtual pin that is set when API is called from https://blynkapi.docs.apiary.io/#reference/0/write-pin-value-via-get/write-pin-value-via-get?console=1
#define COLOR_ORDER RGB  // Are they RGB, GRB or what??
#define LED_TYPE WS2812B // Don't forget to change LEDS.addLeds
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)     // Number of LED's.
#define MAX_DIMENSION ((kMatrixWidth>kMatrixHeight) ? kMatrixWidth : kMatrixHeight)
#define HOLD_PALETTES_X_TIMES_AS_LONG 1

uint8_t noise[MAX_DIMENSION][MAX_DIMENSION];
// Initialize changeable global variables.
uint8_t max_bright = 255; // Overall brightness definition. It can be changed on the fly.
uint16_t scale = 30;
struct CRGB leds[NUM_LEDS]; // Initialize our LED array.
uint8_t       colorLoop = 1;
// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
uint16_t speed = 1; // speed is set dynamically once we've started up
static uint16_t x;
static uint16_t y;
static uint16_t z;

// Define variables used by the sequences.
uint8_t thisfade = 5;         // How quickly does it fade? Lower = slower fade rate.
int thishue = 50;             // Starting hue.
uint8_t thisinc = 1;          // Incremental value for rotating hues
uint8_t thissat = 255;        // The saturation, where 255 = brilliant colours.
uint8_t thisbri = 255;        // Brightness of a sequence. Remember, max_bright is the overall limiter.
int huediff = 256;            // Range of random #'s to use for hue
uint8_t thisdelay = 5;        // We don't need much delay (if any)

// Routine specific variables
uint8_t numdots = 4; // Number of dots in use.
uint8_t thisdiff = 16; // Incremental change in hue between each dot.
uint8_t curhue = 0; // The current hue
uint8_t thisbeat = 5; // Higher = faster movement.

uint8_t fadeval = 224;
int8_t bpm = 30;

CRGBPalette16 currentPalette; // Use palettes instead of direct CHSV or CRGB assignments
CRGBPalette16 targetPalette;  // Also support smooth palette transitioning
TBlendType currentBlending;   // NOBLEND or LINEARBLEND

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "[YOUR KEY HERE]";

// WiFi parameters
const char *ssid = "[YOUR SSID HERE]";
const char *password = "[YOUR NETWORK PASSWORD HERE]";

// Create an instance of the server
String motd;
// Declare functions to be exposed to the API
int ledControl(String command);

// kick off with it off
int restCallState = -1;
