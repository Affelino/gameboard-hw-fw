//////////////////////////////////////////////////////////////////////
// Firmware: Level 0 Implementation for DIY Chessboard
//
// Implements basic functionality as described in:
// https://github.com/Affelino/gameboard-ui-protocol
//
// Hardware details:
// https://github.com/Affelino/gameboard_hw_fw
//
// Sends pick-up and put-down events, receives LED control commands.
// Does not track game state or validate moves (Level 0).
//
// License and usage terms available in the above repositories.
//////////////////////////////////////////////////////////////////////
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <NeoPixelBus.h>
#include <Wire.h>
#include <MCP23017.h>

//////////////////////////////////////////////////////////////////////
//#define BOARD_TYPE  0		// Board with one LED per square
#define BOARD_TYPE  1   	// Board with 4 LEDs per square

//////////////////////////////////////////////////////////////////////
#define MCP23017_ADDR			0x20
MCP23017 						mcp = MCP23017(MCP23017_ADDR);

//////////////////////////////////////////////////////////////////////
// Which pin on the Arduino is connected to the NeoPixels?
#define PIX_PIN					2     // For ESP-01 using 2 (pick what suits your platform)
//#define PIX_PIN   			D1     // For Wemos D1 mini using D1 (pick what suits your platform)

//////////////////////////////////////////////////////////////////////
#if BOARD_TYPE == 0
#define LEDS_PER_SQUARE 1   // Number of indicator LEDs per square
#elif BOARD_TYPE == 1
#define LEDS_PER_SQUARE 4   // Number of indicator LEDs per square
#else
#error "Unsupported BOARD_TYPE. Use 0 or 1."
#endif

#define NUMPIXELS ( 64 * LEDS_PER_SQUARE )
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> pixels(NUMPIXELS, PIX_PIN);

//////////////////////////////////////////////////////////////////////
#define USE_OTA					1	// Define if OTA update is needed.

// Include the secret info, like wifi password etc. from file
#include "secrets.h"

//////////////////////////////////////////////////////////////////////
WiFiClient						chessClient;
IPAddress						chessServer = IPAddress( 192, 168, 1, 200 );
const int						chessPort = 12344;	// Server connection port

//////////////////////////////////////////////////////////////////////
#define	DEBOUNCE_CNT			3		// 1
#define	ROW_CHANGE_DELAY		1		// 10

#define LED_UPDATE_INTERVAL_MS	100
#define LED_COLOR_BASE          0
#define LED_COLOR_BITS          0x00FFFFFF
#define LED_STATE_BASE          24
#define LED_STATE_BITS          0x0F000000
#define LED_INDEX_BASE          28
#define LED_INDEX_BITS          0xF0000000
#define LED_STATE_OFF			( 0 << 24 )
#define LED_STATE_ON			( 1 << 24 )
#define LED_STATE_BLINK_FAST	( 2 << 24 )
#define LED_STATE_BLINK_SLOW	( 3 << 24 )
#define LED_STATE_ROTATE_FAST   ( 4 << 24 )
#define LED_STATE_ROTATE_SLOW   ( 5 << 24 )
#define LED_BLINK_FAST_RATE		100
#define LED_BLINK_SLOW_RATE		1000

#define WHITE_COLOR_R			0xFF0000
#define BLACK_COLOR_R			0xFF0000
#define WHITE_COLOR_G			0x00FF00
#define BLACK_COLOR_G			0x00FF00
#define WHITE_COLOR_B			0x0000FF
#define BLACK_COLOR_B			0x0000FF
#define DROP_WHITE_COLOR		WHITE_COLOR_G
#define DROP_BLACK_COLOR		BLACK_COLOR_G
#define PICK_WHITE_COLOR		WHITE_COLOR_R
#define PICK_BLACK_COLOR		BLACK_COLOR_R
#define POSSIBLE_WHITE_COLOR	WHITE_COLOR_B
#define POSSIBLE_BLACK_COLOR	BLACK_COLOR_B
#define WARNING_WHITE_COLOR		WHITE_COLOR_R
#define WARNING_BLACK_COLOR		BLACK_COLOR_R
#define PIECE_WHITE_COLOR		WHITE_COLOR_G
#define PIECE_BLACK_COLOR		BLACK_COLOR_G

