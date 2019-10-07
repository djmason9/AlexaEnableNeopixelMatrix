#include "FastLED.h" // FastLED library. Preferably the latest copy of FastLED 2.1.
#include "arcade.h"
#include "config.h"
#include <BlynkSimpleEsp32.h>
#include <TimedAction.h>
#include <WiFi.h>
#include <WiFiClient.h>


// Create an instance of the server
WiFiServer server(80);
/*****************************************************
**** Add this before the TimedAction call or fail.**** 
/*****************************************************/
void checkforRestCall()
{
    ledControl((String)restCallState);
}
//https://examples.blynk.cc for details
BLYNK_WRITE(VIRTUAL_PIN)
{
    Serial.println("WebHook data:");
    Serial.println(param.asStr());
    //Setting the current int for the ledControl function.
    restCallState = param.asInt();
}

//This makes our code multi-threaded (well kinda)
TimedAction timedAction = TimedAction(5, checkforRestCall); //


/**
 *  The main setup function where all your setup code lives
 */
void setup()
{

    // setup the Blynk code 
    Blynk.begin(auth, ssid, password);

    // Initialize our coordinates to some random values
    x = random16();
    y = random16();
    z = random16();

    // Sets the pin mode for the ready pin
    pinMode(READY_PIN, OUTPUT);

    delay(1000); // Power-up safety delay or something like that.
    Serial.begin(115200);

    // Sets up our LED Matrix
    FastLED.addLeds<LED_TYPE, LED_DT, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(max_bright);
    FastLED.setMaxPowerInVoltsAndMilliamps(20, 500);

    Serial.print("Delay 2 seconds before wifi.");
    delay(2000);
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    // Start the server
    server.begin();
    Serial.println("Server started");

    // Print the IP address
    Serial.println(WiFi.localIP());
    // Turns on the ready light (optional)
    digitalWrite(READY_PIN, HIGH);

    currentPalette = CRGBPalette16(CRGB::Black);
    targetPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;

} // setup()

/**
 * Main loop of the program
 */
void loop()
{
    Blynk.run();
    timedAction.check(); //this is used for "multi-threading" calls checkforRestCall on each iteration.
    
    // Handle REST calls
    WiFiClient client = server.available();
    if (!client) {
        return;
    }
    while (!client.available()) {
        delay(1);
    }

} // loop()

/*********************************************/
/*************** lED ANIMATIONS **************/
/*********************************************/
void confetti()
{ // random colored speckles that blink in and fade smoothly
    fadeToBlackBy(leds, NUM_LEDS, thisfade); // Low values = slower fade.
    int pos = random16(NUM_LEDS); // Pick an LED at random.
    leds[pos] += CHSV((thishue + random16(huediff)) / 4, thissat, thisbri); // I use 12 bits for hue so that the hue increment isn't too quick.
    thishue = thishue + thisinc; // It increments here.
} // confetti()

void ChangeMe()
{ // A time (rather than loop) based demo sequencer. This gives us full control over the length of each sequence.
    uint8_t secondHand = (millis() / 1000) % 15; // IMPORTANT!!! Change '15' to a different value to change duration of the loop.
    static uint8_t lastSecond = 99; // Static variable, means it's only defined once. This is our 'debounce' variable.
    if (lastSecond != secondHand) { // Debounce to make sure we're not repeating an assignment.
        lastSecond = secondHand;
        switch (secondHand) {
        case 0:
            thisinc = 1;
            thishue = 192;
            thissat = 255;
            thisfade = 2;
            huediff = 256;
            break; // You can change values here, one at a time , or altogether.
        case 5:
            thisinc = 2;
            thishue = 128;
            thisfade = 8;
            huediff = 64;
            break;
        case 10:
            thisinc = 1;
            thishue = random16(255);
            thisfade = 1;
            huediff = 16;
            break; // Only gets called once, and not continuously for the next several seconds. Therefore, no rainbows.
        case 15:
            break; // Here's the matching 15 for the other one.
        }
    }
} // ChangeMe()

void juggle_pal()
{ // Several colored dots, weaving in and out of sync with each other

    curhue = thishue; // Reset the hue values.
    fadeToBlackBy(leds, NUM_LEDS, thisfade);

    for (int i = 0; i < numdots; i++) {
        leds[beatsin16(thisbeat + i + numdots, 0, NUM_LEDS)] += ColorFromPalette(currentPalette, curhue, thisbri, currentBlending); // Munge the values and pick a colour from the palette
        curhue += thisdiff;
    }
} //juggle_pal()

void dot_beat()
{

    uint8_t inner = beatsin8(bpm, NUM_LEDS / 4, NUM_LEDS / 4 * 3); // Move 1/4 to 3/4
    uint8_t outer = beatsin8(bpm, 0, NUM_LEDS - 1); // Move entire length
    uint8_t middle = beatsin8(bpm, NUM_LEDS / 3, NUM_LEDS / 3 * 2); // Move 1/3 to 2/3

    leds[middle] = CRGB::Purple;
    leds[inner] = CRGB::Blue;
    leds[outer] = CRGB::Aqua;

    nscale8(leds, NUM_LEDS, fadeval); // Fade the entire array. Or for just a few LED's, use  nscale8(&leds[2], 5, fadeval);
} //dot_beat()

