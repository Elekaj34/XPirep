// Downloaded from https://developer.x-plane.com/code-sample/hello-world-sdk-3/

#include "XPLMDataAccess.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMNavigation.h"
#include "XPLMProcessing.h"
#include "XPLMPlugin.h"			// Only for plugins reload

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#if IBM
	#include <windows.h>
#endif
#if LIN
	#include <GL/gl.h>
#elif __GNUC__
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

#ifndef XPLM300
	#error This is made to be compiled against the XPLM300 SDK
#endif

#define	XPIREP_VERSION	"1.0.2"
#define	XPIREP_AUTHOR	"Elekaj34"


int 				pluginMenuItem;		// The index of our menu item in the Plugins menu
XPLMMenuID			pluginMenu;			// The menu container we'll append all our menu items to
static XPLMWindowID	pluginWindow;		// An opaque handle to the window we will create

time_t	timeNow;
int		fuelAct,brnState,start,stop;
float	dmeDist,gndSpd,altAGL;
char	strBlockOut[20],strTakeOff[20],strLand[20],strBlockIn[20],strNow[20];
char	strBlockTime[10],strFlightTime[10];

struct FlightState
{
    int state[4] = {0, 0, 0, 0};

    time_t timeBlockOut = 0;
    time_t timeBlockIn  = 0;
    time_t timeTakeOff  = 0;
    time_t timeLand     = 0;

    float fuelBlockOut = 0.0f;
    float fuelTakeOff  = 0.0f;
    float fuelLand     = 0.0f;
    float fuelBlockIn  = 0.0f;

    float fuelBlock = 0.0f;
    float fuelFlight = 0.0f;
    float distance = 0.0f;
    
    double timeFlight = 0.0;
    double timeBlock = 0.0;
    double timeAct = 0.0;
};

FlightState flight;
// Buttons variables
static float pluginResetBtn_lbrt[4]; 	// left, bottom, right, top
static float pluginStartBtn_lbrt[4]; 	// left, bottom, right, top
static float pluginStopBtn_lbrt[4]; 	// left, bottom, right, top

// NAV data variables
char			navDataID[80];

// Callbacks we will register when we create our window
void			drawWindow(XPLMWindowID in_window_id, void * in_refcon);
void 			menuHandler(void *, void *);
int				mouseHandler(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon);
static float	flightLoop(float inElapsedSinceLastCall,float inElapsedTimeSinceLastFlightLoop,int inCounter,void * inRefcon);    

static int		coord_in_rect(float x, float y, float * bounds_lbrt)  { return ((x >= bounds_lbrt[0]) && (x < bounds_lbrt[2]) && (y < bounds_lbrt[3]) && (y >= bounds_lbrt[1])); }

PLUGIN_API int XPluginStart(char * outName,char * outSig,char * outDesc)
{
	strcpy(outName, "XPirepPlugin");
	strcpy(outSig, "xpsdk.examples.xpirepplugin");
	strcpy(outDesc, "An automated pilot report plugin.");
	
	// Auto start monitoring
	
	start=true;
	// Creating menu instance in plugin menu
	pluginMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "XPirep", 0, 0);
	pluginMenu = XPLMCreateMenu("XPirep", XPLMFindPluginsMenu(), pluginMenuItem, menuHandler, NULL);
	XPLMAppendMenuItem(pluginMenu, "Open XPirep", (void *)"MenuOpen", 1);

	XPLMCreateWindow_t params;
	params.structSize = sizeof(params);
	params.visible = 1;
	params.drawWindowFunc = drawWindow;
	// Note on "dummy" handlers:
	// Even if we don't want to handle these events, we have to register a "do-nothing" callback for them
	params.handleMouseClickFunc = mouseHandler;
	params.handleRightClickFunc = NULL;
	params.handleMouseWheelFunc = NULL;
	params.handleKeyFunc = NULL;
	params.handleCursorFunc = NULL;
	params.refcon = NULL;
	params.layer = xplm_WindowLayerFloatingWindows;
	// Opt-in to styling our window like an X-Plane 11 native window
	// If you're on XPLM300, not XPLM301, swap this enum for the literal value 1.
	params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;
	
	// Set the window's initial bounds
	// Note that we're not guaranteed that the main monitor's lower left is at (0, 0)...
	// We'll need to query for the global desktop bounds!
	int left, bottom, right, top;
	XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
	params.left = left + 50;
	params.bottom = bottom + 150;
	params.right = params.left + 300;
	params.top = params.bottom + 200;
	
	pluginWindow = XPLMCreateWindowEx(&params);
	
	// Position the window as a "free" floating window, which the user can drag around
	XPLMSetWindowPositioningMode(pluginWindow, xplm_WindowPositionFree, -1);
	XPLMSetWindowResizingLimits(pluginWindow, 300, 200, 300, 200);
	XPLMSetWindowTitle(pluginWindow, "XPirep");

	// Register call to flight loop each 0.1 sec
	XPLMRegisterFlightLoopCallback(flightLoop,0.1,NULL);
	
	return pluginWindow != NULL;
}