#if BOARD_TYPE == 0
#define DROP_STATE_BLINK		LED_STATE_BLINK_SLOW
#define PICK_STATE_BLINK		LED_STATE_BLINK_SLOW
#elif BOARD_TYPE == 1
#define DROP_STATE_BLINK		LED_STATE_ROTATE_FAST
#define PICK_STATE_BLINK		LED_STATE_ROTATE_FAST
#endif
#define POSSIBLE_STATE_BLINK	LED_STATE_BLINK_SLOW
#define WARNING_STATE_BLINK		LED_STATE_BLINK_FAST
#define PIECE_STATE_BLINK		LED_STATE_BLINK_FAST

#define	DROP_MESSAGE			'+'
#define	PICK_MESSAGE			'-'
#define	POSSIBLE_MESSAGE		'*'
#define	WARNING_MESSAGE			'!'
#define	CLEAR_MESSAGE			'.'
#define	PIECE_MESSAGE			'^'


typedef struct square_t {
	uint8_t		ledIdx[LEDS_PER_SQUARE];       	// Index of the idicator led for this square
#if BOARD_TYPE == 1
	uint32_t	prevChange;						// Timestamp for last rotate event for 4 LED square, not used on single led board
#endif
	uint32_t	ledInfo;      					// State and color of the led (on, off, blinking... RGB)
	uint8_t		state;
	uint8_t		stateDebounce;
} square_t;

#if BOARD_TYPE == 0
square_t    board_g[8][8] = {
                            { {{ 0},0,0,0}, {{ 1},0,0,0}, {{ 2},0,0,0}, {{ 3},0,0,0}, {{ 4},0,0,0}, {{ 5},0,0,0}, {{ 6},0,0,0}, {{ 7},0,0,0 } },
                            { {{15},0,0,0}, {{14},0,0,0}, {{13},0,0,0}, {{12},0,0,0}, {{11},0,0,0}, {{10},0,0,0}, {{ 9},0,0,0}, {{ 8},0,0,0 } },
                            { {{16},0,0,0}, {{17},0,0,0}, {{18},0,0,0}, {{19},0,0,0}, {{20},0,0,0}, {{21},0,0,0}, {{22},0,0,0}, {{23},0,0,0 } },
                            { {{31},0,0,0}, {{30},0,0,0}, {{29},0,0,0}, {{28},0,0,0}, {{27},0,0,0}, {{26},0,0,0}, {{25},0,0,0}, {{24},0,0,0 } },
                            { {{32},0,0,0}, {{33},0,0,0}, {{34},0,0,0}, {{35},0,0,0}, {{36},0,0,0}, {{37},0,0,0}, {{38},0,0,0}, {{39},0,0,0 } },
                            { {{47},0,0,0}, {{46},0,0,0}, {{45},0,0,0}, {{44},0,0,0}, {{43},0,0,0}, {{42},0,0,0}, {{41},0,0,0}, {{40},0,0,0 } },
                            { {{48},0,0,0}, {{49},0,0,0}, {{50},0,0,0}, {{51},0,0,0}, {{52},0,0,0}, {{53},0,0,0}, {{54},0,0,0}, {{55},0,0,0 } },
                            { {{63},0,0,0}, {{62},0,0,0}, {{61},0,0,0}, {{60},0,0,0}, {{59},0,0,0}, {{58},0,0,0}, {{57},0,0,0}, {{56},0,0,0 } }
                          };