void ChangePaletteAndSettingsPeriodically()
{
    uint8_t secondHand = ((millis() / 1000) / HOLD_PALETTES_X_TIMES_AS_LONG) % 60;
    static uint8_t lastSecond = 99;

    if (lastSecond != secondHand) {
        lastSecond = secondHand;
        if (secondHand == 0) {
            currentPalette = RainbowColors_p;
            speed = 20;
            scale = 30;
            colorLoop = 1;
        }
        if (secondHand == 5) {
            SetupPurpleAndGreenPalette();
            speed = 10;
            scale = 50;
            colorLoop = 1;
        }
        if (secondHand == 10) {
            SetupBlackAndWhiteStripedPalette();
            speed = 20;
            scale = 30;
            colorLoop = 1;
        }
        if (secondHand == 15) {
            currentPalette = ForestColors_p;
            speed = 8;
            scale = 120;
            colorLoop = 0;
        }
        if (secondHand == 20) {
            currentPalette = CloudColors_p;
            speed = 4;
            scale = 30;
            colorLoop = 0;
        }
        if (secondHand == 25) {
            currentPalette = LavaColors_p;
            speed = 8;
            scale = 50;
            colorLoop = 0;
        }
        if (secondHand == 30) {
            currentPalette = OceanColors_p;
            speed = 20;
            scale = 90;
            colorLoop = 0;
        }
        if (secondHand == 35) {
            currentPalette = PartyColors_p;
            speed = 20;
            scale = 30;
            colorLoop = 1;
        }
        if (secondHand == 40) {
            SetupRandomPalette();
            speed = 20;
            scale = 20;
            colorLoop = 1;
        }
        if (secondHand == 45) {
            SetupRandomPalette();
            speed = 50;
            scale = 50;
            colorLoop = 1;
        }
        if (secondHand == 50) {
            SetupRandomPalette();
            speed = 90;
            scale = 90;
            colorLoop = 1;
        }
        if (secondHand == 55) {
            currentPalette = RainbowStripeColors_p;
            speed = 30;
            scale = 20;
            colorLoop = 1;
        }
    }
}