PLUGIN_API void	XPluginStop(void)
{
	// Since we created the window and menu, we'll be good citizens and clean it up
	XPLMDestroyWindow(pluginWindow);
	XPLMDestroyMenu(pluginMenu);
	pluginWindow = NULL;
}

PLUGIN_API void XPluginDisable(void) { }
PLUGIN_API int  XPluginEnable(void)  { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFrom, int inMsg, void * inParam) { }

static float flightLoop(float inElapsedSinceLastCall,float inElapsedTimeSinceLastFlightLoop,int inCounter,void * inRefcon)
{
	int				sizeBrn,onGnd,brnFuel[8],navDataIdx;
	struct			tm * timeInfos;
	
	// Reading XPLANE datarefs parameters
	fuelAct = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/weight/m_fuel_total"));
	sizeBrn = XPLMGetDatavi(XPLMFindDataRef("sim/flightmodel2/engines/engine_is_burning_fuel"),brnFuel,0,8);
	brnState = brnFuel[0]||brnFuel[1]||brnFuel[2]||brnFuel[3]||brnFuel[4]||brnFuel[5]||brnFuel[6]||brnFuel[7];
	onGnd = XPLMGetDatai(XPLMFindDataRef("sim/flightmodel/failures/onground_all"));
	gndSpd = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/groundspeed"));
	altAGL = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/y_agl"));
	dmeDist = XPLMGetDataf(XPLMFindDataRef("sim/cockpit2/radios/indicators/hsi_dme_distance_nm_pilot"));
	flight.distance = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/controls/dist"));
	
	// If on ground and any of engines is running and ground speed > 0.3 m/s then
	// this is the block off event and so set flag and time.
	if(start && onGnd && !flight.state[0] && brnState && gndSpd>0.5) {
		time(&flight.timeBlockOut);
		flight.fuelBlockOut=fuelAct;
		// Convert timestamp to string date
		timeInfos=gmtime(&flight.timeBlockOut);
		strftime(strBlockOut,sizeof(strBlockOut),"%H:%M",timeInfos);
		flight.state[0]=true;
	}
	// If altitude AGL > 25 meters and takeoff flag not set then 
	// this is the takeoff event and so set flag and time.
	if(flight.state[0] && !flight.state[1] && altAGL>25) {
		time(&flight.timeTakeOff);
		flight.fuelTakeOff=fuelAct;
		// Convert timestamp to string date
		timeInfos=gmtime(&flight.timeTakeOff);
		strftime(strTakeOff,sizeof(strTakeOff),"%H:%M",timeInfos);
		flight.state[1]=true;
	}
	// If on ground and takeoof flag set and speed < 25kts then
	// this is the landing event and so set flag and time 
	if(flight.state[1] &&  !flight.state[2] && onGnd) {
		time(&flight.timeLand);
		flight.fuelLand=fuelAct;
		// Convert timestamp to string date
		timeInfos=gmtime(&flight.timeLand);
		strftime(strLand,sizeof(strLand),"%H:%M",timeInfos);
		flight.state[2]=true;
	}
	// If on ground and blockout flag set and speed < 0.1 m/s then
	// this is the block in event and so set flag and time.
	if(stop || (flight.state[2] && !flight.state[3] && gndSpd<0.5 && !brnState)) {
		time(&flight.timeBlockIn);
		flight.fuelBlockIn=fuelAct;
		// Convert timestamp to string date
		timeInfos=gmtime(&flight.timeBlockIn);
		strftime(strBlockIn,sizeof(strBlockIn),"%H:%M",timeInfos);
		// Calculating fuel consumption 
		flight.fuelBlock=flight.fuelBlockOut-flight.fuelBlockIn;
		flight.fuelFlight=flight.fuelTakeOff-flight.fuelLand;
		// Calculating flight time
		flight.timeBlock=difftime(flight.timeBlockIn,flight.timeBlockOut);
		flight.timeFlight=difftime(flight.timeLand,flight.timeTakeOff);
		// Set BlockIn flag
		flight.state[3]=true;
		
		// If stop flag is set then  reset it
		if(stop) stop=false;
	}
	return 0.1;		// Recall this function in 0.1 sec
}