#elif BOARD_TYPE == 1
square_t    board_g[8][8] = {
                            { {{  0,  1, 30, 31},0,0,0}, {{  2,  3, 28, 29},0,0,0}, {{  4,  5, 26, 27},0,0,0}, {{  6,  7, 24, 25},0,0,0}, {{  8,  9, 22, 23},0,0,0}, {{ 10, 11, 20, 21},0,0,0}, {{ 12, 13, 18, 19},0,0,0}, {{ 14, 15, 16, 17},0,0,0 } },
                            { {{ 32, 33, 62, 63},0,0,0}, {{ 34, 35, 60, 61},0,0,0}, {{ 36, 37, 58, 59},0,0,0}, {{ 38, 39, 56, 57},0,0,0}, {{ 40, 41, 54, 55},0,0,0}, {{ 42, 43, 52, 53},0,0,0}, {{ 44, 45, 50, 51},0,0,0}, {{ 46, 47, 48, 49},0,0,0 } },
                            { {{ 64, 65, 94, 95},0,0,0}, {{ 66, 67, 92, 93},0,0,0}, {{ 68, 69, 90, 91},0,0,0}, {{ 70, 71, 88, 89},0,0,0}, {{ 72, 73, 86, 87},0,0,0}, {{ 74, 75, 84, 85},0,0,0}, {{ 76, 77, 82, 83},0,0,0}, {{ 78, 79, 80, 81},0,0,0 } },
                            { {{ 96, 97,126,127},0,0,0}, {{ 98, 99,124,125},0,0,0}, {{100,101,122,123},0,0,0}, {{102,103,120,121},0,0,0}, {{104,105,118,119},0,0,0}, {{106,107,116,117},0,0,0}, {{108,109,114,115},0,0,0}, {{110,111,112,113},0,0,0 } },
                            { {{128,129,158,159},0,0,0}, {{130,131,156,157},0,0,0}, {{132,133,154,155},0,0,0}, {{134,135,152,153},0,0,0}, {{136,137,150,151},0,0,0}, {{138,139,148,149},0,0,0}, {{140,141,146,147},0,0,0}, {{142,143,144,145},0,0,0 } },
                            { {{160,161,190,191},0,0,0}, {{162,163,188,189},0,0,0}, {{164,165,186,187},0,0,0}, {{166,167,184,185},0,0,0}, {{168,169,182,183},0,0,0}, {{170,171,180,181},0,0,0}, {{172,173,178,179},0,0,0}, {{174,175,176,177},0,0,0 } },
                            { {{192,193,222,223},0,0,0}, {{194,195,220,221},0,0,0}, {{196,197,218,219},0,0,0}, {{198,199,216,217},0,0,0}, {{200,201,214,215},0,0,0}, {{202,203,212,213},0,0,0}, {{204,205,210,211},0,0,0}, {{206,207,208,209},0,0,0 } },
                            { {{224,225,254,255},0,0,0}, {{226,227,252,253},0,0,0}, {{228,229,250,251},0,0,0}, {{230,231,248,249},0,0,0}, {{232,233,246,247},0,0,0}, {{234,235,244,245},0,0,0}, {{236,237,242,243},0,0,0}, {{238,239,240,241},0,0,0 } }
                          };
#endif


char      hostName[30];

//////////////////////////////////////////////////////////////////////
void  sendBoardMessage( char *sendMessageData, int sendMessageLen ) {

	if ( ! chessClient.connected()) {
		chessClient.connect( chessServer, chessPort );
		// Reset state since we have lost the connection...
		chessClient.write( hostName, strlen( hostName ) );
	}
	chessClient.write( sendMessageData, sendMessageLen );
}

//////////////////////////////////////////////////////////////////////
void clearBoardLights() {

	for ( int i = 0; i < 64; i++ ) {
		board_g[ i / 8 ][ i % 8 ].ledInfo = 0;
	}
}

//////////////////////////////////////////////////////////////////////
void inTo( uint8_t rank, uint8_t file ) {

	char      messageData[20];

	sprintf( messageData, "+%c%d\n", file + 'a', rank + 1 );
	sendBoardMessage( messageData, strlen( messageData ) );
}

//////////////////////////////////////////////////////////////////////
void outFrom( uint8_t rank, uint8_t file ) {

	char      messageData[20];

	sprintf( messageData, "-%c%d\n", file + 'a', rank + 1 );
	sendBoardMessage( messageData, strlen( messageData ) );
}