// This function generates a random palette that's a gradient
// between four different colors.  The first is a dim hue, the second is
// a bright hue, the third is a bright pastel, and the last is
// another bright hue.  This gives some visual bright/dark variation
// which is more interesting than just a gradient of different hues.
void SetupRandomPalette()
{
    currentPalette = CRGBPalette16(
        CHSV(random8(), 255, 32),
        CHSV(random8(), 255, 255),
        CHSV(random8(), 128, 255),
        CHSV(random8(), 255, 255));
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid(currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV(HUE_PURPLE, 255, 255);
    CRGB green = CHSV(HUE_GREEN, 255, 255);
    CRGB black = CRGB::Black;

    currentPalette = CRGBPalette16(
        green, green, black, black,
        purple, purple, black, black,
        green, green, black, black,
        purple, purple, black, black);
}

// Fill the x/y array of 8-bit noise values using the inoise8 function.
void fillnoise8()
{
    // If we're runing at a low "speed", some 8-bit artifacts become visible
    // from frame-to-frame.  In order to reduce this, we can do some fast data-smoothing.
    // The amount of data smoothing we're doing depends on "speed".
    uint8_t dataSmoothing = 0;
    if (speed < 50) {
        dataSmoothing = 200 - (speed * 4);
    }

    for (int i = 0; i < MAX_DIMENSION; i++) {
        int ioffset = scale * i;
        for (int j = 0; j < MAX_DIMENSION; j++) {
            int joffset = scale * j;

            uint8_t data = inoise8(x + ioffset, y + joffset, z);

            // The range of the inoise8 function is roughly 16-238.
            // These two operations expand those values out to roughly 0..255
            // You can comment them out if you want the raw noise data.
            data = qsub8(data, 16);
            data = qadd8(data, scale8(data, 39));

            if (dataSmoothing) {
                uint8_t olddata = noise[i][j];
                uint8_t newdata = scale8(olddata, dataSmoothing) + scale8(data, 256 - dataSmoothing);
                data = newdata;
            }

            noise[i][j] = data;
        }
    }

    z += speed;

    // apply slow drift to X and Y, just for visual variation.
    x += speed / 8;
    y -= speed / 16;
}

void mapNoiseToLEDsUsingPalette()
{
    static uint8_t ihue = 0;

    for (int i = 0; i < kMatrixWidth; i++) {
        for (int j = 0; j < kMatrixHeight; j++) {
            // We use the value at the (i,j) coordinate in the noise
            // array for our brightness, and the flipped value from (j,i)
            // for our pixel's index into the color palette.

            uint8_t index = noise[j][i];
            uint8_t bri = noise[i][j];

            // if this palette is a 'loop', add a slowly-changing base value
            if (colorLoop) {
                index += ihue;
            }

            // brighten up, as the color palette itself often contains the
            // light/dark dynamic range desired
            if (bri > 127) {
                bri = 255;
            } else {
                bri = dim8_raw(bri * 2);
            }

            CRGB color = ColorFromPalette(currentPalette, index, bri);
            leds[XY(i, j)] = color;
        }
    }

    ihue += 1;
}

/************************************************************/
// Mark's xy coordinate mapping code.  See the XYMatrix for more information on it.
/************************************************************/
uint16_t XY(uint8_t x, uint8_t y)
{
    uint16_t i;
    if (kMatrixSerpentineLayout == false) {
        i = (y * kMatrixWidth) + x;
    }
    if (kMatrixSerpentineLayout == true) {
        if (y & 0x01) {
            // Odd rows run backwards
            uint8_t reverseX = (kMatrixWidth - 1) - x;
            i = (y * kMatrixWidth) + reverseX;
        } else {
            // Even rows run forwards
            i = (y * kMatrixWidth) + x;
        }
    }
    return i;
}

/*********************************************/
/************** API CALL CONTROL *************/
/*********************************************/
int ledControl(String command)
{
    // Get state from command
    int state = command.toInt();
    restCallState = state;
    //    Serial.print("ledControl command: ");
    //    Serial.println(state);
    if (state == 1) {
        ChangeMe(); // Check the demo loop for changes to the variables.

        EVERY_N_MILLISECONDS(thisdelay)
        { // FastLED based non-blocking delay to update/display the sequence.
            confetti();
        }
        FastLED.show();
        return 1;
    } else if (state == 2) { //dot
        dot_beat();
        FastLED.show();
        return 2;
    } else if (state == 3) { // juggle
        thisfade = 2; // How long should the trails be. Very low value = longer trails.
        thishue = 0; // Starting hue.
        thissat = 255; // Saturation of the colour. Higher = faster movement.
        EVERY_N_MILLISECONDS(100)
        { // AWESOME palette blending capability provided as required.
            uint8_t maxChanges = 24;
            nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);
        }

        ChangeMe();
        juggle_pal();
        FastLED.show();
        return 3;
    } else if (state == 4) { //Arcade Animations
        int loopcount = 5;

        for (int passtime = 0; passtime < loopcount; passtime++) {
            FastLED.clear();
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = pgm_read_dword(&(Mario01[i])); // Read array from Flash
            }
            FastLED.show();
            delay(200);

            FastLED.clear();
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = pgm_read_dword(&(Mario02[i])); // Read array from Flash
            }
            FastLED.show();
            delay(200);
        }

        for (int passtime = 0; passtime < loopcount; passtime++) {
            FastLED.clear();
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = pgm_read_dword(&(DigDug01[i]));
            }

            FastLED.show();
            delay(250);

            // Put DigDug second frame
            FastLED.clear();
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = pgm_read_dword(&(DigDug02[i]));
            }

            FastLED.show();
            delay(250);
        }

        // show ghost
        for (int passtime = 0; passtime < 2; passtime++) {
            int delaytime = 200;
            FastLED.clear();
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = pgm_read_dword(&(ghost01[i])); // Read array from Flash
            }
            FastLED.show();
            delay(delaytime);

            FastLED.clear();
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = pgm_read_dword(&(ghost02[i])); // Read array from Flash
            }
            FastLED.show();
            delay(delaytime);

            FastLED.clear();
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = pgm_read_dword(&(ghost01[i])); // Read array from Flash
            }
            FastLED.show();
            delay(delaytime);

            FastLED.clear();
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = pgm_read_dword(&(ghost02[i])); // Read array from Flash
            }
            FastLED.show();
            delay(delaytime);

            FastLED.clear();
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = pgm_read_dword(&(ghost03[i])); // Read array from Flash
            }
            FastLED.show();
            delay(delaytime);

            FastLED.clear();
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = pgm_read_dword(&(ghost04[i])); // Read array from Flash
            }
            FastLED.show();
            delay(delaytime);

            FastLED.clear();
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = pgm_read_dword(&(ghost03[i])); // Read array from Flash
            }
            FastLED.show();
            delay(delaytime);

            FastLED.clear();
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = pgm_read_dword(&(ghost04[i])); // Read array from Flash
            }
            FastLED.show();
            delay(delaytime);
        }
        restCallState = 1; //go back to confetti or this will crash at some point
        return 4;
    } else if (state == 5) {
        // Periodically choose a new palette, speed, and scale
        ChangePaletteAndSettingsPeriodically();

        // generate noise data
        fillnoise8();

        // convert the noise data to colors in the LED array
        // using the current palette
        mapNoiseToLEDsUsingPalette();
        FastLED.show();
        return 5;
    } else {
        fadeToBlackBy(leds, NUM_LEDS, thisfade);
        FastLED.show();
        return 9;
    }

} // ledControl()