void resetFlightState()
{
    flight = FlightState();
}

void	drawWindow(XPLMWindowID in_window_id, void * in_refcon)
{
	char	str[128];
	struct	tm * timeInfos;
	float 	col_white[] = {1.0, 1.0, 1.0};		// red, green, blue
	float 	col_red[] = {1.0, 0.0, 0.0};		// red, green, blue
	float 	col_green[] = {0.0, 1.0, 0.0};		// red, green, blue

	// Mandatory: We *must* set the OpenGL state before drawing
	XPLMSetGraphicsState(
						 0 /* no fog */,
						 0 /* 0 texture units */,
						 0 /* no lighting */,
						 0 /* no alpha testing */,
						 1 /* do alpha blend */,
						 1 /* do depth testing */,
						 0 /* no depth writing */
						 );
	
	int l, t, r, b;
	XPLMGetWindowGeometry(in_window_id, &l, &t, &r, &b);

	/***************************************************************************
	 *                             DRAWING BUTTONS                             *
	 **************************************************************************/
	// Define buttons positions
	pluginResetBtn_lbrt[0] = l + 11;	// Border left
	pluginResetBtn_lbrt[3] = t - 170;	// Border top
	pluginResetBtn_lbrt[2] = l + 73;	// Border right
	pluginResetBtn_lbrt[1] = t - 190;	// Border bottom

	pluginStartBtn_lbrt[0] = l + 83;	// Border left
	pluginStartBtn_lbrt[3] = t - 170;	// Border top
	pluginStartBtn_lbrt[2] = l + 145;	// Border right
	pluginStartBtn_lbrt[1] = t - 190;	// Border bottom

	pluginStopBtn_lbrt[0] = l + 155;	// Border left
	pluginStopBtn_lbrt[3] = t - 170;	// Border top
	pluginStopBtn_lbrt[2] = l + 217;	// Border right
	pluginStopBtn_lbrt[1] = t - 190;	// Border bottom

	// Draw the boxes around our rudimentary button
	glColor4f(0.5, 0.0, 0.0, 1.0);
	glBegin(GL_QUADS);
	{
		glVertex2i(pluginResetBtn_lbrt[0], pluginResetBtn_lbrt[3]);
		glVertex2i(pluginResetBtn_lbrt[2], pluginResetBtn_lbrt[3]);
		glVertex2i(pluginResetBtn_lbrt[2], pluginResetBtn_lbrt[1]);
		glVertex2i(pluginResetBtn_lbrt[0], pluginResetBtn_lbrt[1]);
	}
	glEnd();
	sprintf(str, "RESET");
	XPLMDrawString(col_red, l + 27, t-183, str, NULL, xplmFont_Basic);

	// Draw the boxes around our rudimentary button
	glColor4f(0.0, 0.25+start*0.5, 0.0, 1.0);
	glBegin(GL_QUADS);
	{
		glVertex2i(pluginStartBtn_lbrt[0], pluginStartBtn_lbrt[3]);
		glVertex2i(pluginStartBtn_lbrt[2], pluginStartBtn_lbrt[3]);
		glVertex2i(pluginStartBtn_lbrt[2], pluginStartBtn_lbrt[1]);
		glVertex2i(pluginStartBtn_lbrt[0], pluginStartBtn_lbrt[1]);
	}
	glEnd();
	sprintf(str, "START");
	XPLMDrawString(col_green, l + 99, t-183, str, NULL, xplmFont_Basic);

	// Draw the boxes around our rudimentary button
	glColor4f(0.0, 0.25+stop*0.5, 0.0, 1.0);
	glBegin(GL_QUADS);
	{
		glVertex2i(pluginStopBtn_lbrt[0], pluginStopBtn_lbrt[3]);
		glVertex2i(pluginStopBtn_lbrt[2], pluginStopBtn_lbrt[3]);
		glVertex2i(pluginStopBtn_lbrt[2], pluginStopBtn_lbrt[1]);
		glVertex2i(pluginStopBtn_lbrt[0], pluginStopBtn_lbrt[1]);
	}
	glEnd();
	sprintf(str, "STOP");
	XPLMDrawString(col_red, l + 174, t-183, str, NULL, xplmFont_Basic);
			
	/***************************************************************************
	 *                            DRAWING PARAMETERS                           *
	 **************************************************************************/
	timeNow=time(NULL);
	timeInfos=gmtime(&timeNow);
	strftime(strNow,sizeof(strNow),"%F %T",timeInfos);
	sprintf(str, "Actual time : %s", strNow);
	XPLMDrawString(col_white, l + 10, t, str, NULL, xplmFont_Proportional);
	sprintf(str,"Distance : %.0f nm - Speed : %.0f kts",round(flight.distance/1852.0),(gndSpd*1.94384));
	XPLMDrawString(col_white, l + 10, t - 35, str, NULL, xplmFont_Basic);
	sprintf(str,"Version : %s by %s",XPIREP_VERSION,XPIREP_AUTHOR);
	XPLMDrawString(col_white, l + 10, t - 205, str, NULL, xplmFont_Basic);

	// Display live flight infos only if in the air.
	if(flight.state[1] && !flight.state[2]) {
		flight.timeAct=difftime(timeNow,flight.timeTakeOff);
		sprintf(str,"Flight time %02d:%02d:%02d - Fuel burned : %.0f kg",
			(int)(flight.timeAct)/3600,((int)(flight.timeAct) % 3600)/60,(int)(flight.timeAct) % 60,
			flight.fuelBlockOut-fuelAct);
		XPLMDrawString(col_white, l + 10, t - 20, str, NULL, xplmFont_Basic);
	}
	// Display state data info
	{
		sprintf(str,"state : %d %d %d %d - %d",flight.state[0],flight.state[1],flight.state[2],flight.state[3],brnState);
	}

	// Display of block and flight info
	if(flight.state[0]) {
		sprintf(str,"Block Out : %s - %.0f kg",strBlockOut,flight.fuelBlockOut);
		XPLMDrawString(col_white, l + 10, t - 60, str, NULL, xplmFont_Basic);
	}
	if(flight.state[1]) {
		sprintf(str,"Take Off  : %s - %.0f kg",strTakeOff,flight.fuelTakeOff);
		XPLMDrawString(col_white, l + 10, t - 75, str, NULL, xplmFont_Basic);
	}
	if(flight.state[2]) {
		sprintf(str,"Landing . : %s - %.0f kg",strLand,flight.fuelLand);
		XPLMDrawString(col_white, l + 10, t - 90, str, NULL, xplmFont_Basic);
	}
	if(flight.state[3]) {
		sprintf(str,"Block In  : %s - %.0f kg",strBlockIn,flight.fuelBlockIn);
		XPLMDrawString(col_white, l + 10, t - 105, str, NULL, xplmFont_Basic);

		sprintf(str,"Block Fuel : %6.0f kg  Flight Fuel : %6.0f kg",flight.fuelBlock,flight.fuelFlight);
		XPLMDrawString(col_white, l + 10, t - 130, str, NULL, xplmFont_Basic);
		sprintf(str,"Block Time : %02d:%02d:%02d   Flight Time : %02d:%02d:%02d",
			(int)(flight.timeBlock)/3600,((int)(flight.timeBlock) % 3600)/60,(int)(flight.timeBlock) % 60,
			(int)(flight.timeFlight)/3600,((int)(flight.timeFlight) % 3600)/60,(int)(flight.timeFlight) % 60);
		sprintf(str,"Block Time : %02d:%02d      Flight Time : %02d:%02d",
			(int)(flight.timeBlock / 3600),((int)flight.timeBlock % 3600)/60,
			(int)(flight.timeFlight / 3600),((int)flight.timeFlight % 3600)/60);
		XPLMDrawString(col_white, l + 10, t - 145, str, NULL, xplmFont_Basic);
		
		// Auto display windows at block on (end of flight)
		XPLMSetWindowIsVisible(pluginWindow,true);
		// Auto start back to off at end of flight
		start=false;
	}
}

void menuHandler(void * in_menu_ref, void * in_item_ref)
{
	// Show window when choosen in the plugin menu
	if(!strcmp((const char *)in_item_ref, "MenuOpen"))
	{
		XPLMSetWindowIsVisible(pluginWindow,true);
	}
}

int	mouseHandler(XPLMWindowID in_window_id, int x, int y, XPLMMouseStatus is_down, void * in_refcon)
{
	if(is_down == xplm_MouseDown)
	{
		if(coord_in_rect(x, y, pluginResetBtn_lbrt)) // user clicked the reset button
		{
			resetFlightState();
			XPLMSetDataf(XPLMFindDataRef("sim/flightmodel/controls/dist"),0);
			start=0;
		}
		if(coord_in_rect(x, y, pluginStartBtn_lbrt)) // user clicked the start button
		{
			if(start) {start=0;}		// Invert start state
			else {start=1;}
		}
		if(coord_in_rect(x, y, pluginStopBtn_lbrt)) // user clicked the stop button
		{
			if(stop) {stop=0;}		// Invert start state
			else {stop=1;}
		}
	}
	return 1;
}