//////////////////////////////////////////////////////////////////////
void  handleBoardMessage( char *receivedMessage, int receivedMessageLen ) {

	uint32_t	ledColor, ledState;
	uint8_t		moveRank, moveFile;

	receivedMessage[receivedMessageLen] = 0;

	// Is this a take out/put in message?
	if ( receivedMessageLen == 4 ) {
		moveRank = receivedMessage[2] - '1';
		moveFile = receivedMessage[1] - 'a';
		ledState = board_g[moveRank][moveFile].ledInfo & LED_STATE_BITS;
		ledColor = board_g[moveRank][moveFile].ledInfo & LED_COLOR_BITS;

		switch ( receivedMessage[0] ) {
			case DROP_MESSAGE:
				ledColor = ( ( moveRank + moveFile ) % 2 ) ? DROP_WHITE_COLOR : DROP_BLACK_COLOR;
				ledState = DROP_STATE_BLINK;
				break;
			case PICK_MESSAGE:
				ledColor = ( ( moveRank + moveFile ) % 2 ) ? PICK_WHITE_COLOR : PICK_BLACK_COLOR;
				ledState = PICK_STATE_BLINK;
				break;
			case POSSIBLE_MESSAGE:
				ledColor = ( ( moveRank + moveFile ) % 2 ) ? POSSIBLE_WHITE_COLOR : POSSIBLE_BLACK_COLOR;
				ledState = POSSIBLE_STATE_BLINK;
				break;
			case WARNING_MESSAGE:
				ledColor = ( ( moveRank + moveFile ) % 2 ) ? WARNING_WHITE_COLOR : WARNING_BLACK_COLOR;
				ledState = WARNING_STATE_BLINK;
				break;
			case CLEAR_MESSAGE:
				ledColor = 0;
				ledState = 0;
				break;
			case PIECE_MESSAGE:
				ledColor = ( ( moveRank + moveFile ) % 2 ) ? PIECE_WHITE_COLOR : PIECE_BLACK_COLOR;
				ledState = PIECE_STATE_BLINK;
				break;
		}

		board_g[moveRank][moveFile].ledInfo = ledState | ledColor;

	} else if ( receivedMessage[0] == CLEAR_MESSAGE ) {
		clearBoardLights();
	}
}

//////////////////////////////////////////////////////////////////////
void checkRank() {

	static uint8_t        	scanRank = 0, prevScanRank = !scanRank, scanBits = 0xFF, scanBitsFilter = 0x00, tableRanks[8], prevScanBits = !scanBits;
	uint8_t               	scanFile = 0, changedBits;
	static int            	initCounter = DEBOUNCE_CNT * 2;
	unsigned long			msStamp;
	static unsigned long	nextUpdateStamp = 0;

	msStamp = millis();

	if ( msStamp >= nextUpdateStamp ) {

		if ( scanRank != prevScanRank ) {
			mcp.writePort( MCP23017Port::A, ~(1 << scanRank) );
			prevScanRank = scanRank;
			prevScanBits = tableRanks[scanRank];
			nextUpdateStamp = msStamp + ROW_CHANGE_DELAY;

		} else {
			scanBits = mcp.readPort(MCP23017Port::B);
			for ( scanFile = 0; scanFile < 8; scanFile++ ) {
				if ( scanBits & ( 1 << scanFile ) ) { // Square became occupied
					if ( board_g[scanRank][scanFile].stateDebounce < DEBOUNCE_CNT )
						board_g[scanRank][scanFile].stateDebounce++;
					else if ( board_g[scanRank][scanFile].state == 0 ) {
						board_g[scanRank][scanFile].state = 1;
						if( initCounter == 0 )
							inTo( scanRank, scanFile );
					}
				} else {  // Square became free
					if ( board_g[scanRank][scanFile].stateDebounce > 0 )
						board_g[scanRank][scanFile].stateDebounce--;
					else if ( board_g[scanRank][scanFile].state == 1 ) {
						board_g[scanRank][scanFile].state = 0;
						outFrom( scanRank, scanFile );
					}
				}
			}
			tableRanks[scanRank] = scanBits;
			scanRank++;
			if ( scanRank >= 8 ) {
				scanRank = 0;
				if( initCounter > 0 )
					initCounter--;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
void  updateTableLeds() {

  unsigned long         msStamp;
  static unsigned long  nextUpdateStamp = 0, previousFastChange = 0, previousSlowChange = 0;
  uint32_t              ledIndex, ledState, ledColor;
  int                   i;

    
  msStamp = millis();

  if( msStamp >= nextUpdateStamp ) {
    for( int rowIdx = 0; rowIdx < 8; rowIdx++ ) {
      for( int colIdx = 0; colIdx < 8; colIdx++ ) {
        ledIndex = ( board_g[rowIdx][colIdx].ledInfo & LED_INDEX_BITS ) >> LED_INDEX_BASE;
        ledState = board_g[rowIdx][colIdx].ledInfo & LED_STATE_BITS;
        ledColor = board_g[rowIdx][colIdx].ledInfo & LED_COLOR_BITS;

        switch( ledState ) {
          case LED_STATE_ON:
            for( i = 0; i < LEDS_PER_SQUARE; i++ )
              pixels.SetPixelColor( board_g[rowIdx][colIdx].ledIdx[i], RgbColor( (uint8_t)( ledColor >> 16 ), (uint8_t)( ledColor >> 8 ), (uint8_t)ledColor ) );
            break;
          case LED_STATE_OFF:
            ledColor = 0;
            for( i = 0; i < LEDS_PER_SQUARE; i++ )
              pixels.SetPixelColor( board_g[rowIdx][colIdx].ledIdx[i], RgbColor( (uint8_t)( ledColor >> 16 ), (uint8_t)( ledColor >> 8 ), (uint8_t)ledColor ) );
            break;
          case LED_STATE_BLINK_SLOW:
            if( ( msStamp / LED_BLINK_SLOW_RATE ) & 1 )
              ledColor = 0;
            for( i = 0; i < LEDS_PER_SQUARE; i++ )
              pixels.SetPixelColor( board_g[rowIdx][colIdx].ledIdx[i], RgbColor( (uint8_t)( ledColor >> 16 ), (uint8_t)( ledColor >> 8 ), (uint8_t)ledColor ) );
            break;
          case LED_STATE_BLINK_FAST:
            if( ( msStamp / LED_BLINK_FAST_RATE ) & 1 )
              ledColor = 0;
            for( i = 0; i < LEDS_PER_SQUARE; i++ )
              pixels.SetPixelColor( board_g[rowIdx][colIdx].ledIdx[i], RgbColor( (uint8_t)( ledColor >> 16 ), (uint8_t)( ledColor >> 8 ), (uint8_t)ledColor ) );
            break;
#if BOARD_TYPE == 1
          case LED_STATE_ROTATE_SLOW:
            if( ( msStamp / LED_BLINK_SLOW_RATE ) != board_g[rowIdx][colIdx].prevChange ) {
              pixels.SetPixelColor( board_g[rowIdx][colIdx].ledIdx[ledIndex], 0 );
              ledIndex++;
              if( ledIndex >= LEDS_PER_SQUARE )
                ledIndex = 0;
              board_g[rowIdx][colIdx].ledInfo = ( board_g[rowIdx][colIdx].ledInfo & ~LED_INDEX_BITS ) | ( ledIndex << LED_INDEX_BASE );
              pixels.SetPixelColor( board_g[rowIdx][colIdx].ledIdx[ledIndex], RgbColor( (uint8_t)( ledColor >> 16 ), (uint8_t)( ledColor >> 8 ), (uint8_t)ledColor ) );
              board_g[rowIdx][colIdx].prevChange = msStamp / LED_BLINK_SLOW_RATE;
            }
            break;
          case LED_STATE_ROTATE_FAST:
            if( ( msStamp / LED_BLINK_FAST_RATE ) != board_g[rowIdx][colIdx].prevChange ) {
              pixels.SetPixelColor( board_g[rowIdx][colIdx].ledIdx[ledIndex], 0 );
              ledIndex++;
              if( ledIndex >= LEDS_PER_SQUARE )
                ledIndex = 0;
              board_g[rowIdx][colIdx].ledInfo = ( board_g[rowIdx][colIdx].ledInfo & ~LED_INDEX_BITS ) | ( ledIndex << LED_INDEX_BASE );
              pixels.SetPixelColor( board_g[rowIdx][colIdx].ledIdx[ledIndex], RgbColor( (uint8_t)( ledColor >> 16 ), (uint8_t)( ledColor >> 8 ), (uint8_t)ledColor ) );
              board_g[rowIdx][colIdx].prevChange = msStamp / LED_BLINK_FAST_RATE;
            }
            break;
#endif
          default:
            break;
        }
      }
    }
    
    if( pixels.CanShow() ) {
      pixels.Show();   // Send the updated pixel colors to the hardware.
    } else {
    }
    
    nextUpdateStamp = msStamp + LED_UPDATE_INTERVAL_MS;
  }
}

//////////////////////////////////////////////////////////////////////
// SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP
//////////////////////////////////////////////////////////////////////
void setup() {

	int       delayCount = 0;
	uint32_t  deviceId;
	byte      mac[6];

	delay(1000);

	Wire.begin(1, 3);   // For ESP-01 test board
	//  Wire.begin();       // For Wemos D1 mini

	pixels.Begin();
	pixels.ClearTo(RgbColor(0, 0, 0));
	pixels.SetPixelColor( 0, RgbColor(255, 0, 0));
	pixels.Show();   // Send the updated pixel colors to the hardware.

	mcp.init();
	mcp.portMode( MCP23017Port::A, 0 );                                   //Port A as output
	mcp.portMode( MCP23017Port::B, 0b11111111, 0b11111111, 0b11111111 );  //Port B as input, internal pull-ups enabled, bits inverted

	pixels.SetPixelColor( 0, RgbColor(0, 255, 0));
	pixels.Show();   // Send the updated pixel colors to the hardware.

	deviceId = ESP.getChipId();

	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		if ( delayCount % 2 )
			pixels.SetPixelColor( 0, RgbColor(0, 0, 255));
		else
			pixels.SetPixelColor( 0, RgbColor(0, 255, 0));
		pixels.Show();   // Send the updated pixel colors to the hardware.
		delayCount++;
		delay(500);
	}

	snprintf( hostName, 30, "ChessBoardFUE_%lX", deviceId );

	// Send identification to server...
	sendBoardMessage( hostName, strlen( hostName ) );

	pixels.SetPixelColor( 0, RgbColor(0, 0, 255));
	pixels.Show();   // Send the updated pixel colors to the hardware.

#ifdef USE_OTA
	// ---- Start of OTA chunk ----
	// Port defaults to 8266
	// ArduinoOTA.setPort(8266);

	// Hostname defaults to esp8266-[ChipID]
	ArduinoOTA.setHostname( hostName );


	// Password can be set as "clear" or with it's md5 value
	// ArduinoOTA.setPassword("chess");
	// MD5(chess) = b016f48d898c745be5ef382254224582
	ArduinoOTA.setPasswordHash("b016f48d898c745be5ef382254224582");

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		} else { // U_SPIFFS
			type = "filesystem";
		}

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
	});
	ArduinoOTA.onEnd([]() {
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
	});
	ArduinoOTA.onError([](ota_error_t error) {
		if (error == OTA_AUTH_ERROR) {
		} else if (error == OTA_BEGIN_ERROR) {
		} else if (error == OTA_CONNECT_ERROR) {
		} else if (error == OTA_RECEIVE_ERROR) {
		} else if (error == OTA_END_ERROR) {
		}
	});
	ArduinoOTA.begin();
	// ---- End of OTA chunk ----
#endif  // USE_OTA

}

//////////////////////////////////////////////////////////////////////
// LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP
//////////////////////////////////////////////////////////////////////
#define INPUT_MESSAGE_MAX_LEN 100
void loop() {

#ifdef USE_OTA
	ArduinoOTA.handle();
#endif  // USE_OTA

	static char receivedMessage[INPUT_MESSAGE_MAX_LEN];
	static int  receivedMessageLen = 0;
	char        c;

	while ( chessClient.available() ) {
		c = chessClient.read();
		receivedMessage[receivedMessageLen++] = c;
		if ( receivedMessageLen >= INPUT_MESSAGE_MAX_LEN )
			receivedMessageLen = 0;
		if ( ( c == '\n' ) || ( c == ' ' ) ) {
			handleBoardMessage( receivedMessage, receivedMessageLen );
			receivedMessageLen = 0;
		}
	}

	checkRank();  // Call on every loop, will go one rank per call, not the whole table
	updateTableLeds();
}
