#include "script.h"
#include <random>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>

#pragma warning(disable : 4244 4305) // double <-> float conversions

// controller support

// MAIN VARIABLES
Player player;
//std::string Text; // UNUSED TEXT VARIABLE
Ped playerPed;
float x, y, z, sx, sy, sz;
Any mainBlip[2], recrBlip[2];

// BOOLEAN VARIABLES
// SOME VARIABLES ARE UNUSED OR UNEEDED
// TODO :: CLEAN SOME OF THEM.
// USED TO INITIALIZE FUNCTION
bool debugV = true;
// CHECKS IF PLAYER IS IN AIR ... UNUSED.
bool isInAir = false;
bool menuDisplayed[5];
// WETHER PLAYER IS AT RED OR BLUE HOUSE
bool isAtRedHouse1 = false;
bool isAtRedHouse2 = false;
bool joinedClean = false;
bool joinedProtect = false;
bool finishedProtect = false;
bool isDrivingAway = false;
bool isInside = false;
bool handlerInit = false;
bool hardcore = false;
bool covertOps = false;
bool newMenu = false;
bool blipSet = false;
bool mission = false;
bool sentMen = false;
bool wasKilled[32];
// TEMP VARIABLE
bool tmp = false;
bool canCancel = false;

// INTEGERS VARIABLES
// NEEDS SOME CLEANING HERE TOO
int oldRand; // UNUSED ?
int tick;
int randtick; // UNUSED ?
// USED IN VIP MISSIONS TO DETERMINE HOW MANY PEOPLE ARE WE GONNA SPAWN IN THE CAR. PROSTEP IS EQUAL TO 2 IF THE CAR HAS 2 SEATS, 4 IF THE CAR HAS 4.
int proStep; 
// BODYGUARD COUNT, USED IN bodyguard_leave() TO COUNT HOW MANY BODYGUARDS PLAYER FIRED.
int bodyguard = 0;
// USED IN MENUS, MAKES RANDOM YELLOW TEXT APPEAR.
int random_text = 0;
// STRING VARIABLES THAT CONTAINS THE NAME & CASH OF THE CONTRACT
std::string contracts[3];
// ID OF THE CONTRACT IS STORED INSIDE THIS VARIABLE. EXEMPLE : 1 IS ASSASSINATION, 3 IS VIP PROTECTION.
int randContract[3];
// PAYOUT OF THE CONTRACT, USED IN SETUP PAYOUT FUNCTION
int payout[3];
// MISSION COMPLETED COUNTER, USED TO NOW IF PLAYER HAS PLAYED ENOUGH TO UNLOCK HEIST CONTRACT
int totalMission = 0;
// USED IN COVERT MISSIONS TO DETERMINE AT WHAT HOUR THE QUICK CHANGING TIME SHOULD STOP.
// EXEMPLE : IF HOURS = 23 THEN IT WILL STOP AT 23H00.
int hours = 0;
// SELECT TYPE USED IN PREVIOUS CONTRACTS
// TODO :: FIND A BETTER WAY ?
int selectedType, contractPayout[3];
int random;
// CALCULATE THE AMOUNT OF MOBSTERS IN VIP MISSIONS WHO DIED.
int mobDead = 0;
// WORKAROUND FOR THE MENU...
int fiveTick = 0;
// USED IN ASSASSINATION MISSIONS SO WE WON'T SPAM THE PLAYER WITH HIRED PROTECTION
int callBack = 0;

// UI VALUES
// ID OF THE MENU SELECTED
int uiSelected = 0;
// UNUSED VARIABLE TO ADD A SWITCHING BLUE EFFECT IN THE MENU...
int blue = 0;

// VECTOR3 POSITIONS FOR ASSASSINATIONS MISSIONS
Vector3 assMission[10];
// PAYOUT VEHICLE HANDLE
Any payVeh;
// CONTAINS THE 5 RANDOM VEHICLES FOR ASSASSINATION MISSIONS
char * cars[5];

// POSITIONS USED FOR RANDOMIZATION
Vector3 mobClean;
Vector3 mobProtect;

// BODYGUARD HANDS AND BORROWED VEHICLE
Any bodyG[5];
Any borVeh;
// UNUSED.
char * gxt;
// USED IN VIP PROTECTION MISSIONS IN protect_loop() FUNCTION
DWORD pedHi, ennHi, vehHi;



// TICKS 
int press_B_tick = 0;/*
					 int press_E_tick = 0;
					 int press_L_tick = 0;*/

// use them wisely
Any scVeh[64];
Any scPed[64];
Any blips[64];

Vector3 spawnPt, spawnPt1, spawnPt2, position, sposition;

// NEEDED, COULD DO BETTER, BUT HEY, IT'S A MOD, WHO GIVES A SHIT.
void setTextMsg(int type, char *text);
void update();
void recruit_checks();
void pimpMyRide(int type, int id);
void heist_1();
void menuHandler();
void cleanupMissions(int type);

void debug_func()
{
	if (CAM::IS_SCREEN_FADING_IN()) {
		debugV = true;
		UI::REMOVE_BLIP(&mainBlip[0]);
		UI::REMOVE_BLIP(&recrBlip[0]);
		UI::REMOVE_BLIP(&mainBlip[1]);
		UI::REMOVE_BLIP(&recrBlip[1]);
	}

	if (debugV) {
		// DISPLAY FOR 3SEC
		debugV = false;
		handlerInit = false;
		setTextMsg(2, "The ~r~Red ~w~& ~b~Blue ~w~House~w~ v3.3");
		// blip x : 125.6
		// blip y : -1044.59
		// blip z : 29.24
		mainBlip[0] = UI::ADD_BLIP_FOR_COORD(125.6, -1044.59, 29.24);
		mainBlip[1] = UI::ADD_BLIP_FOR_COORD(51, 2785, 57);
		recrBlip[0] = UI::ADD_BLIP_FOR_COORD(147.014, -1059.07, 29.19);
		UI::SET_BLIP_SPRITE(mainBlip[0], 40);
		UI::SET_BLIP_SPRITE(mainBlip[1], 40);
		UI::SET_BLIP_COLOUR(mainBlip[0], 1);
		UI::SET_BLIP_COLOUR(mainBlip[1], 3);
		UI::SET_BLIP_AS_SHORT_RANGE(mainBlip[0], true);
		UI::SET_BLIP_AS_SHORT_RANGE(mainBlip[1], true);
		UI::SET_BLIP_SPRITE(recrBlip[0], 358);
		UI::SET_BLIP_AS_SHORT_RANGE(recrBlip[0], true);

		for (int i = 32; i < 32; i++){
			wasKilled[i] = false;
		}
	}

}

// DON'T EVEN KNOW IF IT'S USED ANYMORE...
void basic_check()
{
	if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, false))
	{
		cleanupMissions(1);
		setTextMsg(2000, "~r~Mission failed ~w~:~n~I thought you were a pro !");
		for (int i = 0; i < 5; i++){ ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&bodyG[i]); }
	}
}

// TAKEN FROM MY ARMY RESPONSE MOD, DRAWS 2 LINES AROUND THE PLAYER AND FINDS SPAWN POINTS WITHIN A RADIUS AROUND THE PLAYER.
void set_spawn_points()
{

	float step = 0.0f;
	if (rand() % 2 == 1){ step = -100.0f; }
	else { step = 100.0f; }
	spawnPt1 = sposition;
	spawnPt1.x = sposition.x + step;
	if (rand() % 2 == 1){ step = -100.0f; }
	else { step = 100.0f; }
	spawnPt2 = sposition;
	spawnPt2.y = sposition.y + step;


	if (rand() % 2 == 1){ step = -1.0f; }
	else { step = 1.0f; }

	// set spawn points with a radius of 500
	int maxStep = 100;
	// finding spawnpt1 loop.
	while (!PATHFIND::IS_POINT_ON_ROAD(spawnPt1.x, spawnPt1.y, spawnPt1.z, 0) && maxStep > 0)
	{
		spawnPt1.x += step;
		maxStep--;
	}
	// resetting max step to 75
	maxStep = 100;
	if (rand() % 2 == 1){ step = -1.0f; }
	else { step = 1.0f; }
	// finding spawnpt2 loop.
	while (!PATHFIND::IS_POINT_ON_ROAD(spawnPt2.x, spawnPt2.y, spawnPt2.z, 0) && maxStep > 0)
	{
		spawnPt2.y -= step;
		maxStep--;
	}
	GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(spawnPt2.x, spawnPt2.y, spawnPt2.z + 100.0f, &spawnPt2.z);
	GAMEPLAY::GET_GROUND_Z_FOR_3D_COORD(spawnPt1.x, spawnPt1.y, spawnPt1.z + 100.0f, &spawnPt1.z);

}

// DISPLAYS A NOTIFICATION.
void setTextMsg(int time, char *text)
{
	UI::_0x202709F4C58A0424((DWORD *) "STRING"); // _SET_NOTIFICATION_TEXT_ENTRY
	//UI::_SET_TEXT_ENTRY("STRING");
	// displays name of an input
	//UI::_0x17299B63C7683A2B(GAMEPLAY::GET_HASH_KEY(gxt));
	UI::_0x80EAD8E2E1D5D52E(GAMEPLAY::GET_HASH_KEY(gxt));
	UI::_ADD_TEXT_COMPONENT_STRING(text);
	UI::_0x2ED7843F8F801023(FALSE, FALSE); // _DRAW_NOTIFICATION
	UI::_0x9D77056A530643F6(time, 1);
}

// SETUPS THE PAYOUT SCENE
void setupPayout()
{
	totalMission++;
	DWORD bison = GAMEPLAY::GET_HASH_KEY("bison");
	DWORD mcase = GAMEPLAY::GET_HASH_KEY("prop_security_case_01");
	DWORD pcase = GAMEPLAY::GET_HASH_KEY("pickup_money_case");
	STREAMING::REQUEST_MODEL(mcase);
	STREAMING::REQUEST_MODEL(pcase);
	STREAMING::REQUEST_MODEL(bison);
	while (!STREAMING::HAS_MODEL_LOADED(bison) || !STREAMING::HAS_MODEL_LOADED(mcase))
		WAIT(100);

	payVeh = VEHICLE::CREATE_VEHICLE(bison, 135.3, -1050.86, 29.3, 159.6, 0, true);
	VEHICLE::SET_VEHICLE_DOOR_OPEN(payVeh, 5, false, false);
	//WAIT(2000);
	if (contractPayout[uiSelected] > 44000 && contractPayout[uiSelected] < 176000) {
		contractPayout[uiSelected] /= 4;
		OBJECT::CREATE_PICKUP_ROTATE(pcase, 136.132, -1048.0, 29.16, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase);
		OBJECT::CREATE_PICKUP_ROTATE(pcase, 136.132, -1048.79, 29.16, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase);
		OBJECT::CREATE_PICKUP_ROTATE(pcase, 136.832, -1048.79, 29.16, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase);
		OBJECT::CREATE_PICKUP_ROTATE(pcase, 137.1, -1048.79, 29.16, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase);
	}
	else if (contractPayout[uiSelected] > 176000) {
		contractPayout[uiSelected] /= 8;
		OBJECT::CREATE_PICKUP_ROTATE(pcase, 136.132, -1048.0, 29.16, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase);
		OBJECT::CREATE_PICKUP_ROTATE(pcase, 136.132, -1048.79, 29.16, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase);
		OBJECT::CREATE_PICKUP_ROTATE(pcase, 136.832, -1048.79, 29.16, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase);
		OBJECT::CREATE_PICKUP_ROTATE(pcase, 137.1, -1048.79, 29.16, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase);
		OBJECT::CREATE_PICKUP_ROTATE(pcase, 136.132, -1048.0, 29.26, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase);
		OBJECT::CREATE_PICKUP_ROTATE(pcase, 136.132, -1048.79, 29.26, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase);
		OBJECT::CREATE_PICKUP_ROTATE(pcase, 136.832, -1048.79, 29.26, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase);
		OBJECT::CREATE_PICKUP_ROTATE(pcase, 137.1, -1048.79, 29.26, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase);
	}
	else { OBJECT::CREATE_PICKUP_ROTATE(pcase, 136.132, -1048.79, 29.16, -90.0, 0.0, 90, 0, contractPayout[uiSelected], 2, 1, mcase); }

	//OBJECT::CREATE_PICKUP(pcase, 136.326, -1048.23, 29, 0, contractPayout, true, mcase);
	contractPayout[uiSelected] = 0;
	selectedType = 0;

}

// SETUPS MAIN SCENE, BOOLEAN VALUES SAYS WETHER THE PLAYER HAS JOINED OR LEFT THE ZONE
// IF SPAWN IS TRUE, WE SPAWN
// IF FALSE, WE DESPAWN.
void setupScene_main(bool spawn)
{
	Any health, armor;
	if (spawn) {

		//GRAPHICS::DRAW_MARKER(2, 125.722, -1046.1, 29.22, 0.0f, 0.0f, 0.0f, 180.0f, 0.0f, 0.0f, 0.75f, 0.75f, 0.75f, 204, 204, 1, 100, false, true, 2, false, false, false, false);
		DWORD dubsta = GAMEPLAY::GET_HASH_KEY("dubsta2");
		DWORD gang1 = GAMEPLAY::GET_HASH_KEY("g_m_y_korean_01");
		DWORD zent = GAMEPLAY::GET_HASH_KEY("zentorno");
		DWORD mav = GAMEPLAY::GET_HASH_KEY("maverick");
		STREAMING::REQUEST_MODEL(dubsta);
		STREAMING::REQUEST_MODEL(gang1);
		STREAMING::REQUEST_MODEL(zent);
		STREAMING::REQUEST_MODEL(mav);

		WAIT(100);
		while (!STREAMING::HAS_MODEL_LOADED(dubsta) && !STREAMING::HAS_MODEL_LOADED(gang1) && !STREAMING::HAS_MODEL_LOADED(zent) && !STREAMING::HAS_MODEL_LOADED(mav))
			WAIT(100);
		scVeh[0] = VEHICLE::CREATE_VEHICLE(dubsta, 122.948, -1044.56, 29.3, 330, 0, true);
		scPed[0] = PED::CREATE_PED(26, gang1, 117.914, -1049.67, 29.2, 182.9, false, true);
		scPed[1] = PED::CREATE_PED(26, gang1, 129.219, -1045.49, 33.32, 98.0, false, true);
		scPed[2] = PED::CREATE_PED(26, gang1, 144.805, -1060.32, 29.2, 116, false, true);
		scVeh[1] = VEHICLE::CREATE_VEHICLE(zent, 138.63, -1060.6, 29.3, 319, 0, true);
		scVeh[2] = VEHICLE::CREATE_VEHICLE(mav, 159, -1075.6, 29.2, 85, 0, true);
		pimpMyRide(0, 1);
		VEHICLE::SET_VEHICLE_DOORS_LOCKED_FOR_ALL_PLAYERS(scVeh[1], true);
		VEHICLE::SET_VEHICLE_COLOURS(0, 1, 0);
		health = OBJECT::CREATE_PICKUP_ROTATE(GAMEPLAY::GET_HASH_KEY("pickup_health_standard"), 125.75, -1046.16, 28.431, -90.0, 0.0, 90, 0, 0, 2, 1, 0);
		armor = OBJECT::CREATE_PICKUP_ROTATE(GAMEPLAY::GET_HASH_KEY("pickup_armour_standard"), 124.455, -1040.76, 28.431, -90.0, 0.0, 90, 0, 0, 2, 1, 0);

	}
	else {
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[0]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[1]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[2]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[15]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[0]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[28]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[29]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[30]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&health);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&armor);
		ENTITY::DELETE_ENTITY(&scVeh[0]);
		ENTITY::DELETE_ENTITY(&health);
		ENTITY::DELETE_ENTITY(&armor);
		ENTITY::DELETE_ENTITY(&scPed[0]);
		ENTITY::DELETE_ENTITY(&scVeh[1]);
		ENTITY::DELETE_ENTITY(&scVeh[15]);
	}


}

void setupScene_blue(bool spawn)
{
	//Any health, armor;
	Any objects[6];
	if (spawn) {

		//GRAPHICS::DRAW_MARKER(2, 125.722, -1046.1, 29.22, 0.0f, 0.0f, 0.0f, 180.0f, 0.0f, 0.0f, 0.75f, 0.75f, 0.75f, 204, 204, 1, 100, false, true, 2, false, false, false, false);
		DWORD towtruck = GAMEPLAY::GET_HASH_KEY("towtruck");
		DWORD rustyTt = GAMEPLAY::GET_HASH_KEY("towtruck2");
		DWORD feltzer = GAMEPLAY::GET_HASH_KEY("feltzer2");
		DWORD bobcat = GAMEPLAY::GET_HASH_KEY("bobcatxl");
		DWORD rat = GAMEPLAY::GET_HASH_KEY("ratloader");
		DWORD dukes = GAMEPLAY::GET_HASH_KEY("dukes");
		
		//OBJECTS
		STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("cs6_10_deserthouse_007"));

		STREAMING::REQUEST_MODEL(towtruck);
		STREAMING::REQUEST_MODEL(rustyTt);
		STREAMING::REQUEST_MODEL(feltzer);
		STREAMING::REQUEST_MODEL(bobcat);
		STREAMING::REQUEST_MODEL(rat);
		STREAMING::REQUEST_MODEL(dukes);

		WAIT(1000);

		scVeh[0] = VEHICLE::CREATE_VEHICLE(rustyTt, 58.823, 2768.7, 57.5, 112, 0, true);
		scVeh[1] = VEHICLE::CREATE_VEHICLE(towtruck, 53.675, 2802.78, 57.8, 55.2, 0, true);
		scVeh[2] = VEHICLE::CREATE_VEHICLE(bobcat, 42.019, 2799.99, 57.65, 145.95, 0, true);
		scVeh[3] = VEHICLE::CREATE_VEHICLE(rat, 53.874, 2785.119, 57.067, 219.302, 0, true);
		scVeh[4] = VEHICLE::CREATE_VEHICLE(dukes, 48.548, 2776.576, 57.454, 232.853, 0, true);
		scVeh[5] = VEHICLE::CREATE_VEHICLE(feltzer, 63.15, 2786.2, 57.7, 142, 0, true);
		pimpMyRide(0, 5);
		if(rand() % 2 == 0) pimpMyRide(0, 4);
		VEHICLE::SET_VEHICLE_DOORS_LOCKED_FOR_ALL_PLAYERS(scVeh[1], true);
		VEHICLE::SET_VEHICLE_COLOURS(0, 1, 0);
		//health = OBJECT::CREATE_PICKUP_ROTATE(GAMEPLAY::GET_HASH_KEY("pickup_health_standard"), 125.75, -1046.16, 28.431, -90.0, 0.0, 90, 0, 0, 2, 1, 0);
		//armor = OBJECT::CREATE_PICKUP_ROTATE(GAMEPLAY::GET_HASH_KEY("pickup_armour_standard"), 124.455, -1040.76, 28.431, -90.0, 0.0, 90, 0, 0, 2, 1, 0);

		//objects[0] = OBJECT::CREATE_OBJECT(GAMEPLAY::GET_HASH_KEY("cs6_10_deserthouse_007"), 17.51, 2800, 57, 0, 0, 0);
		
	}
	else {
		for (int i = 0; i < 6; i++)
		{
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[i]);
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i]);
			//ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&objects[i]);
		}
	}
}

// SETUPS CARS 
// FOR ASSASSINATION MISSIONS
void setupCars(int type)
{
	if (type == 1)
	{
		cars[0] = "sentinel";
		cars[1] = "patriot";
		cars[2] = "zion";
		cars[3] = "oracle2";
		cars[4] = "oracle";

	}
}

// SETUPS STATIC SPAWN POINTS FOR SPAWN POINTS
void setupSpawnPts(int type){

	if (type == 1)
	{

		// assassination missions
		assMission[0].x = 692;
		assMission[0].y = 606;
		assMission[0].z = 129;

		assMission[1].x = -1185;
		assMission[1].y = -125;
		assMission[1].z = 41;

		assMission[2].x = -784;
		assMission[2].y = 971;
		assMission[2].z = 238;

		assMission[3].x = -1232;
		assMission[3].y = -1085;
		assMission[3].z = 9;

		assMission[4].x = 344;
		assMission[4].y = -2506;
		assMission[4].z = 6;

		assMission[5].x = -907;
		assMission[5].y = -2673;
		assMission[5].z = 14;

		assMission[6].x = -887;
		assMission[6].y = -1968;
		assMission[6].z = 28;

		assMission[7].x = 1040;
		assMission[7].y = -2348;
		assMission[7].z = 31;

		assMission[8].x = -850;
		assMission[8].y = -652;
		assMission[8].z = 27;

		assMission[9].x = 106;
		assMission[9].y = -1938;
		assMission[9].z = 21;
	}

}

void mobClean1() {

	mobClean.x = -15.85;
	mobClean.y = -1835.15;
	mobClean.z = 25.35;

	blips[1] = UI::ADD_BLIP_FOR_COORD(mobClean.x, mobClean.y, mobClean.z);
	UI::SET_BLIP_COLOUR(blips[1], 3);

	DWORD vehH = GAMEPLAY::GET_HASH_KEY("landstalker");
	DWORD pedH = GAMEPLAY::GET_HASH_KEY("G_M_Y_BallaEast_01");
	DWORD weapH;

	STREAMING::REQUEST_MODEL(vehH);
	STREAMING::REQUEST_MODEL(pedH);

	while (!STREAMING::HAS_MODEL_LOADED(vehH) || !STREAMING::HAS_MODEL_LOADED(pedH))
		WAIT(100);

	// vehicles
	scVeh[6] = VEHICLE::CREATE_VEHICLE(vehH, 12.20, -1822.6, 25.06, 156.0f, 0, true);
	scVeh[7] = VEHICLE::CREATE_VEHICLE(vehH, 8.29, -1836.6, 24.8, 76.0f, 0, true);
	scVeh[8] = VEHICLE::CREATE_VEHICLE(vehH, 6.95, -1809, 25.35, 266.0f, 0, true);

	// peds (veh1)
	scPed[6] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, -1, false, false);
	scPed[7] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, 0, false, false);
	scPed[8] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, 1, false, false);
	scPed[9] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, 2, false, false);

	// peds (veh3)
	scPed[10] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 3, pedH, -1, false, false);
	scPed[11] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 3, pedH, 0, false, false);
	scPed[12] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 3, pedH, 1, false, false);
	scPed[13] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 3, pedH, 2, false, false);

	// peds wandering
	for (int i = 0; i < 7; i++)
	{
		weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL");
		STREAMING::REQUEST_MODEL(weapH);
		scPed[14] = PED::CREATE_PED(26, pedH, 7.70, -1819.8, 25.37, 181, false, true);
		WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[14], weapH, 1000, false);
		PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[14], 0x90c7da60);
	}
	scPed[14] = PED::CREATE_PED(26, pedH, 7.70, -1819.8, 25.37, 181, false, true);
	scPed[15] = PED::CREATE_PED(26, pedH, 6.33, -1821.67, 25.38, 290, false, true);
	scPed[16] = PED::CREATE_PED(26, pedH, 7.41, -1822, 25.25, 0, false, true);

	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, GAMEPLAY::GET_HASH_KEY("player"), 0x90c7da60);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, 0x90c7da60, GAMEPLAY::GET_HASH_KEY("player"));


	for (int i = 6; i < 17; i++)
	{
		if (rand() % 3 == 0) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL");
		if (rand() % 3 == 1) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_PUMPSHOTGUN");
		if (rand() % 3 == 2) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_MICROSMG");
		STREAMING::REQUEST_MODEL(weapH);
		WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i], weapH, 1000, false);
		WEAPON::SET_CURRENT_PED_WEAPON(scPed[i], weapH, true);
		PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i], 0x90c7da60);

	}




}

// CLEANS THE BLIPS ON THE MAP.
void mobCleanBlips(bool first)
{
	if (first) {
		for (int i = 6; i < 17; i++)
		{
			blips[i] = UI::ADD_BLIP_FOR_ENTITY(scPed[i]);
		}
	}
	else {
		for (int i = 6; i < 17; i++)
		{
			if (ENTITY::IS_ENTITY_DEAD(scPed[i])) UI::REMOVE_BLIP(&blips[i]);
		}
	}
}

void mobClean2() {
	mobClean.x = -1130;
	mobClean.y = -1586;
	mobClean.z = 4.4;

	blips[1] = UI::ADD_BLIP_FOR_COORD(mobClean.x, mobClean.y, mobClean.z);
	UI::SET_BLIP_COLOUR(blips[1], 3);

	DWORD vehH = GAMEPLAY::GET_HASH_KEY("burrito3");
	DWORD pedH = GAMEPLAY::GET_HASH_KEY("G_M_Y_MexGoon_01");
	DWORD weapH;

	STREAMING::REQUEST_MODEL(vehH);
	STREAMING::REQUEST_MODEL(pedH);
	while (!STREAMING::HAS_MODEL_LOADED(vehH) || !STREAMING::HAS_MODEL_LOADED(pedH))
		WAIT(100);

	// vehicles
	scVeh[6] = VEHICLE::CREATE_VEHICLE(vehH, -1129, -1608, 4.39, 270, 0, true);
	scVeh[7] = VEHICLE::CREATE_VEHICLE(vehH, -1114, -1621, 4.5, 304, 0, true);
	scVeh[8] = VEHICLE::CREATE_VEHICLE(vehH, -1111, -1608, 4.54, 32, 0, true);

	// peds (veh1)
	scPed[6] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, -1, false, false);
	scPed[7] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, 0, false, false);
	scPed[8] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, 1, false, false);
	scPed[9] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, 2, false, false);

	// peds (veh3)
	scPed[10] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 3, pedH, -1, false, false);
	scPed[11] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 3, pedH, 0, false, false);
	scPed[12] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 3, pedH, 1, false, false);
	scPed[13] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 3, pedH, 2, false, false);

	for (int i = 0; i < 7; i++)
	{
		weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL");
		STREAMING::REQUEST_MODEL(weapH);
		scPed[14] = PED::CREATE_PED(26, pedH, -1112, -1623, 4.45, 16, false, true);
		WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[14], weapH, 1000, false);
		PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[14], 0x90c7da60);
	}

	// peds wandering
	scPed[14] = PED::CREATE_PED(26, pedH, -1112, -1623, 4.45, 16, false, true);
	scPed[15] = PED::CREATE_PED(26, pedH, -1114, -1623, 4.45, 2, false, true);
	scPed[16] = PED::CREATE_PED(26, pedH, -1119.6, -1616.2, 9.64, 358, false, true);

	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, GAMEPLAY::GET_HASH_KEY("player"), 0x90c7da60);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, 0x90c7da60, GAMEPLAY::GET_HASH_KEY("player"));


	for (int i = 6; i < 17; i++)
	{
		if (rand() % 3 == 0) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL");
		if (rand() % 3 == 1) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_ASSAULTRIFLE");
		if (rand() % 3 == 2) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_MICROSMG");
		STREAMING::REQUEST_MODEL(weapH);
		WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i], weapH, 1000, false);
		WEAPON::SET_CURRENT_PED_WEAPON(scPed[i], weapH, true);
		PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i], 0x90c7da60);
	}

}

void mobClean3() {
	mobClean.x = 1352.5;
	mobClean.y = -572;
	mobClean.z = 74;

	blips[1] = UI::ADD_BLIP_FOR_COORD(mobClean.x, mobClean.y, mobClean.z);
	UI::SET_BLIP_COLOUR(blips[1], 3);

	DWORD vehH = GAMEPLAY::GET_HASH_KEY("comet2");
	DWORD pedH = GAMEPLAY::GET_HASH_KEY("G_M_M_ChiGoon_01");
	DWORD weapH;

	STREAMING::REQUEST_MODEL(vehH);
	STREAMING::REQUEST_MODEL(pedH);

	while (!STREAMING::HAS_MODEL_LOADED(vehH) || !STREAMING::HAS_MODEL_LOADED(pedH))
		WAIT(100);

	// vehicles
	scVeh[6] = VEHICLE::CREATE_VEHICLE(vehH, 1363, -554, 75, 156.0f, 0, true);
	scVeh[7] = VEHICLE::CREATE_VEHICLE(vehH, 1387, -577, 75, 288.0f, 0, true);
	scVeh[8] = VEHICLE::CREATE_VEHICLE(vehH, 1378, -597, 75, 49, 0, true);

	// peds (veh1)
	scPed[6] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, -1, false, false);
	scPed[7] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, 0, false, false);
	scPed[8] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, 1, false, false);
	scPed[9] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, 2, false, false);

	// peds (veh3)
	scPed[10] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 3, pedH, -1, false, false);
	scPed[11] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 3, pedH, 0, false, false);
	scPed[12] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 3, pedH, 1, false, false);
	scPed[13] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 3, pedH, 2, false, false);

	for (int i = 0; i < 7; i++)
	{
		weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL");
		STREAMING::REQUEST_MODEL(weapH);
		scPed[14] = PED::CREATE_PED(26, pedH, 1359, -599, 75, 354, false, true);
		WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[14], weapH, 1000, false);
		PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[14], 0x90c7da60);
	}

	// peds wandering
	scPed[14] = PED::CREATE_PED(26, pedH, 1359, -599, 75, 354, false, true);
	scPed[15] = PED::CREATE_PED(26, pedH, 1361, -599, 75, 354, false, true);
	scPed[16] = PED::CREATE_PED(26, pedH, 1363, -599, 75, 354, false, true);

	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, GAMEPLAY::GET_HASH_KEY("player"), 0x90c7da60);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, 0x90c7da60, GAMEPLAY::GET_HASH_KEY("player"));


	for (int i = 6; i < 17; i++)
	{
		if (rand() % 3 == 0) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_ASSAULTRIFLE");
		if (rand() % 3 == 1) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_MG");
		if (rand() % 3 == 2) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_MICROSMG");
		STREAMING::REQUEST_MODEL(weapH);
		WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i], weapH, 1000, false);
		WEAPON::SET_CURRENT_PED_WEAPON(scPed[i], weapH, true);
		PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i], 0x90c7da60);
	}
}

// DISPLAYS THE CANCEL MESSAGE
void cancelMsg()
{
	setTextMsg(1000, "~b~Info~w~:~n~Press Right + ~y~Y~w~ or [O] at any moment to cancel the contract.");
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
}

// PROTECTION LOOP.
void protectLoop()
{
	STREAMING::REQUEST_MODEL(ennHi);
	while (!STREAMING::HAS_MODEL_LOADED(ennHi))
		WAIT(100);
	DWORD weapH;
	tick++;


	if (!finishedProtect) {
		for (int i = 7; i < 24; i += proStep)
		{
			set_spawn_points();
			if ((!ENTITY::DOES_ENTITY_EXIST(scPed[i]) || ENTITY::IS_ENTITY_DEAD(scPed[i])) && tick > 120) {
				if (!ENTITY::IS_ENTITY_AT_COORD(playerPed, spawnPt1.x, spawnPt1.y, spawnPt1.z, 35, 35, 35, 0, 1, 0) && !ENTITY::IS_ENTITY_AT_COORD(playerPed, spawnPt2.x, spawnPt2.y, spawnPt2.z, 35, 35, 35, 0, 1, 0)) {
					tick = 0;
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i]);
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[i]);
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i + 2]);
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i + 3]);
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i + 1]);
					//ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[7]);
					//ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[8]);
					weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_MICROSMG");
					if (rand() % 3 == 0) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_ASSAULTRIFLE");
					if (rand() % 3 == 1) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_PUMPSHOTGUN");
					if (rand() % 3 == 2) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_MICROSMG");
					STREAMING::REQUEST_MODEL(weapH);
					//if(rand() % 2 == 1) scPed[i] = PED::CREATE_PED(26, ennHi, spawnPt1.x, spawnPt1.y, spawnPt1.z, 16, false, true);
					//if(rand() % 2 == 0) scPed[i] = PED::CREATE_PED(26, ennHi, spawnPt2.x, spawnPt2.y, spawnPt2.z, 16, false, true);
					if (rand() % 2 == 0) {
						scVeh[i] = VEHICLE::CREATE_VEHICLE(vehHi, spawnPt1.x, spawnPt1.y, spawnPt1.z, ENTITY::GET_ENTITY_HEADING(playerPed), 0, true);
						if (rand() % 2 == 1) pimpMyRide(0, 8);
						VEHICLE::SET_VEHICLE_FORWARD_SPEED(scVeh[i], 10.0f);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], -1)) scPed[i] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, -1, false, false);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], 0)) scPed[i + 1] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, 0, false, false);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], 1) && VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(scVeh[i]) > 2) scPed[i + 2] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, 1, false, false);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], 2) && VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(scVeh[i]) > 3 && rand() % 2 == 1) scPed[i + 3] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, 2, false, false);
					}
					else {
						scVeh[i] = VEHICLE::CREATE_VEHICLE(vehHi, spawnPt2.x, spawnPt2.y, spawnPt2.z, ENTITY::GET_ENTITY_HEADING(playerPed), 0, true);
						if (rand() % 2 == 1) pimpMyRide(0, 8);
						VEHICLE::SET_VEHICLE_FORWARD_SPEED(scVeh[i], 10.0f);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], -1)) scPed[i] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, -1, false, false);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], 0)) scPed[i + 1] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, 0, false, false);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], 1) && VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(scVeh[i]) > 2) scPed[i + 2] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, 1, false, false);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], 2) && VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(scVeh[i]) > 3 && rand() % 2 == 1) scPed[i + 3] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, 2, false, false);
					}
					AI::TASK_VEHICLE_CHASE(scPed[i], playerPed);
					VEHICLE::SET_VEHICLE_FORWARD_SPEED(PED::GET_VEHICLE_PED_IS_IN(scPed[i], true), 7.0f);
					for (int a = 0; a < proStep; a++) {
						PED::SET_PED_ACCURACY(scPed[i + a], 40);
						WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i + a], weapH, 100, false);
						WEAPON::SET_CURRENT_PED_WEAPON(scPed[i + a], weapH, true);
						PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i + a], 0x90c7da60);
						AI::TASK_COMBAT_PED(scPed[i + a], playerPed, 0, 0);
						UI::REMOVE_BLIP(&blips[i + a]);
						blips[i + a] = UI::ADD_BLIP_FOR_ENTITY(scPed[i + a]);
						mobDead++;
					}
				}

			}
		}
	}
	if (finishedProtect && tick > 200) {
		tick = 0;
		for (int i = 7; i < 24; i += proStep)
		{
			set_spawn_points();
			if ((!ENTITY::DOES_ENTITY_EXIST(scPed[i]) || ENTITY::IS_ENTITY_DEAD(scPed[i])) && rand() % 3 == 1) {
				if (!ENTITY::IS_ENTITY_AT_COORD(playerPed, spawnPt1.x, spawnPt1.y, spawnPt1.z, 35, 35, 35, 0, 1, 0) && !ENTITY::IS_ENTITY_AT_COORD(playerPed, spawnPt2.x, spawnPt2.y, spawnPt2.z, 35, 35, 35, 0, 1, 0)) {
					tick = 0;
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i]);
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[i]);
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i + 2]);
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i + 3]);
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i + 1]);
					//ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[7]);
					//ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[8]);
					weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_MICROSMG");
					if (rand() % 3 == 0) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_ASSAULTRIFLE");
					if (rand() % 3 == 1) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_PUMPSHOTGUN");
					if (rand() % 3 == 2) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_MICROSMG");
					STREAMING::REQUEST_MODEL(weapH);
					//if(rand() % 2 == 1) scPed[i] = PED::CREATE_PED(26, ennHi, spawnPt1.x, spawnPt1.y, spawnPt1.z, 16, false, true);
					//if(rand() % 2 == 0) scPed[i] = PED::CREATE_PED(26, ennHi, spawnPt2.x, spawnPt2.y, spawnPt2.z, 16, false, true);
					if (rand() % 2 == 0) {
						scVeh[i] = VEHICLE::CREATE_VEHICLE(vehHi, spawnPt1.x, spawnPt1.y, spawnPt1.z, ENTITY::GET_ENTITY_HEADING(playerPed), 0, true);
						if (rand() % 2 == 1) pimpMyRide(0, 8);
						VEHICLE::SET_VEHICLE_FORWARD_SPEED(scVeh[i], 10.0f);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], -1)) scPed[i] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, -1, false, false);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], 0)) scPed[i + 1] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, 0, false, false);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], 1) && VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(scVeh[i]) > 2) scPed[i + 2] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, 1, false, false);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], 2) && VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(scVeh[i]) > 3 && rand() % 2 == 1) scPed[i + 3] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, 2, false, false);
					}
					else {
						scVeh[i] = VEHICLE::CREATE_VEHICLE(vehHi, spawnPt2.x, spawnPt2.y, spawnPt2.z, ENTITY::GET_ENTITY_HEADING(playerPed), 0, true);
						if (rand() % 2 == 1) pimpMyRide(0, 8);
						VEHICLE::SET_VEHICLE_FORWARD_SPEED(scVeh[i], 10.0f);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], -1)) scPed[i] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, -1, false, false);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], 0)) scPed[i + 1] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, 0, false, false);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], 1) && VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(scVeh[i]) > 2) scPed[i + 2] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, 1, false, false);
						if (VEHICLE::IS_VEHICLE_SEAT_FREE(scVeh[i], 2) && VEHICLE::GET_VEHICLE_MAX_NUMBER_OF_PASSENGERS(scVeh[i]) > 3 && rand() % 2 == 1) scPed[i + 3] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[i], 26, ennHi, 2, false, false);
					}
					AI::TASK_VEHICLE_CHASE(scPed[i], playerPed);
					VEHICLE::SET_VEHICLE_FORWARD_SPEED(PED::GET_VEHICLE_PED_IS_IN(scPed[i], true), 7.0f);
					for (int a = 0; a < proStep; a++) {
						WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i + a], weapH, 100, false);
						WEAPON::SET_CURRENT_PED_WEAPON(scPed[i + a], weapH, true);
						PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i + a], 0x90c7da60);
						AI::TASK_COMBAT_PED(scPed[i + a], playerPed, 0, 0);
						UI::REMOVE_BLIP(&blips[i + a]);
						blips[i + a] = UI::ADD_BLIP_FOR_ENTITY(scPed[i + a]);
						mobDead++;
					}
				}

			}
		}
	}


}

// SHITS ON FIRE YO.
// RANDOM PIMPING, CAN BE COOL
// CAN BE SHIT
// CAN BE OK.
void pimpMyRide(int type, int id)
{
	if (type == 0) {
		VEHICLE::SET_VEHICLE_MOD_KIT(scVeh[id], 0);
		VEHICLE::TOGGLE_VEHICLE_MOD(scVeh[id], 18, true);
		VEHICLE::TOGGLE_VEHICLE_MOD(scVeh[id], 22, true);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 16, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 16) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 12, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 12) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 11, rand() % (3 - 1 + 1) + 1, false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 14, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 14) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 15, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 15) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 13, 2, false);
		if (id != 1) VEHICLE::SET_VEHICLE_WHEEL_TYPE(scVeh[id], 7);
		VEHICLE::SET_VEHICLE_WINDOW_TINT(scVeh[id], rand() % 5);
		if (id != 1) VEHICLE::SET_VEHICLE_MOD(scVeh[id], 23, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 23) + 1), true);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 0, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 0) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 1, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 1) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 2, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 2) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 3, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 3) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 4, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 4) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 5, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 5) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 6, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 6) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 7, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 7) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 8, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 8) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 9, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 9) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(scVeh[id], 10, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(scVeh[id], 10) + 1), false);
	}
	if (type == 1)
	{
		VEHICLE::SET_VEHICLE_MOD_KIT(borVeh, 0);
		VEHICLE::TOGGLE_VEHICLE_MOD(borVeh, 18, true);
		VEHICLE::TOGGLE_VEHICLE_MOD(borVeh, 22, true);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 16, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 16) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 12, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 12) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 11, rand() % (3 - 1 + 1) + 1, false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 14, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 14) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 15, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 15) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 13, 2, false);
		VEHICLE::SET_VEHICLE_WHEEL_TYPE(borVeh, rand() % 8);
		VEHICLE::SET_VEHICLE_WINDOW_TINT(borVeh, rand() % 5);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 23, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 23) + 1), true);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 0, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 0) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 1, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 1) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 2, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 2) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 3, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 3) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 4, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 4) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 5, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 5) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 6, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 6) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 7, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 7) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 8, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 8) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 9, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 9) + 1), false);
		VEHICLE::SET_VEHICLE_MOD(borVeh, 10, rand() % (VEHICLE::GET_NUM_VEHICLE_MODS(borVeh, 10) + 1), false);
	}
}

// SETUP PROTECTION MISSIONS
void setup_protect()
{
	tick = 1000;
	PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
	PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);
	PLAYER::SET_MAX_WANTED_LEVEL(0);
	mission = true;
	DWORD vehH, pedH, ennH, rumH;

	// debug var
	int debug_var = totalMission;


	switch (rand() % 13){
	case 0: rumH = GAMEPLAY::GET_HASH_KEY("gburrito2"); proStep = 4; break;
	case 1: rumH = GAMEPLAY::GET_HASH_KEY("huntley"); proStep = 4; break;
	case 2: rumH = GAMEPLAY::GET_HASH_KEY("alpha"); proStep = 2; break;
	case 3: rumH = GAMEPLAY::GET_HASH_KEY("voodoo2"); proStep = 2; break;
	case 4: rumH = GAMEPLAY::GET_HASH_KEY("sentinel"); proStep = 2; break;
	case 5: rumH = GAMEPLAY::GET_HASH_KEY("phoenix"); proStep = 2; break;
	case 6: rumH = GAMEPLAY::GET_HASH_KEY("habanero"); proStep = 4; break;
	case 7: rumH = GAMEPLAY::GET_HASH_KEY("oracle"); proStep = 4; break;
	case 8: rumH = GAMEPLAY::GET_HASH_KEY("oracle2"); proStep = 4; break;
	case 9: rumH = GAMEPLAY::GET_HASH_KEY("stanier"); proStep = 4; break;
	case 10: rumH = GAMEPLAY::GET_HASH_KEY("premier"); proStep = 4; break;
	case 11: rumH = GAMEPLAY::GET_HASH_KEY("intruder"); proStep = 4; break;
	case 12: rumH = GAMEPLAY::GET_HASH_KEY("manana"); proStep = 2; break;
	}
	vehHi = rumH;
	int myRand = rand() % 15;
	//int myRand = 0;
	// TODO : ADD MORE PROTECTION MISSIONS
	switch (myRand) {
	case 0:
		vehH = GAMEPLAY::GET_HASH_KEY("btype");
		pedH = GAMEPLAY::GET_HASH_KEY("G_M_M_KorBoss_01");
		ennH = GAMEPLAY::GET_HASH_KEY("G_M_M_ArmGoon_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = 331;
		mobProtect.y = -209;
		mobProtect.z = 55;
		break;
	case 1:
		vehH = GAMEPLAY::GET_HASH_KEY("superd");
		pedH = GAMEPLAY::GET_HASH_KEY("G_M_M_ArmBoss_01");
		ennH = GAMEPLAY::GET_HASH_KEY("G_M_M_ArmLieut_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = 1180;
		mobProtect.y = -1556;
		mobProtect.z = 35;
		break;
	case 2:
		vehH = GAMEPLAY::GET_HASH_KEY("stingergt");
		pedH = GAMEPLAY::GET_HASH_KEY("G_M_M_MexBoss_01");
		ennH = GAMEPLAY::GET_HASH_KEY("G_M_Y_BallaOrig_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = -548;
		mobProtect.y = -1637;
		mobProtect.z = 20;
		break;
	case 3:
		vehH = GAMEPLAY::GET_HASH_KEY("ztype");
		pedH = GAMEPLAY::GET_HASH_KEY("A_F_M_Business_02");
		ennH = GAMEPLAY::GET_HASH_KEY("A_M_M_AfriAmer_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = -609;
		mobProtect.y = -1033;
		mobProtect.z = 22;
		break;
	case 4:
		vehH = GAMEPLAY::GET_HASH_KEY("hotknife");
		pedH = GAMEPLAY::GET_HASH_KEY("A_F_Y_Vinewood_01");
		ennH = GAMEPLAY::GET_HASH_KEY("A_M_Y_MexThug_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = -766;
		mobProtect.y = -414;
		mobProtect.z = 36;
		break;
	case 5:
		vehH = GAMEPLAY::GET_HASH_KEY("infernus");
		pedH = GAMEPLAY::GET_HASH_KEY("A_M_M_Malibu_01");
		ennH = GAMEPLAY::GET_HASH_KEY("U_M_M_BikeHire_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = -690;
		mobProtect.y = 41;
		mobProtect.z = 42;
		break;
	case 6:
		vehH = GAMEPLAY::GET_HASH_KEY("adder");
		pedH = GAMEPLAY::GET_HASH_KEY("A_F_M_BevHills_01");
		ennH = GAMEPLAY::GET_HASH_KEY("A_M_M_SouCent_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = -589;
		mobProtect.y = -755;
		mobProtect.z = 29;
		break;
	case 7:
		vehH = GAMEPLAY::GET_HASH_KEY("buffalo");
		pedH = GAMEPLAY::GET_HASH_KEY("A_F_M_BevHills_01");
		ennH = GAMEPLAY::GET_HASH_KEY("G_M_Y_Azteca_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = 1377;
		mobProtect.y = -738;
		mobProtect.z = 68;
		break;
	case 8:
		vehH = GAMEPLAY::GET_HASH_KEY("gauntlet");
		pedH = GAMEPLAY::GET_HASH_KEY("A_M_Y_Runner_01");
		ennH = GAMEPLAY::GET_HASH_KEY("G_M_Y_Azteca_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = 958;
		mobProtect.y = 969;
		mobProtect.z = 227;
		break;
	case 9:
		vehH = GAMEPLAY::GET_HASH_KEY("ruiner");
		pedH = GAMEPLAY::GET_HASH_KEY("A_M_Y_Hippy_01");
		ennH = GAMEPLAY::GET_HASH_KEY("G_M_Y_Lost_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = -694;
		mobProtect.y = -2193;
		mobProtect.z = 6;
		break;
	case 10:
		vehH = GAMEPLAY::GET_HASH_KEY("dukes");
		pedH = GAMEPLAY::GET_HASH_KEY("S_F_Y_Shop_MID");
		ennH = GAMEPLAY::GET_HASH_KEY("G_M_Y_Lost_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = -94;
		mobProtect.y = -1581;
		mobProtect.z = 32;
		break;
	case 11:
		vehH = GAMEPLAY::GET_HASH_KEY("peyote");
		pedH = GAMEPLAY::GET_HASH_KEY("S_F_Y_SweatShop_01");
		ennH = GAMEPLAY::GET_HASH_KEY("G_M_M_ArmGoon_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = 540;
		mobProtect.y = -138;
		mobProtect.z = 60;
		break;
	case 12:
		vehH = GAMEPLAY::GET_HASH_KEY("fusilade");
		pedH = GAMEPLAY::GET_HASH_KEY("A_F_M_BevHills_01");
		ennH = GAMEPLAY::GET_HASH_KEY("A_M_M_SouCent_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = -128.4;
		mobProtect.y = -1438.4;
		mobProtect.z = 34;
		break;
	case 13:
		vehH = GAMEPLAY::GET_HASH_KEY("voltic");
		pedH = GAMEPLAY::GET_HASH_KEY("A_F_Y_Vinewood_01");
		ennH = GAMEPLAY::GET_HASH_KEY("A_M_Y_MexThug_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = 901;
		mobProtect.y = -1023;
		mobProtect.z = 35;
		break;
	case 14:
		vehH = GAMEPLAY::GET_HASH_KEY("dubsta3");
		pedH = GAMEPLAY::GET_HASH_KEY("S_F_Y_SweatShop_01");
		ennH = GAMEPLAY::GET_HASH_KEY("G_M_Y_Azteca_01");
		pedHi = pedH;
		ennHi = ennH;
		mobProtect.x = 364.318;
		mobProtect.y = -1666;
		mobProtect.z = 32.6;
		break;
	}

	cancelMsg();
	blips[1] = UI::ADD_BLIP_FOR_COORD(mobProtect.x, mobProtect.y, mobProtect.z);
	UI::SET_BLIP_COLOUR(blips[1], 3);
	UI::SET_BLIP_ROUTE(blips[1], true);

	STREAMING::REQUEST_MODEL(vehH);
	STREAMING::REQUEST_MODEL(pedH);
	STREAMING::REQUEST_MODEL(rumH);
	while (!STREAMING::HAS_MODEL_LOADED(vehH) || !STREAMING::HAS_MODEL_LOADED(pedH) || !STREAMING::HAS_MODEL_LOADED(rumH))
		WAIT(100);

	scVeh[6] = VEHICLE::CREATE_VEHICLE(vehH, mobProtect.x, mobProtect.y, mobProtect.z, 180.0f, 0, true);
	blips[4] = UI::ADD_BLIP_FOR_ENTITY(scVeh[6]);
	UI::SET_BLIP_SPRITE(blips[4], 225);
	UI::SET_BLIP_COLOUR(blips[4], 3);
	scPed[6] = PED::CREATE_PED(26, pedH, mobProtect.x + 2.0f, mobProtect.y, mobProtect.z, 16, false, true);
	scVeh[7] = VEHICLE::CREATE_VEHICLE(rumH, spawnPt1.x, spawnPt1.y, spawnPt1.z, 180.0f, 0, true);
	scVeh[8] = VEHICLE::CREATE_VEHICLE(rumH, spawnPt2.x, spawnPt2.y, spawnPt2.z, 180.0f, 0, true);
	//ENTITY::SET_ENTITY_INVINCIBLE(scVeh[7], true);
	//ENTITY::SET_ENTITY_INVINCIBLE(scVeh[8], true);
	VEHICLE::SET_VEHICLE_DOOR_OPEN(scVeh[7], 2, false, false);
	VEHICLE::SET_VEHICLE_DOOR_OPEN(scVeh[7], 3, false, false);
	VEHICLE::SET_VEHICLE_DOOR_OPEN(scVeh[8], 2, false, false);
	VEHICLE::SET_VEHICLE_DOOR_OPEN(scVeh[8], 3, false, false);
	WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[6], GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), 1000, false);
	ENTITY::SET_ENTITY_HEALTH(scPed[6], 200);
	PED::SET_PED_CAN_RAGDOLL(scPed[6], false);
	// like group
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(2, GAMEPLAY::GET_HASH_KEY("player"), 0x90c7da61);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(2, 0x90c7da61, GAMEPLAY::GET_HASH_KEY("player"));
	// hate group
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, GAMEPLAY::GET_HASH_KEY("player"), 0x90c7da60);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, 0x90c7da60, GAMEPLAY::GET_HASH_KEY("player"));
	PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[6], 0x90c7da61);
	blips[6] = UI::ADD_BLIP_FOR_ENTITY(scPed[6]);
	UI::SET_BLIP_COLOUR(blips[6], 3);

	// pimp it
	pimpMyRide(0, 6);

	//CANCEL
	canCancel = true;

}

void setup_mobClean()
{
	PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
	PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);
	PLAYER::SET_MAX_WANTED_LEVEL(0);
	cancelMsg();
	mission = true;
	//char * guy[4] = { "G_M_Y_MexGoon_01", "G_M_M_ChiGoon_01", "G_M_Y_BallaEast_01", "G_M_Y_Lost_01" };
	random = rand() % 3;
	if (random == 0) mobClean1();
	if (random == 1) mobClean2();
	if (random == 2) mobClean3();
	UI::SET_BLIP_ROUTE(blips[1], true);

	//CANCEL
	canCancel = true;


}


void cleanupBlips()
{
	for (int i = 7; i < 32; i++)
	{
		if (ENTITY::IS_ENTITY_DEAD(scPed[i])) {
			UI::REMOVE_BLIP(&blips[i]);
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i]);
		}
	}
}

// STOPS THE MUSIC.
void stopMusic()
{
	AUDIO::REQUEST_SCRIPT_AUDIO_BANK("SCRIPT\RAMPAGE_01", 0);
	AUDIO::REQUEST_SCRIPT_AUDIO_BANK("SCRIPT\RAMPAGE_02", 0);
	AUDIO::START_AUDIO_SCENE("RAMPAGE_REDNECKS_SCENE");
	AUDIO::PREPARE_MUSIC_EVENT("RAMPAGE_STOP");
	AUDIO::TRIGGER_MUSIC_EVENT("RAMPAGE_STOP");
}

// MAIN CLEANING FUNCTIONS.
// RESETS EVERYTHING THAT WAS SPAWNED, REMOVE BLIPS.
// RESET RELATIONS.
void cleanupMissions(int type)
{
	// type 0 = mission complete
	// type 1 = mission canceled
	//GAMEPLAY::SET_MISSION_FLAG(0);
	//GAMEPLAY::SET_SAVE_MENU_ACTIVE(true);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(255, GAMEPLAY::GET_HASH_KEY("player"), 0x783e3868);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(255, 0x783e3868, GAMEPLAY::GET_HASH_KEY("player"));
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(255, 0x90c7da60, 0x783e3868);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(255, 0x783e3868, 0x90c7da60);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(255, GAMEPLAY::GET_HASH_KEY("player"), 0x90c7da60);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(255, 0x90c7da60, GAMEPLAY::GET_HASH_KEY("player"));
	mission = false;
	canCancel = false;
	stopMusic();
	AUDIO::RELEASE_MISSION_AUDIO_BANK();
	AUDIO::RELEASE_SCRIPT_AUDIO_BANK();
	mobDead = 0;
	PLAYER::SET_MAX_WANTED_LEVEL(5);
	for (int i = 32; i < 32; i++){
		wasKilled[i] = false;
	}
	PED::RESET_AI_WEAPON_DAMAGE_MODIFIER();
	joinedClean = false;
	joinedProtect = false;
	sentMen = false;
	finishedProtect = false;
	isDrivingAway = false;

	for (int i = 0; i < 64; i++)
	{
		Any dVeh = PED::GET_VEHICLE_PED_IS_IN(scPed[i], true);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&dVeh);
	}

	if (type == 0)
	{
		AUDIO::PLAY_SOUND_FRONTEND(-1, "PROPERTY_PURCHASE", "HUD_AWARDS", 1);
		selectedType = 0;
		for (int i = 0; i < 64; i++)
		{
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[i]);
			UI::REMOVE_BLIP(&blips[i]);
		}
		for (int i = 6; i < 64; i++)
		{
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i]);
			UI::SET_BLIP_ROUTE(blips[i], false);
		}
		std::string pay;
		setTextMsg(500, "~g~Mission complete~w~");
		pay = "Great job! Come back at the ~r~RedHouse~w~, The ~g~$";
		pay += std::to_string((contractPayout[uiSelected] / 1000)) + ",000~w~ are in the garage.";
		setTextMsg(2000, (char*)pay.c_str());
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		setupPayout();
	}
	if (type == 1)
	{
		
		AUDIO::PLAY_SOUND_FRONTEND(-1, "1st_Person_Transition", "PLAYER_SWITCH_CUSTOM_SOUNDSET", 1);
		selectedType = 0;
		contractPayout[0] = contractPayout[1] = contractPayout[2] = 0;
		for (int i = 0; i < 64; i++)
		{
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[i]);
		}
		for (int i = 0; i < 64; i++)
		{
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i]);
			UI::SET_BLIP_ROUTE(blips[i], false);
			UI::REMOVE_BLIP(&blips[i]);
		}
	}
}

// DISPLAYS OBJECTIVE.
// CALLED ONCE PER FRAME.
void message(char * text)
{
	UI::SET_TEXT_FONT(0);
	UI::SET_TEXT_SCALE(0.4f, 0.4f);
	UI::SET_TEXT_COLOUR(255, 255, 255, 255);
	UI::SET_TEXT_WRAP(0.0, 1.0);
	UI::SET_TEXT_CENTRE(true);
	UI::SET_TEXT_DROPSHADOW(2, 2, 0, 0, 0);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(text);
	UI::_DRAW_TEXT(0.5, 0.95);
}

void setup_Assassination(){
	callBack = 0;
	cancelMsg();
	mission = true;
	setupSpawnPts(1);
	setupCars(1);

	char * guy[5] = { "A_M_M_Business_01", "A_M_Y_BevHills_02", "A_M_Y_Golfer_01", "A_M_Y_Gay_02", "A_M_Y_Polynesian_01" };
	random = rand() % 5;
	int spawnR = rand() % 10;

	bool targetAlive = true;
	DWORD vehH, pedH;
	vehH = GAMEPLAY::GET_HASH_KEY(cars[random]);
	pedH = GAMEPLAY::GET_HASH_KEY(guy[random]);

	STREAMING::REQUEST_MODEL(vehH);
	STREAMING::REQUEST_MODEL(pedH);
	WAIT(1000);

	scVeh[6] = VEHICLE::CREATE_VEHICLE(vehH, assMission[spawnR].x, assMission[spawnR].y, assMission[spawnR].z, 180.0f, 0, true);
	scPed[6] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, -1, false, false);
	scPed[7] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[6], 3, pedH, 0, false, false);
	ENTITY::SET_ENTITY_HEALTH(scPed[6], 900);
	WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[6], GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), 1000, false);
	WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[7], GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), 1000, false);
	PED::SET_PED_ACCURACY(scPed[6], 95);

	blips[6] = UI::ADD_BLIP_FOR_ENTITY(scPed[6]);
	//ON-FOOT ASSASSINATION
	if (rand() % 2 == 0){ AI::TASK_VEHICLE_DRIVE_TO_COORD(scPed[6], scVeh[6], 490, -3359, 6.5, 50, 1, ENTITY::GET_ENTITY_MODEL(scVeh[6]), 1, 5.0, -1); }
	else {
		AI::TASK_WANDER_STANDARD(scPed[6], 0x471c4000, 0);
		AI::TASK_VEHICLE_CHASE(scPed[7], playerPed);
	}
	PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[6], GAMEPLAY::GET_HASH_KEY("ENEMIES"));
	PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[7], GAMEPLAY::GET_HASH_KEY("ENEMIES"));
	PED::SET_PED_SEEING_RANGE(scPed[6], 150.0f);


	// CANCEL
	canCancel = true;

}

// COVERT OPS
void setup_covertOps()
{
	covertOps = true;
	int myrand = 0;
	mission = true;
	int thresh = 0;
	PLAYER::SET_PLAYER_CONTROL(PLAYER::PLAYER_ID(), 0, 0);
	AUDIO::REQUEST_SCRIPT_AUDIO_BANK("TIME_LAPSE", 0);
	WAIT(1000);
	AUDIO::START_AUDIO_SCENE("TOD_SHIFT_SCENE");
	int soundid = AUDIO::GET_SOUND_ID();
	Vector3 campos = CAM::GET_GAMEPLAY_CAM_COORD();
	// CAM WORK
	Any cam = CAM::CREATE_CAMERA_WITH_PARAMS(0x19286a9, CAM::_0xA200EB1EE790F448().x, CAM::_0xA200EB1EE790F448().y, CAM::_0xA200EB1EE790F448().z, CAM::_0x5B4E4C817FCC2DFB(2).x, CAM::_0x5B4E4C817FCC2DFB(2).y, CAM::_0x5B4E4C817FCC2DFB(2).z, CAM::_0x80EC114669DAEFF4(), 1, 2);
	Any cam2 = CAM::CREATE_CAMERA_WITH_PARAMS(0x19286a9, CAM::_0xA200EB1EE790F448().x, CAM::_0xA200EB1EE790F448().y, CAM::_0xA200EB1EE790F448().z, CAM::_0x5B4E4C817FCC2DFB(2).x, CAM::_0x5B4E4C817FCC2DFB(2).y, CAM::_0x5B4E4C817FCC2DFB(2).z, CAM::_0x80EC114669DAEFF4(), 1, 2);

	CAM::SET_CAM_COORD(cam2, 59, 2757, 61);
	CAM::SET_CAM_COORD(cam, campos.x, campos.y, campos.z);
	CAM::POINT_CAM_AT_ENTITY(playerPed, 0, 0, 0, 0, 0);
	CAM::POINT_CAM_AT_COORD(cam2, 57.78, 2793, 62.27);
	CAM::SET_CAM_FOV(cam, 50.0f);
	CAM::SET_CAM_FOV(cam2, 70.0f);
	CAM::SET_CAM_ACTIVE_WITH_INTERP(cam, cam2, 7000, true, true);
	CAM::RENDER_SCRIPT_CAMS(1, 0, 3000, 1, 0);
	AUDIO::PLAY_SOUND_FRONTEND(soundid, "TIME_LAPSE_MASTER", 0, 1);
	
	int days = 0;
	while (days < 3)
	{
		if (days < 2) {
			if (TIME::GET_CLOCK_HOURS() < 23) { TIME::SET_CLOCK_TIME(TIME::GET_CLOCK_HOURS() + 1, 0, 0); }
			if (TIME::GET_CLOCK_HOURS() == 23) { TIME::SET_CLOCK_TIME(0, 0, 0); days++; if (rand() % 2 == 0) GAMEPLAY::SET_WEATHER_TYPE_NOW("RAIN"); }
			if (days > 1) { GAMEPLAY::SET_WEATHER_TYPE_NOW("SMOG"); }
		}
		
		if (TIME::GET_CLOCK_HOURS() != hours){ TIME::SET_CLOCK_TIME(TIME::GET_CLOCK_HOURS() + 1, 0, 0); }
		else { days = 3; }
		WAIT(120);
	}
	
	while (CAM::IS_CAM_INTERPOLATING(cam2) || CAM::IS_CAM_INTERPOLATING(cam)) WAIT(0);
	CAM::RENDER_SCRIPT_CAMS(0, 0, 3000, 1, 0);
	CAM::DESTROY_CAM(cam, true);
	CAM::DESTROY_CAM(cam2, true);
	
	AUDIO::STOP_SOUND(soundid);
	AUDIO::STOP_AUDIO_SCENE("TOD_SHIFT_SCENE");
	AUDIO::RELEASE_NAMED_SCRIPT_AUDIO_BANK("TIME_LAPSE");
	PLAYER::SET_PLAYER_CONTROL(PLAYER::PLAYER_ID(), 1, 0);
	//mission = false;

	// WE WILL HAVE 3 RANDOM MISSIONS
	//myrand = rand() % 2;
	if (uiSelected < 2)
	{
		bool radioMessage = false;
		GAMEPLAY::SET_WEATHER_TYPE_NOW("THUNDER");
		setTextMsg(0, "Guys are at a Motel somewhere near Sandy Shores. Get there.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Franklin", 1);

		blips[63] = UI::ADD_BLIP_FOR_COORD(1552, 3487, 36.5);
		UI::SET_BLIP_COLOUR(blips[63], 3);
		UI::SET_BLIP_ROUTE(blips[63], true);

		// SETUP SCENE
		DWORD deal, mar, scrap, burrito, weapH;
		deal = GAMEPLAY::GET_HASH_KEY("S_M_Y_Dealer_01");
		mar = GAMEPLAY::GET_HASH_KEY("g_m_y_lost_03");
		scrap = GAMEPLAY::GET_HASH_KEY("scrap");
		burrito = GAMEPLAY::GET_HASH_KEY("burrito");

		STREAMING::REQUEST_MODEL(deal);
		STREAMING::REQUEST_MODEL(mar);
		STREAMING::REQUEST_MODEL(scrap);
		STREAMING::REQUEST_MODEL(burrito);

		while (!ENTITY::IS_ENTITY_AT_COORD(playerPed, 1552, 3487, 36.5, 15.0, 15.0, 15.0, 0, 1, 0)){
			WAIT(0);
			AUDIO::SET_VEHICLE_RADIO_ENABLED(PED::GET_VEHICLE_PED_IS_IN(playerPed, true), false);
			if ((CONTROLS::IS_CONTROL_JUST_PRESSED(0, 189) || GetAsyncKeyState('A')) && !radioMessage) {
				radioMessage = true;
				setTextMsg(0, "I've removed the radios from the cars, don't listen to music during missions.");
				AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Franklin", 1);
			}
			message("Get to the ~b~destination.");
			if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, false))
			{
				cleanupMissions(1);
				setTextMsg(2000, "~r~Mission failed ~w~:~n~I thought you were a pro !");
				for (int i = 0; i < 5; i++){ ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&bodyG[i]); }
				goto end;
			}
		}

		UI::REMOVE_BLIP(&blips[63]);

		myrand = rand() % 3;
		if(myrand == 0) setTextMsg(0, "Hey tough boy, remember : no big explosions, stick to silencers.");
		if(myrand == 1) setTextMsg(0, "I want a clean job, no RPGs and all that Shan stuff.");
		if(myrand == 2) setTextMsg(0, "Remember, you're solo, so no big noise. Clean job.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Franklin", 1);

		UI::TOGGLE_STEALTH_RADAR(true);

		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, GAMEPLAY::GET_HASH_KEY("player"), 0x90c7da60);
		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, 0x90c7da60, GAMEPLAY::GET_HASH_KEY("player"));

		scPed[6] = PED::CREATE_PED(26, deal, 1578.658, 3599.093, 35.330, 122.0, false, false);
		scPed[7] = PED::CREATE_PED(26, deal, 1528.630, 3589.894, 35.452, 15.501, false, false);
		scPed[8] = PED::CREATE_PED(26, deal, 1538.184, 3583.007, 38.767, 205.509, false, false);
		scPed[9] = PED::CREATE_PED(26, deal, 1505.088, 3575.725, 35.435, 20.021, false, false);

		// PATROLS
		scPed[10] = PED::CREATE_PED(26, mar, 1510.633, 3564.483, 38.730, 161.783, false, false);
		scPed[11] = PED::CREATE_PED(26, mar, 1563.902, 3616.729, 35.228, 208.375, false, false);
		scPed[12] = PED::CREATE_PED(26, mar, 1533.266, 3527.331, 35.357, 340.702, false, false);
	
		// VEHICLES
		scVeh[6] = VEHICLE::CREATE_VEHICLE(scrap, 1572.962, 3590.479, 35.338, 118, false, false);
		scVeh[7] = VEHICLE::CREATE_VEHICLE(burrito, 1523.663, 3567.066, 35.165, 268, false, false);
		
		if (hardcore) UI::DISPLAY_RADAR(false);
		// ARM THEM
		for (int i = 6; i < 15; i++)
		{
			myrand = rand() % 3;
			blips[i] = UI::ADD_BLIP_FOR_ENTITY(scPed[i]);
			UI::SET_BLIP_SHOW_CONE(blips[i], true);
			PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i], 0x90c7da60);
			if (myrand == 0) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_BULLPUPSHOTGUN");
			if (myrand == 1) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_SMG");
			if (myrand == 2) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_ASSAULTRIFLE");
			PED::SET_PED_SEEING_RANGE(scPed[i], 40.0);
			PED::SET_PED_HEARING_RANGE(scPed[i], 40.0);
			STREAMING::REQUEST_MODEL(weapH);
			WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i], weapH, 1000, false);
			WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(scPed[i], weapH, GAMEPLAY::GET_HASH_KEY("COMPONENT_AT_AR_FLSH"));
			WEAPON::SET_CURRENT_PED_WEAPON(scPed[i], weapH, true);
		}

		// PATROLS
		AI::OPEN_PATROL_ROUTE("MISS_PATROL_6");
		AI::ADD_PATROL_ROUTE_NODE(1, "WORLD_HUMAN_GUARD_STAND", 1570.243, 3598.664, 38.734, 1570.243, 3598.664, 38.734, 3000);
		AI::ADD_PATROL_ROUTE_NODE(2, "WORLD_HUMAN_GUARD_STAND", 1562.056, 3612.835, 38.734, 1562.056, 3612.835, 38.734, 3000);
		AI::ADD_PATROL_ROUTE_LINK(1, 2);
		AI::ADD_PATROL_ROUTE_LINK(2, 1);
		AI::CLOSE_PATROL_ROUTE();
		AI::CREATE_PATROL_ROUTE();

		AI::OPEN_PATROL_ROUTE("MISS_PATROL_7");
		AI::ADD_PATROL_ROUTE_NODE(1, "WORLD_HUMAN_GUARD_STAND", 1579.792, 3576.234, 35.569, 1579.792, 3576.234, 35.569, 3000);
		AI::ADD_PATROL_ROUTE_NODE(2, "WORLD_HUMAN_GUARD_STAND", 1593.029, 3547.181, 35.435, 1593.029, 3547.181, 35.435, 3000);
		AI::ADD_PATROL_ROUTE_LINK(1, 2);
		AI::ADD_PATROL_ROUTE_LINK(2, 1);
		AI::CLOSE_PATROL_ROUTE();
		AI::CREATE_PATROL_ROUTE();
		bool alldead = false;
		int dead = 0;

		// PATROL ORDERS
		AI::TASK_PATROL(scPed[10], "MISS_PATROL_6", 1, 0, 1);
		AI::TASK_PATROL(scPed[11], "MISS_PATROL_7", 1, 0, 1);

		while (!alldead){
			WAIT(0);
			PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
			PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, true);
			dead = 0;
			message("Eliminate silently all the ~r~enemies.");
			for (int i = 6; i < 15; i++)
			{
				if (ENTITY::IS_ENTITY_DEAD(scPed[i])) {
					dead++; UI::REMOVE_BLIP(&blips[i]);
					ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i]);
				}
				if (thresh > 1) {
					cleanupMissions(1);
					setTextMsg(0, "My missions require a little bit more finess than drawing an RPG and blowing everything up.");
					AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Franklin", 1); goto end;
				}
				if (PED::IS_PED_SHOOTING(scPed[i])) { thresh++; if (hardcore) thresh = 2; }
			}
			if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, false))
			{
				cleanupMissions(1);
				setTextMsg(2000, "~r~Mission failed ~w~:~n~Try again.");
				for (int i = 0; i < 5; i++){ ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&bodyG[i]); }
				goto end;
			}
			if (dead == 9) alldead = true;
		}


		STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("prop_drug_package"));
		STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("pickup_custom_script"));
		WAIT(1000);

		// PICKUP
		// OBJECT::HAS_PICKUP_BEEN_COLLECTED
		// OBJECT::CREATE_PICKUP(${pickup_custom_script}, v_6, a_1._f1, a_1, 1, 0);
		// prop_drug_package


		Any drug;
		myrand = rand() % 4;
		//myrand == 0;
		if (myrand == 0) drug = OBJECT::CREATE_PICKUP_ROTATE(GAMEPLAY::GET_HASH_KEY("pickup_custom_script"), 1512.86, 3571.56, 38.7, -90.0, 0.0, 90, 0, 0, 2, 1, GAMEPLAY::GET_HASH_KEY("prop_drug_package"));
		if (myrand == 1) drug = OBJECT::CREATE_PICKUP_ROTATE(GAMEPLAY::GET_HASH_KEY("pickup_custom_script"), 1533.77, 3586.8, 38.7, -90.0, 0.0, 90, 0, 0, 2, 1, GAMEPLAY::GET_HASH_KEY("prop_drug_package"));
		if (myrand == 2) drug = OBJECT::CREATE_PICKUP_ROTATE(GAMEPLAY::GET_HASH_KEY("pickup_custom_script"), 1511, 3573.5, 38.7, -90.0, 0.0, 90, 0, 0, 2, 1, GAMEPLAY::GET_HASH_KEY("prop_drug_package"));
		if (myrand == 3) drug = OBJECT::CREATE_PICKUP_ROTATE(GAMEPLAY::GET_HASH_KEY("pickup_custom_script"), 1579.89, 3616, 38.7, -90.0, 0.0, 90, 0, 0, 2, 1, GAMEPLAY::GET_HASH_KEY("prop_drug_package"));

		setTextMsg(0, "You're good at this. Now find the drug package.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Franklin", 1);



		Vector3 drugPos = OBJECT::GET_PICKUP_COORDS(drug);

		blips[62] = UI::ADD_BLIP_FOR_COORD(drugPos.x, drugPos.y, drugPos.z);
		UI::SET_BLIP_COLOUR(blips[62], 3);
		UI::SET_BLIP_SPRITE(blips[62], 9);
		UI::SET_BLIP_SCALE(blips[62], 0.75f);
		UI::SET_BLIP_ALPHA(blips[62], 30);

		// TIME TO SEARCH NOW
		UI::DISPLAY_RADAR(true);

		while (!ENTITY::IS_ENTITY_AT_COORD(playerPed, drugPos.x, drugPos.y, drugPos.z, 0.7, 0.7, 0.7, 0, 1, 0)) {
			WAIT(0);
			message("Find the ~b~drug package.");
			if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, false))
			{
				cleanupMissions(1);
				setTextMsg(2000, "~r~Mission failed ~w~:~n~I have no words... You really suck at this.");
				for (int i = 0; i < 5; i++){ ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&bodyG[i]); }
				goto end;
			}
		}

		UI::REMOVE_BLIP(&blips[63]);
		myrand = rand() % 3;
		if (myrand == 0) setTextMsg(0, "Impressive. I think we're gonna like each others...");
		if (myrand == 1) setTextMsg(0, "You made it alive, and the gang don't even know what hit them.");
		if (myrand == 2) setTextMsg(0, "It takes a little bit more skills than what Shan ask you for. But good job.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Franklin", 1);
		WAIT(2000);

		int pId;
		if (ENTITY::GET_ENTITY_MODEL(playerPed) == GAMEPLAY::GET_HASH_KEY("Player_Zero")) pId = 0;
		if (ENTITY::GET_ENTITY_MODEL(playerPed) == GAMEPLAY::GET_HASH_KEY("Player_One")) pId = 1;
		if (ENTITY::GET_ENTITY_MODEL(playerPed) == GAMEPLAY::GET_HASH_KEY("Player_Two")) pId = 2;

		// YES EVEN THOUGH HE MADE IT.
		cleanupMissions(1);

		char statNameFull[32];
		sprintf_s(statNameFull, "SP%d_TOTAL_CASH", pId);
		Hash hash = GAMEPLAY::GET_HASH_KEY(statNameFull);
		int val;
		STATS::STAT_GET_INT(hash, &val, -1);
		if (hardcore) { val += 950000; }
		else { val += 450000; }
		STATS::STAT_SET_INT(hash, val, 1);
		
	}

	if (uiSelected == 2)
	{
		GAMEPLAY::SET_WEATHER_TYPE_NOW("CLEAR");

		setTextMsg(0, "Alright, time for a more serious operation. These rednecks got an Insurgent that I might like.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Franklin", 1);
		WAIT(2000);
		setTextMsg(0, "They are hiding it on a scrapyard, get there and bring it back. Remember : clean job.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Franklin", 1);

		blips[63] = UI::ADD_BLIP_FOR_COORD(1552, 3487, 36.5);
		UI::SET_BLIP_COLOUR(blips[63], 3);
		UI::SET_BLIP_ROUTE(blips[63], true);
	}

	end:
	myrand = 0;
	hardcore = false;
	UI::DISPLAY_RADAR(true);
	cleanupMissions(1);

}


void menu(bool active, int type)
{
	if (active)
	{

		// TYPE ::
		// 0 : RECRUIT
		// 1 : ASSASSINATION
		// 2 : DRUG LORD TAKEDOWN
		// 3 : MOB CLEANING

		if (type == 0)
		{
			gxt = "INPUT_FRONTEND_RIGHT";
			setTextMsg(1000, "Press Right or [K] for an Exemplar crew (~g~3 men~w~).~n~Price : ~g~$25,000");
			setTextMsg(1000, "Press Left or [L] for an Bison crew (~g~5 men~w~).~n~Price : ~g~$90,000");
			setTextMsg(1000, "Press LS or [J] for an ~y~elite~w~ crew (~g~3 ~y~elite~g~ men~w~).~n~Price : ~g~$40,000");
		}
	}
}

// HIRED PROTECTION.
void setAssassinBackup()
{
	DWORD vehH, pedH;
	vehH = GAMEPLAY::GET_HASH_KEY(cars[random]);
	pedH = GAMEPLAY::GET_HASH_KEY("G_M_Y_MexGoon_01");

	STREAMING::REQUEST_MODEL(vehH);
	STREAMING::REQUEST_MODEL(pedH);

	while (!STREAMING::HAS_MODEL_LOADED(vehH) || !STREAMING::HAS_MODEL_LOADED(pedH))
		WAIT(100);
	set_spawn_points();
	scVeh[7] = VEHICLE::CREATE_VEHICLE(vehH, spawnPt1.x, spawnPt1.y, spawnPt1.z, 180.0f, 0, true);
	scVeh[8] = VEHICLE::CREATE_VEHICLE(vehH, spawnPt2.x, spawnPt2.y, spawnPt2.z, 180.0f, 0, true);
	//scVeh[7] = VEHICLE::CREATE_VEHICLE(vehH, position.x, position.y, position.z, 180.0f, 0, true);
	//scVeh[8] = VEHICLE::CREATE_VEHICLE(vehH, position.x, position.y, position.z, 180.0f, 0, true);
	scPed[8] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[7], 18, pedH, -1, false, false);
	scPed[9] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[7], 18, pedH, 0, false, false);
	scPed[10] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[7], 18, pedH, 1, false, false);
	scPed[11] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[7], 18, pedH, 2, false, false);
	scPed[12] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 18, pedH, -1, false, false);
	scPed[13] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 18, pedH, 0, false, false);
	scPed[14] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 18, pedH, 1, false, false);
	scPed[15] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 18, pedH, 2, false, false);

	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, GAMEPLAY::GET_HASH_KEY("player"), 0x90c7da60);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, 0x90c7da60, GAMEPLAY::GET_HASH_KEY("player"));

	for (int i = 7; i < 16; i++)
	{
		WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i], GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), 1000, false);
		PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i], 0x90c7da60);
		WEAPON::SET_CURRENT_PED_WEAPON(scPed[i], GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), true);
	}


	AI::TASK_VEHICLE_CHASE(scPed[8], playerPed);
	AI::TASK_VEHICLE_CHASE(scPed[10], playerPed);
}

void setMobCleanBackup()
{
	setTextMsg(1000, "Oh shit! The Feds are gonna show up in no time, clean the place and get out quick!");
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
	DWORD vehH, pedH;
	vehH = GAMEPLAY::GET_HASH_KEY("police4");
	pedH = GAMEPLAY::GET_HASH_KEY("S_M_M_CIASec_01");

	STREAMING::REQUEST_MODEL(vehH);
	STREAMING::REQUEST_MODEL(pedH);

	while (!STREAMING::HAS_MODEL_LOADED(vehH) || !STREAMING::HAS_MODEL_LOADED(pedH))
		WAIT(100);
	set_spawn_points();

	scVeh[9] = VEHICLE::CREATE_VEHICLE(vehH, spawnPt1.x, spawnPt1.y, spawnPt1.z, 180.0f, 0, true);
	scVeh[10] = VEHICLE::CREATE_VEHICLE(vehH, spawnPt2.x, spawnPt2.y, spawnPt2.z, 180.0f, 0, true);
	scPed[17] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[9], 18, pedH, -1, false, false);
	scPed[18] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[9], 18, pedH, 0, false, false);
	scPed[19] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[9], 18, pedH, 1, false, false);
	scPed[20] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[9], 18, pedH, 2, false, false);
	scPed[21] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[10], 18, pedH, -1, false, false);
	scPed[22] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[10], 18, pedH, 0, false, false);
	scPed[23] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[10], 18, pedH, 1, false, false);
	scPed[24] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[10], 18, pedH, 2, false, false);

	VEHICLE::SET_VEHICLE_SIREN(scVeh[9], true);
	VEHICLE::SET_VEHICLE_SIREN(scVeh[10], true);

	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, GAMEPLAY::GET_HASH_KEY("player"), 0x90c7da61);
	PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, 0x90c7da61, GAMEPLAY::GET_HASH_KEY("player"));

	for (int i = 17; i < 25; i++)
	{
		WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i], GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), 1000, false);
		PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i], 0x90c7da60);
		WEAPON::SET_CURRENT_PED_WEAPON(scPed[i], GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), true);
	}


	AI::TASK_VEHICLE_CHASE(scPed[17], scPed[6]);
	AI::TASK_VEHICLE_CHASE(scPed[21], scPed[6]);

}

void recruit(int type)
{
	// types ::
	// 0 EXEMPLAR CREW
	// 1 BISON CREW
	// 2 ELITE CREW
	int pId;
	if (ENTITY::GET_ENTITY_MODEL(playerPed) == GAMEPLAY::GET_HASH_KEY("Player_Zero")) pId = 0;
	if (ENTITY::GET_ENTITY_MODEL(playerPed) == GAMEPLAY::GET_HASH_KEY("Player_One")) pId = 1;
	if (ENTITY::GET_ENTITY_MODEL(playerPed) == GAMEPLAY::GET_HASH_KEY("Player_Two")) pId = 2;

	// EXEMPLAR + 3 CREW GUY
	if (type == 0)
	{
		int val;
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&borVeh);
		DWORD exemp = GAMEPLAY::GET_HASH_KEY("exemplar");
		DWORD gang1 = GAMEPLAY::GET_HASH_KEY("g_m_y_korean_01");
		STREAMING::REQUEST_MODEL(exemp);
		STREAMING::REQUEST_MODEL(gang1);
		WAIT(1000);
		for (int i = 0; i < 5; i++)
		{
			WEAPON::REMOVE_ALL_PED_WEAPONS(bodyG[i], true);
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&bodyG[i]);
		}

		// REMOVE 25000$
		bool can = false;
		char statNameFull[32];
		sprintf_s(statNameFull, "SP%d_TOTAL_CASH", pId);
		Hash hash = GAMEPLAY::GET_HASH_KEY(statNameFull);
		STATS::STAT_GET_INT(hash, &val, -1);
		if (val > 25000) {
			can = true;
			val -= 25000;
			STATS::STAT_SET_INT(hash, val, 1);
		}
		else { setTextMsg(500, "You don't have enough cash."); }
		if (can) {
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&payVeh);
			borVeh = VEHICLE::CREATE_VEHICLE(exemp, 135.3, -1050.86, 29.3, 159.6, 0, true);
			VEHICLE::TOGGLE_VEHICLE_MOD(borVeh, 18, true);
			VEHICLE::TOGGLE_VEHICLE_MOD(borVeh, 22, true);
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(borVeh, false);
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&borVeh);

			// CREW
			STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("WEAPON_CARBINERIFLE"));
			PED::SET_PED_AS_GROUP_LEADER(playerPed, PLAYER::GET_PLAYER_GROUP(player));
			for (int i = 0; i < 3; i++)
			{
				bodyG[i] = PED::CREATE_PED(26, gang1, 150, -1054, 29.2, 160, false, true);
				WEAPON::GIVE_DELAYED_WEAPON_TO_PED(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_CARBINERIFLE"), 1000, false);
				WEAPON::GIVE_DELAYED_WEAPON_TO_PED(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_MICROSMG"), 1000, false);
				PED::SET_PED_RELATIONSHIP_GROUP_HASH(bodyG[i], GAMEPLAY::GET_HASH_KEY("player"));
				PED::SET_PED_AS_GROUP_MEMBER(bodyG[i], PLAYER::GET_PLAYER_GROUP(player));
				PED::SET_PED_CAN_RAGDOLL(bodyG[i], false);
				PED::SET_PED_SHOOT_RATE(bodyG[i], 8000);
				ENTITY::SET_ENTITY_HEALTH(bodyG[i], 250);
				WEAPON::SET_CURRENT_PED_WEAPON(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_CARBINERIFLE"), true);
			}
			setTextMsg(1000, "Your Crew joined, the Exemplar is available in the garage.");
			AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);

		}
		else { setTextMsg(500, "You don't have enough cash."); }


	}
	// BISON + 5 GUYS (AP PISTOLS + SNIPER + M4 + MG)
	if (type == 1)
	{
		int val;
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&borVeh);
		DWORD exemp = GAMEPLAY::GET_HASH_KEY("bison");
		DWORD gang1 = GAMEPLAY::GET_HASH_KEY("g_m_y_korean_01");
		STREAMING::REQUEST_MODEL(exemp);
		STREAMING::REQUEST_MODEL(gang1);
		WAIT(1000);
		for (int i = 0; i < 5; i++)
		{
			WEAPON::REMOVE_ALL_PED_WEAPONS(bodyG[i], true);
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&bodyG[i]);
		}

		// REMOVE 50000$
		bool can = false;
		char statNameFull[32];
		sprintf_s(statNameFull, "SP%d_TOTAL_CASH", pId);
		Hash hash = GAMEPLAY::GET_HASH_KEY(statNameFull);
		STATS::STAT_GET_INT(hash, &val, -1);
		if (val > 90000) {
			can = true;
			val -= 90000;
			STATS::STAT_SET_INT(hash, val, 1);
		}
		else { setTextMsg(500, "You don't have enough cash."); }
		if (can) {
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&payVeh);
			borVeh = VEHICLE::CREATE_VEHICLE(exemp, 135.3, -1050.86, 29.3, 159.6, 0, true);
			VEHICLE::TOGGLE_VEHICLE_MOD(borVeh, 18, true);
			VEHICLE::TOGGLE_VEHICLE_MOD(borVeh, 22, true);
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(borVeh, false);
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&borVeh);

			STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("WEAPON_CARBINERIFLE"));
			STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("WEAPON_APPISTOL"));
			STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("WEAPON_COMBATMG"));
			STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("WEAPON_ASSAULTSHOTGUN"));
			PED::SET_PED_AS_GROUP_LEADER(playerPed, PLAYER::GET_PLAYER_GROUP(player));
			for (int i = 0; i < 5; i++)
			{
				bodyG[i] = PED::CREATE_PED(26, gang1, 150, -1054, 29.2, 160, false, true);
				if (i < 3) WEAPON::GIVE_DELAYED_WEAPON_TO_PED(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_CARBINERIFLE"), 1000, false);
				if (i > 2) WEAPON::GIVE_DELAYED_WEAPON_TO_PED(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_COMBATMG"), 1000, false);
				if (i == 4)WEAPON::GIVE_DELAYED_WEAPON_TO_PED(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_ASSAULTSHOTGUN"), 1000, false);
				WEAPON::GIVE_DELAYED_WEAPON_TO_PED(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_APPISTOL"), 1000, false);
				PED::SET_PED_RELATIONSHIP_GROUP_HASH(bodyG[i], GAMEPLAY::GET_HASH_KEY("player"));
				PED::SET_PED_AS_GROUP_MEMBER(bodyG[i], PLAYER::GET_PLAYER_GROUP(player));
				PED::SET_PED_CAN_RAGDOLL(bodyG[i], false);
				PED::SET_PED_SHOOT_RATE(bodyG[i], 8000);
				ENTITY::SET_ENTITY_HEALTH(bodyG[i], 250);
				WEAPON::SET_CURRENT_PED_WEAPON(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_CARBINERIFLE"), true);
			}
			PED::SET_GROUP_FORMATION(PLAYER::GET_PLAYER_GROUP(player), 3);
			setTextMsg(1000, "Your Tactical Crew joined, the Bison is available in the garage.");
			AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);

		}
		else { setTextMsg(500, "You don't have enough cash."); }

	}
	if (type == 2)
	{
		int val;
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&borVeh);
		DWORD exemp;
		exemp = GAMEPLAY::GET_HASH_KEY("oracle2");
		if (rand() % 3 == 0)	exemp = GAMEPLAY::GET_HASH_KEY("oracle");
		if (rand() % 3 == 1) exemp = GAMEPLAY::GET_HASH_KEY("sultan");
		DWORD gang1 = GAMEPLAY::GET_HASH_KEY("G_M_Y_KorLieut_01");
		STREAMING::REQUEST_MODEL(exemp);
		STREAMING::REQUEST_MODEL(gang1);
		WAIT(1000);
		for (int i = 0; i < 5; i++)
		{
			WEAPON::REMOVE_ALL_PED_WEAPONS(bodyG[i], true);
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&bodyG[i]);
		}

		// REMOVE 40000$
		bool can = false;
		char statNameFull[32];
		sprintf_s(statNameFull, "SP%d_TOTAL_CASH", pId);
		Hash hash = GAMEPLAY::GET_HASH_KEY(statNameFull);
		STATS::STAT_GET_INT(hash, &val, -1);
		if (val > 40000) {
			can = true;
			val -= 40000;
			STATS::STAT_SET_INT(hash, val, 1);
		}
		else { setTextMsg(500, "You don't have enough cash."); }
		if (can) {
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&payVeh);
			borVeh = VEHICLE::CREATE_VEHICLE(exemp, 135.3, -1050.86, 29.3, 159.6, 0, true);
			pimpMyRide(1, 0);
			VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(borVeh, false);
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&borVeh);

			// CREW
			STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("WEAPON_SMG"));
			STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("WEAPON_ASSAULTSHOTGUN"));
			STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("WEAPON_GUSENBERG"));
			WAIT(1000);
			PED::SET_PED_AS_GROUP_LEADER(playerPed, PLAYER::GET_PLAYER_GROUP(player));
			for (int i = 0; i < 3; i++)
			{
				bodyG[i] = PED::CREATE_PED(26, gang1, 150, -1054, 29.2, 160, false, true);
				/*if (i == 0) { WEAPON::GIVE_DELAYED_WEAPON_TO_PED(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_SMG"), 1000, false); WEAPON::SET_AMMO_IN_CLIP(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_SMG"), 120); }
				if (i == 1){ WEAPON::GIVE_DELAYED_WEAPON_TO_PED(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_MG"), 1000, false); WEAPON::SET_AMMO_IN_CLIP(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_MG"), 1500); }
				if (i == 2){ WEAPON::GIVE_DELAYED_WEAPON_TO_PED(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_GUSENBERG"), 1000, false); WEAPON::SET_AMMO_IN_CLIP(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_GUSENBERG"), 500); }*/
				// FULL SMG
				DWORD weap = GAMEPLAY::GET_HASH_KEY("WEAPON_MICROSMG");
				WEAPON::GIVE_DELAYED_WEAPON_TO_PED(bodyG[i], weap, 100, false);
				WAIT(110);
				WEAPON::SET_AMMO_IN_CLIP(bodyG[i], weap, 1000);
				WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(bodyG[i], weap, GAMEPLAY::GET_HASH_KEY("COMPONENT_AT_PI_FLSH"));
				if(rand() % 2 == 0) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(bodyG[i], weap, GAMEPLAY::GET_HASH_KEY("COMPONENT_AT_AR_SUPP_02"));
				WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(bodyG[i], weap, GAMEPLAY::GET_HASH_KEY("COMPONENT_AT_SCOPE_MACRO"));
				WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(bodyG[i], weap, GAMEPLAY::GET_HASH_KEY("COMPONENT_MICROSMG_CLIP_02"));
				WEAPON::SET_PED_WEAPON_TINT_INDEX(bodyG[i], weap, 6);
				WEAPON::SET_CURRENT_PED_WEAPON(bodyG[i], weap, true);
				// FULL SMG END
				WEAPON::GIVE_DELAYED_WEAPON_TO_PED(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_APPISTOL"), 1000, false);
				PED::SET_PED_RELATIONSHIP_GROUP_HASH(bodyG[i], GAMEPLAY::GET_HASH_KEY("player"));
				PED::SET_PED_AS_GROUP_MEMBER(bodyG[i], PLAYER::GET_PLAYER_GROUP(player));
				PED::SET_PED_CAN_RAGDOLL(bodyG[i], false);
				PED::SET_PED_SHOOT_RATE(bodyG[i], 60000);
				PED::SET_PED_ACCURACY(bodyG[i], 80);
				ENTITY::SET_ENTITY_HEALTH(bodyG[i], 750);
				PED::SET_PED_GROUP_MEMBER_PASSENGER_INDEX(bodyG[i], i);
				PED::SET_PED_COMBAT_ABILITY(bodyG[i], 2);
				PED::SET_PED_COMBAT_RANGE(bodyG[i], 2);
				PED::SET_PED_FIRING_PATTERN(bodyG[i], 0xC6EE6B4C);
				PED::SET_PED_SUFFERS_CRITICAL_HITS(bodyG[i], false);
				PED::SET_PED_MONEY(bodyG[i], 7500);
				PED::GIVE_PED_HELMET(bodyG[i], 1, 4096, -1);
				PED::SET_PED_SEEING_RANGE(bodyG[i], 500);
				//WEAPON::SET_CURRENT_PED_WEAPON(bodyG[i], GAMEPLAY::GET_HASH_KEY("WEAPON_CARBINERIFLE"), true);
			}
			setTextMsg(1000, "They will follow you everywhere, the sport vehicle is available in the garage.");
			AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);

		}
		else { setTextMsg(500, "You don't have enough cash."); }

		setTextMsg(1000, "~b~Info:~n~~w~Press B or Left + ~y~Y~w~ to fire 1 bodyguard.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);

	}
}

void setup_dealbreak()
{
	PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
	PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);
	PLAYER::SET_MAX_WANTED_LEVEL(0);
	cancelMsg();
	mission = true;
	int vehId, step;

	// spawning vehicles
	// speeder as coke vehicle
	DWORD vehH, riotH, swatH, weapH, weapA, weapS, dubH, robH, helH;
	Any helB;

	dubH = GAMEPLAY::GET_HASH_KEY("dubsta2");
	weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_CARBINERIFLE");
	weapA = GAMEPLAY::GET_HASH_KEY("COMPONENT_AT_AR_FLSH");
	weapS = GAMEPLAY::GET_HASH_KEY("COMPONENT_AT_AR_SUPP");
	vehH = GAMEPLAY::GET_HASH_KEY("speeder");
	riotH = GAMEPLAY::GET_HASH_KEY("riot");
	swatH = GAMEPLAY::GET_HASH_KEY("S_M_Y_Swat_01");
	robH = GAMEPLAY::GET_HASH_KEY("S_M_Y_Robber_01");
	helH = GAMEPLAY::GET_HASH_KEY("frogger2");

	STREAMING::REQUEST_MODEL(dubH);
	STREAMING::REQUEST_MODEL(vehH);
	STREAMING::REQUEST_MODEL(riotH);
	STREAMING::REQUEST_MODEL(swatH);
	STREAMING::REQUEST_MODEL(robH);
	STREAMING::REQUEST_MODEL(helH);


	while (!STREAMING::HAS_MODEL_LOADED(vehH) || !STREAMING::HAS_MODEL_LOADED(riotH) || !STREAMING::HAS_MODEL_LOADED(swatH) || !STREAMING::HAS_MODEL_LOADED(dubH) || !STREAMING::HAS_MODEL_LOADED(robH) || !STREAMING::HAS_MODEL_LOADED(helH))
		WAIT(100);

	scVeh[7] = VEHICLE::CREATE_VEHICLE(vehH, -1140, -1086, 0.23, 30, false, false);
	blips[1] = UI::ADD_BLIP_FOR_ENTITY(scVeh[7]);
	UI::SET_BLIP_COLOUR(blips[1], 3);


	while (!ENTITY::IS_ENTITY_AT_COORD(playerPed, -1130, -1068, 2.07, 100, 100, 100, false, true, false)) {
		if (ENTITY::IS_ENTITY_DEAD(playerPed) || ENTITY::IS_ENTITY_DEAD(scVeh[7])) { break; cleanupMissions(1); }
		if (GetAsyncKeyState('O') || CONTROLS::IS_CONTROL_JUST_PRESSED(0, 190) && CONTROLS::IS_CONTROL_JUST_PRESSED(0, 204)) { cleanupMissions(1); break; }
		message("Help your ~b~guys~w~ next to the ~b~boat~w~ full of heroin.");
		recruit_checks();
		WAIT(0);
	}

	// GOOD !! YOU KNOW WHERE IT IS STORED !
	if (mission) {
		scVeh[6] = VEHICLE::CREATE_VEHICLE(riotH, -1163, -1088, 2.1, 300, false, false);
		scVeh[8] = VEHICLE::CREATE_VEHICLE(riotH, -1179, -1097.6, 2.4, 237, false, false);
		scVeh[9] = VEHICLE::CREATE_VEHICLE(dubH, -1131.1, -1067, 2.07, 141, false, false);
		scVeh[10] = VEHICLE::CREATE_VEHICLE(dubH, -1131.62, -1087.63, 2.16, 209, false, false);
		scVeh[11] = VEHICLE::CREATE_VEHICLE(helH, -1156, -1106, 9.63 + 20.0f, 360, false, false);
		helB = UI::ADD_BLIP_FOR_ENTITY(scVeh[11]);
		UI::SET_BLIP_SPRITE(helB, 353);
		VEHICLE::SET_VEHICLE_LIVERY(scVeh[11], 0);
		VEHICLE::SET_VEHICLE_DOOR_OPEN(scVeh[10], 5, 0, 0);
		VEHICLE::SET_VEHICLE_SIREN(scVeh[6], true);
		VEHICLE::SET_VEHICLE_SIREN(scVeh[8], true);
		VEHICLE::SET_HELI_BLADES_FULL_SPEED(scVeh[11]);
		vehId = 6;
		step = 7;

		// ped id 6-22
		for (int i = -1; i < 9; i++)
		{
			scPed[i + step] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[vehId], 26, swatH, i, false, false);
			blips[i + step] = UI::ADD_BLIP_FOR_ENTITY(scPed[i + step]);
			UI::SET_BLIP_SCALE(blips[i + step], 0.75f);
			PED::SET_PED_AS_COP(scPed[i + 7], false);
			WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i + step], weapH, 100, false);
			WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(scPed[i + step], weapH, weapA);
			PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i + step], 0x783e3868);
			PED::SET_PED_AS_ENEMY(scPed[i + step], true);
			if (rand() % 2 == 1) WEAPON::GIVE_WEAPON_COMPONENT_TO_PED(scPed[i + step], weapH, weapS);
			if (i == 8 && vehId == 6) { vehId = 8; i = -1; step = 15; }
		}

		// ped id 23-31


		weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_MICROSMG");

		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, GAMEPLAY::GET_HASH_KEY("player"), 0x783e3868);
		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, 0x783e3868, GAMEPLAY::GET_HASH_KEY("player"));
		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, 0x90c7da60, 0x783e3868);
		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, 0x783e3868, 0x90c7da60);
		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(2, GAMEPLAY::GET_HASH_KEY("player"), 0x90c7da60);
		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(2, 0x90c7da60, GAMEPLAY::GET_HASH_KEY("player"));


		// dubsta 1
		step = 23;
		vehId = 9;
		for (int i = -1; i < 5; i++)
		{
			scPed[i + step] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[vehId], 26, robH, i, false, false);
			blips[i + step] = UI::ADD_BLIP_FOR_ENTITY(scPed[i + step]);
			UI::SET_BLIP_COLOUR(blips[i + step], 3);
			UI::SET_BLIP_SCALE(blips[i + step], 0.75f);
			PED::SET_PED_CAN_RAGDOLL(scPed[i + step], false);
			WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i + step], weapH, 100, false);
			PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i + step], 0x90c7da60);
			AI::TASK_LEAVE_VEHICLE(scPed[i + step], scVeh[vehId], 8);
			ENTITY::SET_ENTITY_HEALTH(scPed[i + step], 250);
		}

		// dubsta 2
		step = 27;
		vehId = 10;
		for (int i = -1; i < 5; i++)
		{
			scPed[i + step] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[vehId], 26, robH, i, false, false);
			blips[i + step] = UI::ADD_BLIP_FOR_ENTITY(scPed[i + step]);
			UI::SET_BLIP_COLOUR(blips[i + step], 3);
			UI::SET_BLIP_SCALE(blips[i + step], 0.75f);
			PED::SET_PED_CAN_RAGDOLL(scPed[i + step], false);
			WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i + step], weapH, 100, false);
			PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i + step], 0x90c7da60);
			AI::TASK_LEAVE_VEHICLE(scPed[i + step], scVeh[vehId], 8);
		}

		//swat heli
		step = 31;
		vehId = 11;
		weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_CARBINERIFLE");
		for (int i = -1; i < 5; i++)
		{
			PED::SET_PED_AS_COP(scPed[i + step], false);
			scPed[i + step] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[vehId], 26, swatH, i, false, false);
			PED::SET_PED_CAN_RAGDOLL(scPed[i + step], false);
			WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i + step], weapH, 100, false);
			PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i + step], 0x783e3868);
		}


		AI::TASK_COMBAT_PED(scPed[7], scPed[24], 0, 0);
		// is not yet there 

		while (!PED::IS_PED_IN_VEHICLE(playerPed, scVeh[7], true))
		{
			PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
			PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);
			if (ENTITY::IS_ENTITY_DEAD(playerPed) || ENTITY::IS_ENTITY_DEAD(scVeh[7])) { break; cleanupMissions(1); }
			if (GetAsyncKeyState('O') || CONTROLS::IS_CONTROL_JUST_PRESSED(0, 190) && CONTROLS::IS_CONTROL_JUST_PRESSED(0, 204)) { cleanupMissions(1); break; }
			for (int i = 6; i < 64; i++) { if (ENTITY::IS_ENTITY_DEAD(scPed[i])) UI::REMOVE_BLIP(&blips[i]); }
			if (ENTITY::IS_ENTITY_DEAD(scVeh[11])) UI::REMOVE_BLIP(&helB);
			message("Kill the ~r~enemies~w~ and get in the ~b~boat.");
			WAIT(0);
		}

		blips[3] = UI::ADD_BLIP_FOR_COORD(-767, -1425, 1.6);
		UI::SET_BLIP_COLOUR(blips[3], 17);

		while (!ENTITY::IS_ENTITY_AT_COORD(scVeh[7], -767, -1425, 1.60, 7, 7, 7, 0, 1, 0))
		{
			if (ENTITY::IS_ENTITY_DEAD(playerPed) || ENTITY::IS_ENTITY_DEAD(scVeh[7])) { break; cleanupMissions(1); }
			if (GetAsyncKeyState('O') || CONTROLS::IS_CONTROL_JUST_PRESSED(0, 190) && CONTROLS::IS_CONTROL_JUST_PRESSED(0, 204)) { cleanupMissions(1); break; }
			for (int i = 6; i < 64; i++) { if (ENTITY::IS_ENTITY_DEAD(scPed[i])) UI::REMOVE_BLIP(&blips[i]); }
			if (ENTITY::IS_ENTITY_DEAD(scVeh[11])) UI::REMOVE_BLIP(&helB);
			if (ENTITY::IS_ENTITY_AT_COORD(scVeh[7], -767, -1425, 1.60, 9, 9, 9, 0, 1, 0)) { cleanupMissions(0); break; }
			message("Get the ~b~boat~w~ to the ~y~Marina.");
			WAIT(0);
		}

	}

}


void bodyguards_leave()
{
	if ((GetAsyncKeyState('B') || (CONTROLS::IS_CONTROL_JUST_PRESSED(0, 189) && CONTROLS::IS_CONTROL_JUST_PRESSED(0, 204))) && press_B_tick == 0) {
		press_B_tick = 1;
		if (bodyguard > 4) bodyguard = 0;
		WEAPON::REMOVE_ALL_PED_WEAPONS(bodyG[bodyguard], true);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&bodyG[bodyguard]);
		bodyguard++;
	}
}

void generateRandomContracts_hot()
{
	int myRand;

	for (int i = 0; i < 3; i++) {
		std::ostringstream oss;
		myRand = rand() % 101;
		while (oldRand >= (myRand - 25) && oldRand <= (myRand + 25)) {
			myRand = rand() % 101;
			WAIT(0);
		}
		oldRand = myRand;
		// first contract
		if (myRand <= 24) {
			payout[i] = rand() % (50 - 20 + 1) + 20;
			contractPayout[i] = payout[i] * 1000;
			oss << "~r~Assassination ~g~$" << payout[i] << ",000";
			contracts[i] = oss.str();
			randContract[i] = 1;
		}
		if (myRand >= 25 && myRand <= 49) {
			payout[i] = rand() % (150 - 75 + 1) + 75;
			contractPayout[i] = payout[i] * 1000;
			oss << "~y~Mob Cleaning ~g~$" << payout[i] << ",000";
			contracts[i] = oss.str();
			randContract[i] = 2;
		}
		if (myRand >= 50 && myRand <= 74) {
			payout[i] = rand() % (410 - 275 + 1) + 275;
			contractPayout[i] = payout[i] * 1000;
			oss << "~b~VIP Protection ~g~$" << payout[i] << ",000";
			contracts[i] = oss.str();
			randContract[i] = 3;
		}
		if (myRand >= 75 && myRand <= 94) {
			payout[i] = rand() % (275 - 150 + 1) + 150;
			contractPayout[i] = payout[i] * 1000;
			oss << "~m~Deal Breaker ~g~$" << payout[i] << ",000";
			contracts[i] = oss.str();
			randContract[i] = 5;
		}
		if (myRand >= 95 && myRand <= 100) {
			if (totalMission > 4){
				contracts[i] = "~p~Big Score ~g~+$3,000,000";
				randContract[i] = 4;
			}
			else { i--; }
			
		}
	}

}

void generateRandomContracts_cold()
{
	// HERE
	contracts[0] = "Op. Nightfall ~w~(~g~$450,000~w~)";
	contracts[1] = "Op. Nightfall ~w~(~r~HC~w~) (~g~$950,000~w~)";
	contracts[2] = "Op. Black Thunder ~w~(~g~$640,000~w~)";
}

void redhouse_1()
{
	// CONTRACTS
	if (ENTITY::IS_ENTITY_AT_COORD(playerPed, 125.6, -1044.59, 29.3, 5.0f, 5.0f, 5.0f, 0, 1, 0) && !menuDisplayed[0]) {
		// 25 % chance assassination (1)
		// 25 % chance mob cleaning (2)
		// 20% chance deal breaker (5)
		// 25% protect contract (3)
		// 5% heist (4)
		menuDisplayed[0] = true;
		generateRandomContracts_hot();
		random_text = rand() % 6;
		// GENERATE 3 RANDOM CONTRACTS HERE
	}

	if (!ENTITY::IS_ENTITY_AT_COORD(playerPed, 125.6, -1044.59, 29.3, 5.0f, 5.0f, 5.0f, 0, 1, 0)) menuDisplayed[0] = false;
	
	if (menuDisplayed[0]) menuHandler();

	// RED HOUSE 1 CHECKS
	if (ENTITY::IS_ENTITY_AT_COORD(playerPed, 125.6, -1044.59, 29.3, 75.0f, 75.0f, 75.0f, 0, 1, 0) && !isAtRedHouse1) {
		setupScene_main(true);
		isAtRedHouse1 = true;
	}
	if (!ENTITY::IS_ENTITY_AT_COORD(playerPed, 125.6, -1044.59, 29.3, 75.0f, 75.0f, 75.0f, 0, 1, 0) && isAtRedHouse1) {
		setupScene_main(false);
		isAtRedHouse1 = false;
	}


}

void redhouse_2()
{
	if (ENTITY::IS_ENTITY_AT_COORD(playerPed, 50.69, 2787, 57.8, 5.0f, 5.0f, 5.0f, 0, 1, 0) && !menuDisplayed[2]) {
		menuDisplayed[2] = true;
		generateRandomContracts_cold();
		random_text = (rand() % 6) + 6;
		// GENERATE 3 RANDOM CONTRACTS HERE
	}

	if (!ENTITY::IS_ENTITY_AT_COORD(playerPed, 50.69, 2787, 57.8, 5.0f, 5.0f, 5.0f, 0, 1, 0)) menuDisplayed[2] = false;

	if (menuDisplayed[2]) menuHandler();

	// RED HOUSE 2 CHECKS
	if (ENTITY::IS_ENTITY_AT_COORD(playerPed, 48, 2784, 57, 150.0f, 150.0f, 150.0f, 0, 1, 0) && !isAtRedHouse2) {
		setupScene_blue(true);
		isAtRedHouse2 = true;
	}
	if (isAtRedHouse2){
		GRAPHICS::DRAW_LIGHT_WITH_RANGE(45.62, 2797.8, 61.0, 255, 255, 153, 30.0, 0.5);
		//GRAPHICS::DRAW_SPOT_LIGHT(45.62, 2797.8, 61.0, 40.83, 2800, 58, 255, 255, 153, 30.0, 1.0, 5.0, 1.0, 0);
		//GRAPHICS::DRAW_SPOT_LIGHT(45.62, 2797.8, 61.0, -5.0, 3.0, -5.0, 255, 255, 153, 8, 10.0, 0.2, 10.0, 1.0);

	
	}
	if (!ENTITY::IS_ENTITY_AT_COORD(playerPed, 48, 2784, 57, 150.0f, 150.0f, 150.0f, 0, 1, 0) && isAtRedHouse2) {
		setupScene_blue(false);
		isAtRedHouse2 = false;
	}
}

void assassinsChecks()
{
	message("Kill the ~r~target~w~.");
	if (PED::IS_PED_SHOOTING(scPed[6]) || PED::IS_PED_SHOOTING(scPed[7])) {
		if (rand() % 3 == 1 && callBack < 4) {
			callBack++;
			setTextMsg(2000, "~b~Info ~w~:~n~The target just called the hired protection, they'll be chasing you now!");
			AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
			sposition = ENTITY::GET_ENTITY_COORDS(playerPed, false);
			set_spawn_points();
			setAssassinBackup();
		}
	}
	if (ENTITY::IS_ENTITY_DEAD(scPed[6])) cleanupMissions(0);
}

void cleanChecks()
{
	if (ENTITY::IS_ENTITY_AT_COORD(playerPed, mobClean.x, mobClean.y, mobClean.z, 40.0f, 40.0f, 40.0f, false, true, false) && !joinedClean) { joinedClean = true;  UI::REMOVE_BLIP(&blips[1]); sposition = ENTITY::GET_ENTITY_COORDS(playerPed, false); mobCleanBlips(true); if (rand() % 3 == 1) setMobCleanBackup(); }
	if (!joinedClean) message("Go to the ~b~destination~w~.");
	if (joinedClean) message("Kill ~r~everyone~w~.");
	if (mobDead > 9) cleanupMissions(0);
	if (joinedClean){
		mobDead = 0;
		mobCleanBlips(false);
		for (int i = 6; i < 17; i++)
		{
			if (ENTITY::IS_ENTITY_DEAD(scPed[i])) {
				mobDead++; ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i]);
			}
		}
	}
}

void protectChecks()
{
	if (ENTITY::IS_ENTITY_AT_COORD(playerPed, mobProtect.x, mobProtect.y, mobProtect.z, 10.0f, 10.0f, 10.0f, false, true, false) && !joinedProtect) { joinedProtect = true;  UI::REMOVE_BLIP(&blips[1]); sposition = ENTITY::GET_ENTITY_COORDS(playerPed, false); PED::SET_PED_AS_GROUP_MEMBER(scPed[6], PLAYER::GET_PLAYER_GROUP(player)); }
	if (!joinedProtect) message("Get to the ~b~destination~w~ quickly.");
	if (joinedProtect && !finishedProtect) message("Kill the ~r~enemies~w~ and keep the ~b~contractor~w~ alive.");
	if (finishedProtect && !isDrivingAway) { while (!PED::IS_PED_IN_VEHICLE(playerPed, scVeh[6], true)) { WAIT(0); cleanupBlips(); if (ENTITY::IS_ENTITY_DEAD(playerPed) || ENTITY::IS_ENTITY_DEAD(scVeh[6])) break; message("Get in the ~b~contractor's~w~ vehicle."); } protectLoop(); message("Take the ~b~contractor~w~ back to the ~r~Red House~w~."); if (!blipSet) { UI::SET_BLIP_ROUTE(mainBlip[0], true); blipSet = true; } }
	if (finishedProtect) cleanupBlips();
	if (finishedProtect && isDrivingAway) message("Wait for the ~b~contractor~w~ to leave the area.");
	if (joinedProtect && !finishedProtect) protectLoop();
	if (mobDead > 36) finishedProtect = true;
	if (finishedProtect && ENTITY::IS_ENTITY_AT_COORD(scPed[6], 125.6, -1044.59, 29.24, 15, 15, 15, false, true, false)) {

		WEAPON::REMOVE_ALL_PED_WEAPONS(scPed[6], true);
		while (PED::IS_PED_IN_VEHICLE(playerPed, scVeh[6], false)) { WAIT(0); message("Get out of the vehicle."); }
		PED::REMOVE_PED_FROM_GROUP(scPed[6]);
		UI::SET_BLIP_ROUTE(mainBlip[0], false);
		blipSet = false;
		//AI::TASK_ENTER_VEHICLE(scPed[6], scVeh[6], 2000, -1, 30.0f, 0, 0);
		AI::TASK_SHUFFLE_TO_NEXT_VEHICLE_SEAT(-1, scPed[6]);
		WAIT(3000);
		AI::TASK_VEHICLE_DRIVE_TO_COORD(scPed[6], scVeh[6], 490, -3359, 6.5, 100, 1, ENTITY::GET_ENTITY_MODEL(scVeh[6]), 1, 5.0, -1);
		isDrivingAway = true;
	}
	if (isDrivingAway) { if (!ENTITY::IS_ENTITY_AT_COORD(scPed[6], position.x, position.y, position.z, 70.0f, 70.0f, 70.0f, 0, 1, 0)) cleanupMissions(0); }
	//if (selectedType == 3 && joinedProtect && mobDead > 60) { PED::REMOVE_PED_FROM_GROUP(scPed[6]); WEAPON::REMOVE_ALL_PED_WEAPONS(scPed[6], true); AI::TASK_ENTER_VEHICLE(scPed[6], scVeh[6], 100, -1, 10.0f, 0, 0); cleanupMissions(0); }
	if (joinedProtect && ENTITY::IS_ENTITY_DEAD(scPed[6])){ cleanupMissions(1); setTextMsg(2000, "~r~Mission failed ~w~:~n~I just heard the guy died ! How are people gonna trust me after this? Please be careful next time."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); }
	if (joinedProtect && ENTITY::IS_ENTITY_DEAD(scVeh[6])){ cleanupMissions(1); setTextMsg(2000, "~r~Mission failed ~w~:~n~The car is totaled ! This car is worth more than what the guy payed me!"); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); }
}

void check()
{
	if (mission) {
		// display objectives
		if (selectedType == 1) assassinsChecks();
		if (selectedType == 2) cleanChecks();
		if (selectedType == 3) protectChecks();

		if (selectedType > 1) {
			PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
			PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);
		}

	}
}

void heist_1()
{
	ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[1]);
	AUDIO::STOP_AUDIO_SCENE("JSH_1_GET_TO_ROOF");
	AUDIO::STOP_AUDIO_SCENE("EXILE_2_SNIPE_STAGE");
	AUDIO::STOP_AUDIO_SCENE("FAMILY_1_YACHT_ARRIVES");
	mission = true;
	//GAMEPLAY::SET_MISSION_FLAG(true);
	AUDIO::REQUEST_SCRIPT_AUDIO_BANK("FAMILY_1_CHASE_01", 0);
	AUDIO::REQUEST_MISSION_AUDIO_BANK("JWL_HEIST_SETUP", 0);
	//AUDIO::REQUEST_SCRIPT_AUDIO_BANK("CAR_THEFT_FINALE", 0);
	AUDIO::PREPARE_MUSIC_EVENT("FAM1_START");
	AUDIO::PREPARE_MUSIC_EVENT("JH1_START");



	for (int i = 0; i < 5; i++)
	{
		WEAPON::REMOVE_ALL_PED_WEAPONS(bodyG[i], true);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&bodyG[i]);
	}
	bool changedBlip = false;
	bool stage1 = false;
	bool stage2 = false;
	bool stage3 = false;
	bool stage4 = false;
	WAIT(0);
	setTextMsg(1000, "Alright, time for a big score now.");
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
	WAIT(5000);

	// AUDIO PLAYING (1)
	AUDIO::START_AUDIO_SCENE("JSH_1_GET_TO_ROOF");
	AUDIO::TRIGGER_MUSIC_EVENT("JH1_START");
	setTextMsg(1000, "This one is gonna get you pretty big money..");
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
	WAIT(4000);
	setTextMsg(1000, "This is not your regular contract, so you can't let me down on this one.");
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
	WAIT(3000);
	setTextMsg(1000, "Alright, I just got an Insurgent from some guys. You now have 2 choices.");
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
	WAIT(1000);
	setTextMsg(1000, "A : You drive the Insurgent all the way back here with the cops along the ride.");
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
	WAIT(4000);
	setTextMsg(1000, "B : You steal a Cargobob and someone will transport the Insurgent without the cops.");
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
	WAIT(1000);
	setTextMsg(1000, "Your choice.");
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);



	if (!stage1) {
		blips[1] = UI::ADD_BLIP_FOR_COORD(1531.1, 1715.5, 110);
		UI::SET_BLIP_COLOUR(blips[1], 3);
		UI::SET_BLIP_ROUTE(blips[1], true);
		UI::FLASH_MINIMAP_DISPLAY();

		DWORD vehH, helH, pedH, weapH, polH;
		vehH = GAMEPLAY::GET_HASH_KEY("insurgent");
		helH = GAMEPLAY::GET_HASH_KEY("cargobob2");
		polH = GAMEPLAY::GET_HASH_KEY("police");
		pedH = GAMEPLAY::GET_HASH_KEY("S_M_M_Security_01");
		STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("S_M_Y_Cop_01"));
		STREAMING::REQUEST_MODEL(polH);
		STREAMING::REQUEST_MODEL(pedH);
		STREAMING::REQUEST_MODEL(vehH);
		STREAMING::REQUEST_MODEL(helH);
		STREAMING::REQUEST_MODEL(GAMEPLAY::GET_HASH_KEY("dubsta"));
		WAIT(1000);
		while (!STREAMING::HAS_MODEL_LOADED(vehH) && !STREAMING::HAS_MODEL_LOADED(helH) && !STREAMING::HAS_MODEL_LOADED(pedH))
		{
			WAIT(0);
		}
		scVeh[6] = VEHICLE::CREATE_VEHICLE(vehH, 1531.1, 1715.5, 110, 147.0f, 0, true);
		scVeh[5] = VEHICLE::CREATE_VEHICLE(helH, 884.76, 2352, 52.68, 147.0f, 0, true);
		scVeh[7] = VEHICLE::CREATE_VEHICLE(GAMEPLAY::GET_HASH_KEY("dubsta"), 904, 2361, 51.73, 126.0f, 0, true);
		scVeh[8] = VEHICLE::CREATE_VEHICLE(GAMEPLAY::GET_HASH_KEY("dubsta"), 897, 2344, 51.93, 40.0f, 0, true);
		scVeh[9] = VEHICLE::CREATE_VEHICLE(polH, 1523, 1728, 110.1, 206, 0, true);

		// blips
		blips[2] = UI::ADD_BLIP_FOR_ENTITY(scVeh[5]);
		UI::SET_BLIP_SPRITE(blips[2], 353);
		//UI::SET_BLIP_AS_FRIENDLY(blips[2], true);
		UI::SET_BLIP_ROUTE(blips[2], true);

		scPed[6] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[7], 26, pedH, -1, false, false);
		scPed[7] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[7], 26, pedH, 0, false, false);
		scPed[8] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[7], 26, pedH, 1, false, false);
		scPed[9] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[7], 26, pedH, 2, false, false);
		scPed[10] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 26, pedH, -1, false, false);
		scPed[11] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 26, pedH, 0, false, false);
		scPed[12] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 26, pedH, 1, false, false);
		scPed[13] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[8], 26, pedH, 2, false, false);
		scPed[14] = PED::CREATE_PED(26, pedH, 880, 2362, 52, 290, false, true);
		scPed[15] = PED::CREATE_PED(26, pedH, 870, 2335, 53, 311, false, true);
		scPed[16] = PED::CREATE_PED(26, GAMEPLAY::GET_HASH_KEY("S_M_Y_Cop_01"), 1523, 1717, 110, 253, false, true);

		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, GAMEPLAY::GET_HASH_KEY("player"), 0x90c7da60);
		PED::SET_RELATIONSHIP_BETWEEN_GROUPS(5, 0x90c7da60, GAMEPLAY::GET_HASH_KEY("player"));
		PED::SET_PED_AS_COP(scPed[16], true);

		for (int i = 6; i < 17; i++)
		{
			if (rand() % 3 == 0) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL");
			if (rand() % 3 == 1) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_PUMPSHOTGUN");
			if (rand() % 3 == 2) weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_CARBINERIFLE");
			STREAMING::REQUEST_MODEL(weapH);
			PED::SET_PED_ACCURACY(scPed[i], 50);
			WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[i], weapH, 1000, false);
			WEAPON::SET_CURRENT_PED_WEAPON(scPed[i], weapH, true);
			PED::SET_PED_RELATIONSHIP_GROUP_HASH(scPed[i], 0x90c7da60);

		}
	}

	WAIT(2000);
	setTextMsg(1000, "The vehicle is at a farm near the desert. There will be cops... so careful.");
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
	WAIT(2000);
	setTextMsg(1000, "Bring it back at the Red House, we'll store it inside the Garage.");
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
	WAIT(3000);
	setTextMsg(1000, "Oh and one thing... Find a way to kill the cop without alerting everyone. Like, no guns, okay?");
	AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);

	bool cargobob = false;
	bool insurgent = false;
	bool cops = false;

	// PICKUP THE INSURGENT
	while (!stage1)
	{
		WAIT(0);
		if (PLAYER::GET_PLAYER_WANTED_LEVEL(player) > 0) { cops = true; }
		else { cops = false; }

		if (!PED::IS_PED_IN_VEHICLE(playerPed, scVeh[6], true) && !PED::IS_PED_IN_VEHICLE(playerPed, scVeh[5], true)) message("Find a way to bring the ~b~Insurgent~w~.");
		if (PED::IS_PED_IN_VEHICLE(playerPed, scVeh[5], true)) message("Take the Cargobob back to the ~y~Marina~w~.");
		if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, false)) { setTextMsg(1000, "I don't know how you managed to fuck up this..."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); break; cleanupMissions(1); }
		if (ENTITY::IS_ENTITY_DEAD(scVeh[6]) || ENTITY::IS_ENTITY_DEAD(scVeh[5])) { setTextMsg(1000, "How did you just managed to blow up the vehicle ? Good thing that thing was free!"); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); break; cleanupMissions(1); }
		if (PED::IS_PED_IN_VEHICLE(playerPed, scVeh[6], true)) {
			if (!cops) message("Get the ~b~Insurgent~w~ to the ~y~garage~w~.");
			if (cops) message("Lose the cops.");
			if (!changedBlip){
				insurgent = true;
				if (!PED::IS_PED_IN_VEHICLE(playerPed, scVeh[5], true)) {
					if (!PED::WAS_PED_KILLED_BY_STEALTH(scPed[16])) {
						AUDIO::START_AUDIO_SCENE("FAMILY_1_YACHT_ARRIVES");
						AUDIO::TRIGGER_MUSIC_EVENT("FAM1_START");
						PLAYER::SET_PLAYER_WANTED_LEVEL(player, 4, false);
						PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);
					}
				}
				UI::REMOVE_BLIP(&blips[1]);
				UI::REMOVE_BLIP(&blips[2]);
				blips[1] = UI::ADD_BLIP_FOR_COORD(135.3, -1050.86, 29.2);
				UI::SET_BLIP_COLOUR(blips[1], 17);
				UI::SET_BLIP_ROUTE(blips[1], true);
				changedBlip = true;
			}

		}
		if (PED::IS_PED_IN_VEHICLE(playerPed, scVeh[5], true) && !cargobob) { cargobob = true; setTextMsg(1000, "Cargogbob heh ? Take it back to the Marina, one of my guy will bring the Insurgent."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); }
		//if (VEHICLE::IS_VEHICLE_ATTACHED_TO_CARGOBOB(scVeh[6], scVeh[5]) && !cargobob) { cargobob = true; setTextMsg(1000, "Cargogbob heh ? Well as long as you drive the Insurgent to the garage..."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); }
		if (ENTITY::IS_ENTITY_AT_COORD(playerPed, 880, 2370, 52, 60, 60, 60, 0, 1, 0)) { UI::TOGGLE_STEALTH_RADAR(true); }
		else { UI::TOGGLE_STEALTH_RADAR(false); }
		if (ENTITY::IS_ENTITY_AT_COORD(scVeh[6], 135.3, -1050.86, 29.3, 3, 3, 3, 0, 1, 0) && !cops) { stage1 = true; UI::REMOVE_BLIP(&blips[1]); }
		if (ENTITY::IS_ENTITY_AT_COORD(scVeh[5], -735, -1455, 5, 8, 8, 3, 0, 1, 0) && !cops) { stage1 = true; UI::REMOVE_BLIP(&blips[1]); }
		if (PED::IS_PED_IN_VEHICLE(playerPed, scVeh[5], true) && !changedBlip) { changedBlip = true; UI::REMOVE_BLIP(&blips[1]); UI::REMOVE_BLIP(&blips[2]); blips[1] = UI::ADD_BLIP_FOR_COORD(-735, -1455, 5); UI::SET_BLIP_COLOUR(blips[1], 17); UI::SET_BLIP_ROUTE(blips[1], true); }

	}

	if (stage1) {
		stopMusic();
		AUDIO::TRIGGER_MUSIC_EVENT("JH1_START");
		for (int i = 5; i < 31; i++)
		{
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[i]);
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[i]);
		}
		if (insurgent) {
			setTextMsg(1000, "Great! Now we have our Insurgent. Now it's time for me to brief you on the attack.");
			AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		}
		else {
			setTextMsg(1000, "I'll charge on of my men to bring the Insurgent back with the Cargobob. Time for another mission.");
			AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		}

		PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
		PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);


		DWORD vehH, pedH;
		vehH = GAMEPLAY::GET_HASH_KEY("comet2");
		pedH = GAMEPLAY::GET_HASH_KEY("A_M_M_Business_01");
		STREAMING::REQUEST_MODEL(vehH);
		STREAMING::REQUEST_MODEL(pedH);
		WAIT(6000);

		scVeh[7] = VEHICLE::CREATE_VEHICLE(vehH, -128, -767, 33.37, 92.0f, 0, true);
		scPed[7] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[7], 3, pedH, -1, false, false);
		AI::TASK_VEHICLE_DRIVE_TO_COORD(scPed[7], scVeh[7], -631, -371, 34, 20, 1, ENTITY::GET_ENTITY_MODEL(scVeh[7]), 1, 5.0, -1);
		WAIT(2000);

		// camera work
		Any cam;
		cam = CAM::CREATE_CAMERA_WITH_PARAMS(0x19286a9, CAM::_0xA200EB1EE790F448().x, CAM::_0xA200EB1EE790F448().y, CAM::_0xA200EB1EE790F448().z, CAM::_0x5B4E4C817FCC2DFB(2).x, CAM::_0x5B4E4C817FCC2DFB(2).y, CAM::_0x5B4E4C817FCC2DFB(2).z, CAM::_0x80EC114669DAEFF4(), 1, 2);
		CAM::POINT_CAM_AT_ENTITY(cam, scVeh[7], 1.5073, 1.6429, 0.717, 1);
		CAM::ATTACH_CAM_TO_ENTITY(cam, scVeh[7], 3.582200050354, 3.3028998374938965, 1.5527, 1);
		CAM::SET_CAM_ACTIVE(cam, true);
		CAM::DO_SCREEN_FADE_OUT(1000);
		while (!CAM::IS_SCREEN_FADED_OUT()) {
			WAIT(0);
		}
		CAM::DO_SCREEN_FADE_IN(1000);
		CAM::RENDER_SCRIPT_CAMS(1, 0, 3000, 1, 0);
		Vector3 oldpos = ENTITY::GET_ENTITY_COORDS(playerPed, false);
		ENTITY::SET_ENTITY_COORDS(playerPed, -153.5, -730, 42, 1, false, false, false);
		while (!CAM::IS_SCREEN_FADED_IN()) {
			WAIT(0);
		}
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[6]);
		UI::DISPLAY_RADAR(0);
		//CAM::SET_GAMEPLAY_VEHICLE_HINT(scVeh[7], camStuff.x, camStuff.y, camStuff.z, 1, -1, 600, 600);
		CAM::_0xF8BDBF3D573049A1(0.95f);
		CONTROLS::DISABLE_ALL_CONTROL_ACTIONS(0);
		setTextMsg(1000, "Okay here is the target, he works for some security company that we don't really care about..");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(4000);
		setTextMsg(1000, "When a bank does a money transfer, they give a bank account filled with the same amount.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(4000);
		setTextMsg(1000, "Just in case if the Stockade gets robbed, the cash is marked and the money isn't lost.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(3000);
		setTextMsg(1000, "And the feds are gonna be all over that money... So yeah, not a good idea.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(4000);
		setTextMsg(1000, "We need the documents that contains info on wich Stockade is stored the cash.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(4000);
		setTextMsg(1000, "Kill him. Bring the documents and the car, and we'll continue.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(3000);
		ENTITY::SET_ENTITY_COORDS(playerPed, oldpos.x, oldpos.y, oldpos.z, 1, false, false, false);
		UI::DISPLAY_RADAR(1);
		CONTROLS::ENABLE_ALL_CONTROL_ACTIONS(0);
		CAM::DESTROY_CAM(cam, 0);
		CAM::RENDER_SCRIPT_CAMS(0, 0, 3000, 1, 0);
		//ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[7]);
		//ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[7]);
		stopMusic();

	}


	if (stage1){
		PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
		PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);
		setTextMsg(1000, "Target's gonna take the subway. Get there and wait until he arrives.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(3000);

		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[1]);
		ENTITY::DELETE_ENTITY(&scVeh[1]);
		ENTITY::SET_ENTITY_COORDS(scVeh[1], -1816, -1210, 14, 1, false, false, false);
		DWORD zent = GAMEPLAY::GET_HASH_KEY("zentorno");
		STREAMING::REQUEST_MODEL(zent);
		while (!STREAMING::HAS_MODEL_LOADED(zent))
			WAIT(100);

		if (insurgent) scVeh[1] = VEHICLE::CREATE_VEHICLE(zent, 138.63, -1060.6, 29.3, 319, 0, true);
		if (insurgent == false) scVeh[1] = VEHICLE::CREATE_VEHICLE(zent, -692, -1411, 5.1, 319, 0, true);


		blips[1] = UI::ADD_BLIP_FOR_ENTITY(scVeh[1]);
		UI::SET_BLIP_COLOUR(blips[1], 3);
		UI::FLASH_MINIMAP_DISPLAY();


		setTextMsg(1000, "Take my Zentorno. You'll get faster there.");
		AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(2000);
	}


	while (stage1 && !PED::IS_PED_IN_VEHICLE(playerPed, scVeh[1], true))
	{
		if (ENTITY::IS_ENTITY_DEAD(scVeh[1])) {
			cleanupMissions(1);
			setTextMsg(1000, "Really ?");
			AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
			break;

		}
		WAIT(0);
		message("Get in the ~b~Zentorno~w~");
	}

	if (stage1) {
		PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
		PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);
		UI::REMOVE_BLIP(&blips[1]);
		blips[1] = UI::ADD_BLIP_FOR_COORD(-809.2, 40.75, 48.55);
		UI::SET_BLIP_COLOUR(blips[1], 3);
		UI::SET_BLIP_ROUTE(blips[1], true);
	}

	changedBlip = false;
	bool joined = false;
	bool givenOrder = false;
	bool sentMsg = false;
	bool sentOrder = false;
	bool leftVeh = false;


	if (stage1) {
		OBJECT::CREATE_PICKUP_ROTATE(GAMEPLAY::GET_HASH_KEY("PICKUP_WEAPON_HEAVYSNIPER"), -807.406, 41.6, 47.76, -90.0, 0.0, 90, 0, 0, 2, 1, 0);
	}

	if (stage1) while (!stage2)
	{
		WAIT(0);
		if (!ENTITY::IS_ENTITY_AT_COORD(playerPed, -809.2, 40.75, 48.55, 4, 4, 4, 0, 1, 0) && !joined) { message("Get in ~b~position~w~ for the assassination."); }
		if (ENTITY::IS_ENTITY_AT_COORD(playerPed, -809.2, 40.75, 48.55, 4, 4, 4, 0, 1, 0) && !joined) {
			joined = true;
			setTextMsg(1000, "Alright, he'll be coming in a moment. He's gonna take the subway. Wait for him, don't shoot yet.");
			AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
			UI::REMOVE_BLIP(&blips[1]);
			CONTROLS::DISABLE_CONTROL_ACTION(0, 266, 1);
			CONTROLS::DISABLE_CONTROL_ACTION(0, 267, 1);
			CONTROLS::DISABLE_CONTROL_ACTION(0, 268, 1);
			CONTROLS::DISABLE_CONTROL_ACTION(0, 269, 1);
			CONTROLS::DISABLE_CONTROL_ACTION(0, 195, 1);
			CONTROLS::DISABLE_CONTROL_ACTION(0, 196, 1);

		}
		if (joined && !givenOrder) { AI::TASK_VEHICLE_DRIVE_TO_COORD(scPed[7], scVeh[7], -789.024, -95.215, 37.192, 50, 1, ENTITY::GET_ENTITY_MODEL(scVeh[7]), 1, 5.0, -1); givenOrder = true; blips[2] = UI::ADD_BLIP_FOR_ENTITY(scPed[7]); }
		if (joined && givenOrder && !ENTITY::IS_ENTITY_DEAD(scPed[7]) && !leftVeh) message("Wait for the ~r~target~w~ to arrive.");
		if (joined && givenOrder && ENTITY::IS_ENTITY_AT_COORD(scPed[7], -789.024, -95.215, 37.192, 4, 4, 4, 0, 1, 0) && !leftVeh) {
			Any sequence;
			AI::OPEN_SEQUENCE_TASK(&sequence);
			AI::TASK_VEHICLE_PARK(scPed[7], scVeh[7], -760.5, -80.215, 36.588, 295.0f, -1, 5.0f, 0);
			AI::TASK_LEAVE_VEHICLE(scPed[7], scVeh[7], true);
			AI::CLOSE_SEQUENCE_TASK(sequence);
			AI::TASK_PERFORM_SEQUENCE(scPed[7], sequence);
			setTextMsg(1000, "Take the shot now."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); WAIT(1000); AI::TASK_WANDER_STANDARD(scPed[7], 0x471c4000, 0);
			leftVeh = true;
			AI::TASK_GO_STRAIGHT_TO_COORD(scPed[7], -830, -103, 28.2, 1.0f, -1, 27.0f, 0.5f);
		}
		if (joined && givenOrder && !ENTITY::IS_ENTITY_DEAD(scPed[7]) && !PED::IS_PED_IN_VEHICLE(scPed[7], scVeh[7], 0)) message("Take the shot now.");
		if (ENTITY::IS_ENTITY_AT_COORD(scPed[7], -821, -108, 28.2, 5, 5, 5, 0, 1, 0)) {
			sentMsg = true; setTextMsg(1000, "Wow. After all of this, and you didn't manage to kill him ?"); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
			cleanupMissions(1);
			break;
		}
		if (ENTITY::IS_ENTITY_DEAD(scPed[7]) && !sentMsg) {
			CONTROLS::ENABLE_CONTROL_ACTION(0, 266, 1);
			CONTROLS::ENABLE_CONTROL_ACTION(0, 267, 1);
			CONTROLS::ENABLE_CONTROL_ACTION(0, 268, 1);
			CONTROLS::ENABLE_CONTROL_ACTION(0, 269, 1);
			CONTROLS::ENABLE_CONTROL_ACTION(0, 195, 1);
			CONTROLS::ENABLE_CONTROL_ACTION(0, 196, 1);
			sentMsg = true; setTextMsg(1000, "Great. Now take the Comet back to the garage. Files are inside."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); UI::REMOVE_BLIP(&blips[2]); ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[7]); blips[2] = UI::ADD_BLIP_FOR_ENTITY(scVeh[7]); UI::SET_BLIP_COLOUR(blips[2], 3);
		}
		if (sentMsg && !PED::IS_PED_IN_VEHICLE(playerPed, scVeh[7], true)) message("Get inside the ~b~Comet~w~.");
		if (sentMsg && PED::IS_PED_IN_VEHICLE(playerPed, scVeh[7], true) && !sentOrder) { sentOrder = true; setTextMsg(1000, "Alright, take it back to the garage now."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); }
		//if (sentMsg && PED::IS_PED_IN_VEHICLE(playerPed, scVeh[7], true)) message("Drive the ~b~Comet~w~ back to the Red House's garage.");
		if (PED::IS_PED_IN_VEHICLE(playerPed, scVeh[7], true)) { message("Get the ~b~Comet~w~ to the ~y~garage~w~."); if (!changedBlip){ changedBlip = true; UI::REMOVE_BLIP(&blips[1]); blips[1] = UI::ADD_BLIP_FOR_COORD(135.3, -1050.86, 29.2); UI::SET_BLIP_COLOUR(blips[1], 17); UI::SET_BLIP_ROUTE(blips[1], true); } }
		if (ENTITY::IS_ENTITY_AT_COORD(scVeh[7], 135.3, -1050.86, 29.3, 4, 4, 4, 0, 1, 0)) { stage2 = true; }
		if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, false)) { setTextMsg(1000, "You're really bad at this job..."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); break; cleanupMissions(1); }
		if (ENTITY::IS_ENTITY_DEAD(scVeh[7])) { setTextMsg(1000, "Great! Now we lost the files! How are we gonna know wich Stockade to attack now?"); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); break; cleanupMissions(1); }

	}

	joined = false;
	givenOrder = false;
	sentMsg = false;
	sentOrder = false;
	if (stage2){
		PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
		PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);
		UI::REMOVE_BLIP(&blips[1]);
		UI::REMOVE_BLIP(&blips[2]);
		VEHICLE::SET_VEHICLE_ENGINE_HEALTH(scVeh[7], 0.0f);
		WAIT(2000);
		setTextMsg(1000, "Excellent. Now we got the files. I'll text you the rest of the details."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(4000);
		setTextMsg(1000, "It's gonna me a little minute to decrypt these files. I'll send you a message when I'm done."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		// TODO :: change to 30000
		WAIT(5000);
		setTextMsg(1000, "Hmm... Okay, the money will be in Stockade #141, 224 and 162."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(4000);
		setTextMsg(1000, "I've just located the Stockades in the city. Get there and blow them up."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(3000);
		setTextMsg(1000, "I've recruited a guy to drive the Insurgent when things gets hot."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);

		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[7]);

		// spawn insurgent with Cheng

		DWORD vehH, pedH;
		vehH = GAMEPLAY::GET_HASH_KEY("insurgent");
		pedH = GAMEPLAY::GET_HASH_KEY("G_M_Y_KorLieut_01");
		STREAMING::REQUEST_MODEL(vehH);
		STREAMING::REQUEST_MODEL(pedH);
		while (!STREAMING::HAS_MODEL_LOADED(vehH) || !STREAMING::HAS_MODEL_LOADED(pedH)) WAIT(100);
		scVeh[7] = VEHICLE::CREATE_VEHICLE(vehH, -807, -792, 19, 176.0f, 0, true);
		scPed[7] = PED::CREATE_PED_INSIDE_VEHICLE(scVeh[7], 3, pedH, 0, false, false);

		vehH = GAMEPLAY::GET_HASH_KEY("stockade");
		STREAMING::REQUEST_MODEL(vehH);
		while (!STREAMING::HAS_MODEL_LOADED(vehH)) WAIT(100);
		// stockades
		scVeh[8] = VEHICLE::CREATE_VEHICLE(vehH, -692.5, -613, 24.94, 266.0f, 0, true);
		scVeh[9] = VEHICLE::CREATE_VEHICLE(vehH, -693, -618.5, 24.94, 266.0f, 0, true);
		scVeh[10] = VEHICLE::CREATE_VEHICLE(vehH, -692.8, -624.2, 24.94, 266.0f, 0, true);
		WAIT(3000);
		setTextMsg(1000, "The Insurgent is hidden in a little place downtown. Yeah I know, hidden, downtown... Trust me."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(3000);
		UI::FLASH_MINIMAP_DISPLAY();
		blips[1] = UI::ADD_BLIP_FOR_COORD(-807, -792, 19);
		UI::SET_BLIP_COLOUR(blips[1], 3);
		UI::SET_BLIP_ROUTE(blips[1], true);
	}

	if (stage2) while (!stage3)
	{
		WAIT(0);
		if (!PED::IS_PED_IN_VEHICLE(playerPed, scVeh[7], true) && !sentMsg) { message("Get in the ~b~Insurgent~w~."); }
		if (PED::IS_PED_IN_VEHICLE(playerPed, scVeh[7], true) && !joined){
			setTextMsg(1000, "Get in the Insurgent, and park it near the Stockade Center."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); joined = true;
			PED::SET_PED_AS_GROUP_LEADER(playerPed, PLAYER::GET_PLAYER_GROUP(player));
			PED::SET_PED_AS_GROUP_MEMBER(scPed[7], PLAYER::GET_PLAYER_GROUP(player));
			ENTITY::SET_ENTITY_INVINCIBLE(scPed[7], true);
			UI::REMOVE_BLIP(&blips[1]);
			UI::FLASH_MINIMAP_DISPLAY();
			blips[1] = UI::ADD_BLIP_FOR_COORD(-691, -576, 26);
			UI::SET_BLIP_COLOUR(blips[1], 3);
			UI::SET_BLIP_ROUTE(blips[1], true);
		}
		if (ENTITY::IS_ENTITY_AT_COORD(scVeh[7], -691, -576, 26, 6, 6, 6, 0, 1, 0) && joined && !sentMsg) {
			sentMsg = true; setTextMsg(1000, "Good. Now go blow up the 3 Stockades."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
			UI::REMOVE_BLIP(&blips[1]);
			blips[1] = UI::ADD_BLIP_FOR_ENTITY(scVeh[8]);
			blips[2] = UI::ADD_BLIP_FOR_ENTITY(scVeh[9]);
			blips[3] = UI::ADD_BLIP_FOR_ENTITY(scVeh[10]);
		}
		if ((!ENTITY::IS_ENTITY_DEAD(scVeh[8]) || !ENTITY::IS_ENTITY_DEAD(scVeh[9]) || !ENTITY::IS_ENTITY_DEAD(scVeh[10])) && sentMsg) {
			message("Blow up the ~r~Stockades~w~.");
			if (PED::IS_PED_IN_VEHICLE(playerPed, scVeh[7], true)) {
				AI::TASK_LEAVE_VEHICLE(playerPed, scVeh[7], 0x8);
				VEHICLE::SET_VEHICLE_FORWARD_SPEED(scVeh[7], 0.0f);
			}
		}
		for (int i = 1; i <= 3; i++) { if (ENTITY::IS_ENTITY_DEAD(scVeh[i + 7])) UI::REMOVE_BLIP(&blips[i]); }
		if ((ENTITY::IS_ENTITY_DEAD(scVeh[8]) && ENTITY::IS_ENTITY_DEAD(scVeh[9]) && ENTITY::IS_ENTITY_DEAD(scVeh[10])) && !sentOrder) {
			AUDIO::START_AUDIO_SCENE("FAMILY_1_YACHT_ARRIVES");
			AUDIO::TRIGGER_MUSIC_EVENT("FAM1_START");
			setTextMsg(1000, "Look at all that cash... Well cops are coming now."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
			WAIT(2000);
			sentOrder = true;
			CONTROLS::DISABLE_ALL_CONTROL_ACTIONS(0);
			DWORD weapH = GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL");
			STREAMING::REQUEST_MODEL(weapH);
			WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[7], weapH, 1000, false);
			//PED::REMOVE_PED_FROM_GROUP(scPed[7]);
			AI::TASK_ENTER_VEHICLE(scPed[7], scVeh[7], 15000, -1, 30, 0, 0);
			AI::TASK_ENTER_VEHICLE(playerPed, scVeh[7], 15000, 7, 30, 1, 0);
			WAIT(1000);
			while (!PED::IS_PED_IN_VEHICLE(playerPed, scVeh[7], false)) WAIT(0);
			WAIT(2000);
			if (VEHICLE::GET_PED_IN_VEHICLE_SEAT(scVeh[7], 7) != playerPed) PED::SET_PED_INTO_VEHICLE(playerPed, scVeh[7], 7);
			PLAYER::SET_PLAYER_WANTED_LEVEL(player, 5, false);
			PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);
			VEHICLE::SET_VEHICLE_HAS_STRONG_AXLES(scVeh[7], 1);
			VEHICLE::STEER_UNLOCK_BIAS(scVeh[7], 1);
			AI::CLEAR_PED_TASKS(scPed[7]);
			PED::SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(scPed[7], 1);
			//AI::TASK_SMART_FLEE_PED(scPed[7], playerPed, 200.0, 20000, 0, 0);
			AI::TASK_SMART_FLEE_COORD(scPed[7], ENTITY::GET_ENTITY_COORDS(scPed[7], true).x, ENTITY::GET_ENTITY_COORDS(scPed[7], true).y, ENTITY::GET_ENTITY_COORDS(scPed[7], true).z, 1000.0, -1, 1, 0);
			WAIT(1000);
			AI::TASK_VEHICLE_DRIVE_TO_COORD(scPed[7], scVeh[7], 153.3, -1050.86, 29.26, 200.0, 0, 0, 786599, 5.0, 2.0);
			PED::SET_PED_SPHERE_DEFENSIVE_AREA(scPed[7], ENTITY::GET_ENTITY_COORDS(scPed[7], 1).x, ENTITY::GET_ENTITY_COORDS(scPed[7], 1).y, ENTITY::GET_ENTITY_COORDS(scPed[7], 1).z, 10.0, 0, 0);
			//AI::TASK_VEHICLE_DRIVE_TO_COORD(scPed[7], scVeh[7], 135.3, -1050.86, 29.265299224853516, 50.0f, 0, ENTITY::GET_ENTITY_MODEL(scVeh[7]), 786603, 2.0, 4.0);
			VEHICLE::SET_VEHICLE_DOORS_LOCKED_FOR_ALL_PLAYERS(scVeh[7], true);
			CONTROLS::ENABLE_ALL_CONTROL_ACTIONS(0);
			PED::SET_AI_WEAPON_DAMAGE_MODIFIER(0.25);
			setTextMsg(1000, "I wish you good luck."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		}

		if (sentOrder && PED::IS_PED_IN_VEHICLE(playerPed, scVeh[7], true)) {
			WAIT(2500);
			AI::CLEAR_PED_TASKS(scPed[7]); AI::TASK_SMART_FLEE_COORD(scPed[7], ENTITY::GET_ENTITY_COORDS(scPed[7], true).x, ENTITY::GET_ENTITY_COORDS(scPed[7], true).y, ENTITY::GET_ENTITY_COORDS(scPed[7], true).z, 1000.0, -1, 1, 0);
			WAIT(2000);
			if (!PED::IS_PED_IN_VEHICLE(scPed[7], scVeh[7], true)) PED::SET_PED_INTO_VEHICLE(scPed[7], scVeh[7], -1);
			AI::TASK_VEHICLE_DRIVE_TO_COORD(scPed[7], scVeh[7], 153.3, -1050.86, 29.26, 200.0, 0, 0, 786599, 5.0, 2.0);
		}

		if (ENTITY::IS_ENTITY_DEAD(playerPed)) { setTextMsg(1000, "Damn it! And I thought you were the one!"); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); break; cleanupMissions(1); }
		if (ENTITY::IS_ENTITY_DEAD(scVeh[7])) { setTextMsg(1000, "How did the vehicle blow up!? It's supposed to be armored!"); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0); break; cleanupMissions(1); }
		if (ENTITY::IS_ENTITY_AT_COORD(scVeh[7], 135.3, -1050.86, 29.2, 70, 70, 70, 0, 1, 0)) { stage3 = true; CONTROLS::DISABLE_ALL_CONTROL_ACTIONS(0); }

	}


	if (stage3){
		stopMusic();
		WAIT(1000);
		PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
		PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);
		PLAYER::SET_MAX_WANTED_LEVEL(0);
		WAIT(2000);
		setTextMsg(1000, "Wow that was impressive. I think Weazel news is gonna have a blast tonight."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(2000);
		CONTROLS::ENABLE_ALL_CONTROL_ACTIONS(0);
		while (PED::IS_PED_IN_VEHICLE(playerPed, scVeh[7], true)) { WAIT(0); message("Get out of the Insurgent."); }
		AI::TASK_SMART_FLEE_COORD(scPed[7], ENTITY::GET_ENTITY_COORDS(scPed[7], true).x, ENTITY::GET_ENTITY_COORDS(scPed[7], true).y, ENTITY::GET_ENTITY_COORDS(scPed[7], true).z, 1000.0, -1, 1, 0);
		WAIT(5000);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[7]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[6]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scPed[7]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[8]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[9]);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&scVeh[10]);
		setTextMsg(1000, "Okay wait a little moment... I'll send you a message when I'll transfer the cash."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(20000);
		setTextMsg(1000, "Alright, time for a little clean up."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(2000);
		Vector3 oldpos;
		oldpos = ENTITY::GET_ENTITY_COORDS(playerPed, false);
		setTextMsg(1000, "The guy who drove the Insurgent is at Del Perro Beach."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(2000);
		DWORD pedH;
		pedH = GAMEPLAY::GET_HASH_KEY("G_M_Y_KorLieut_01");
		STREAMING::REQUEST_MODEL(pedH);
		while (!STREAMING::HAS_MODEL_LOADED(pedH)) WAIT(100);
		scPed[6] = PED::CREATE_PED(26, pedH, -1836, -1248, 13.02, 144, false, true);
		Any cam, cam2;
		cam = CAM::CREATE_CAMERA_WITH_PARAMS(0x19286a9, CAM::_0xA200EB1EE790F448().x, CAM::_0xA200EB1EE790F448().y, CAM::_0xA200EB1EE790F448().z, CAM::_0x5B4E4C817FCC2DFB(2).x, CAM::_0x5B4E4C817FCC2DFB(2).y, CAM::_0x5B4E4C817FCC2DFB(2).z, CAM::_0x80EC114669DAEFF4(), 1, 2);
		cam2 = CAM::CREATE_CAMERA_WITH_PARAMS(0x19286a9, CAM::_0xA200EB1EE790F448().x, CAM::_0xA200EB1EE790F448().y, CAM::_0xA200EB1EE790F448().z, CAM::_0x5B4E4C817FCC2DFB(2).x, CAM::_0x5B4E4C817FCC2DFB(2).y, CAM::_0x5B4E4C817FCC2DFB(2).z, CAM::_0x80EC114669DAEFF4(), 1, 2);
		//CAM::POINT_CAM_AT_ENTITY(cam, scPed[6], 1.5073, 1.6429, 0.717, 1);
		//CAM::ATTACH_CAM_TO_ENTITY(cam, scPed[6], 2.582200050354, 2.3028998374938965, 1.0527, 1);
		CAM::POINT_CAM_AT_COORD(cam2, -1789, -1203, 13.01);
		CAM::SET_CAM_COORD(cam, -1816.77, -1249.5, 15);
		CAM::SET_CAM_COORD(cam2, -1760, -1207, 18);
		CAM::SET_CAM_ACTIVE(cam, true);
		CAM::SET_CAM_ACTIVE_WITH_INTERP(cam2, cam, 35000, true, true);
		CAM::DO_SCREEN_FADE_OUT(1000);
		while (!CAM::IS_SCREEN_FADED_OUT()) {
			WAIT(0);
		}
		ENTITY::SET_ENTITY_COORDS(playerPed, -1816, -1210, 14, 1, false, false, false);
		CAM::DO_SCREEN_FADE_IN(1000);
		CAM::RENDER_SCRIPT_CAMS(1, 0, 3000, 1, 0);
		while (!CAM::IS_SCREEN_FADED_IN()) {
			WAIT(0);
		}
		PLAYER::SET_PLAYER_WANTED_LEVEL(player, 0, false);
		PLAYER::SET_PLAYER_WANTED_LEVEL_NOW(player, false);
		AI::TASK_WANDER_STANDARD(scPed[6], 0x471c4000, 0);
		UI::DISPLAY_RADAR(0);
		CAM::_0xF8BDBF3D573049A1(0.95f);
		setTextMsg(1000, "Don't worry, you've done enough."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(5000);
		setTextMsg(1000, "I just sent one guy do the job. I trust him."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(6000);
		setTextMsg(1000, "You know you did good out there. Money get's transfered very often to various banks."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(6000);
		scPed[7] = PED::CREATE_PED(26, pedH, -1793, -1197, 13.05, 144, false, true);
		WEAPON::GIVE_DELAYED_WEAPON_TO_PED(scPed[7], GAMEPLAY::GET_HASH_KEY("WEAPON_PISTOL"), 100, false);
		AI::TASK_COMBAT_PED(scPed[7], scPed[6], 0, 16);
		setTextMsg(1000, "The regular guys prefer to rob the bank, I prefer to steal the money from the bank account."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(4000);
		setTextMsg(1000, "I've got someone who works in a big bank, he takes a fair cut but atleast I get the money."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(4000);
		setTextMsg(1000, "And I don't even have to fire a single shot. Isn't this amazing?"); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(4000);
		setTextMsg(1000, "I'm gonna receive a call in a minute from the guy I sent that the target's dead."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(5000);
		setTextMsg(1000, "If you worry about the same happening to you, don't. You're far too valuable."); AUDIO::PLAY_SOUND_FRONTEND(-1, "Text_Arrive_Tone", "Phone_SoundSet_Default", 0);
		WAIT(4000);
		UI::DISPLAY_RADAR(1);
		CAM::DO_SCREEN_FADE_OUT(1000);
		while (!CAM::IS_SCREEN_FADED_OUT()) {
			WAIT(0);
		}
		ENTITY::SET_ENTITY_COORDS(playerPed, oldpos.x, oldpos.y, oldpos.z + 1.0f, 1, false, false, false);
		CAM::DO_SCREEN_FADE_IN(1000);
		CAM::RENDER_SCRIPT_CAMS(1, 0, 3000, 1, 0);
		while (!CAM::IS_SCREEN_FADED_IN()) {
			WAIT(0);
		}
		CAM::DESTROY_CAM(cam, 0);
		CAM::RENDER_SCRIPT_CAMS(0, 0, 3000, 1, 0);

		int pId;
		if (ENTITY::GET_ENTITY_MODEL(playerPed) == GAMEPLAY::GET_HASH_KEY("Player_Zero")) pId = 0;
		if (ENTITY::GET_ENTITY_MODEL(playerPed) == GAMEPLAY::GET_HASH_KEY("Player_One")) pId = 1;
		if (ENTITY::GET_ENTITY_MODEL(playerPed) == GAMEPLAY::GET_HASH_KEY("Player_Two")) pId = 2;

		int val;
		char statNameFull[32];
		sprintf_s(statNameFull, "SP%d_TOTAL_CASH", pId);
		Hash hash = GAMEPLAY::GET_HASH_KEY(statNameFull);
		STATS::STAT_GET_INT(hash, &val, -1);

		val += rand() % (5000000 - 3500000 + 1) + 3500000;
		STATS::STAT_SET_INT(hash, val, 1);
		PLAYER::SET_MAX_WANTED_LEVEL(5);
		cleanupMissions(1);
	}

	cleanupMissions(1);


}



void recruit_checks()
{

	// RECRUIT ZONE
	if (ENTITY::IS_ENTITY_AT_COORD(playerPed, 144.805, -1060.32, 29.2, 7.0f, 7.0f, 5.0f, 0, 1, 0) && !menuDisplayed[1]) {
		menu(true, 0);
		menuDisplayed[1] = true;
	}
	if (!ENTITY::IS_ENTITY_AT_COORD(playerPed, 144.805, -1060.32, 29.2, 7.0f, 7.0f, 5.0f, 0, 1, 0)) menuDisplayed[1] = false;

	// RECRUIT CHECKS
	if (menuDisplayed[1]) {
		// right button
		if (GetAsyncKeyState('K') || CONTROLS::IS_CONTROL_JUST_PRESSED(0, 190))  {
			recruit(0);
			WAIT(1000);
		}
		// left button
		if (GetAsyncKeyState('L') || CONTROLS::IS_CONTROL_JUST_PRESSED(0, 189)) {
			recruit(1);
			WAIT(1000);
		}
		// left stick
		if ((GetAsyncKeyState('J') || CONTROLS::IS_CONTROL_JUST_PRESSED(0, 209)) && !GetAsyncKeyState(0x10)) {
			recruit(2);
			WAIT(1000);
		}
	}


}

void menu_title(int font, char * text)
{
	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(0.8f, 0.8f);
	if(isAtRedHouse1) UI::SET_TEXT_COLOUR(255, 150, 255, 255);
	if(isAtRedHouse2) UI::SET_TEXT_COLOUR(150, 150, 255, 255);
	UI::SET_TEXT_WRAP(0.0, 1.0);
	UI::SET_TEXT_CENTRE(true);
	UI::SET_TEXT_DROPSHADOW(2, 2, 0, 0, 0);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(text);
	UI::_DRAW_TEXT(0.25, 0.375);

	UI::SET_TEXT_FONT(font);
	UI::SET_TEXT_SCALE(0.35f, 0.35f);
	UI::SET_TEXT_COLOUR(255, 255, 150, 255);
	UI::SET_TEXT_WRAP(0.0, 1.0);
	UI::SET_TEXT_CENTRE(true);
	UI::SET_TEXT_DROPSHADOW(2, 2, 0, 0, 0);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY("STRING");
	if (random_text == 0) UI::_ADD_TEXT_COMPONENT_STRING("I hope one of these contract interest you. - Shan");
	if (random_text == 1) UI::_ADD_TEXT_COMPONENT_STRING("Don't hesitate to come back, there is always work. - Shan");
	if (random_text == 2) UI::_ADD_TEXT_COMPONENT_STRING("With this kind of payout, even cops will start working for me. - Shan");
	if (random_text == 3) UI::_ADD_TEXT_COMPONENT_STRING("Still looking to buy that Golf thing ? - Shan");
	if (random_text == 4) UI::_ADD_TEXT_COMPONENT_STRING("Don't hesitate to recruit some men, they can help. - Shan");
	if (random_text == 5) UI::_ADD_TEXT_COMPONENT_STRING("Don't hesitate to recruit some men, they can help. - Shan");
	// JACK
	if (random_text == 6) UI::_ADD_TEXT_COMPONENT_STRING("Ain't no Shan here, can't just show up with an RPG. - Roy");
	if (random_text == 7) UI::_ADD_TEXT_COMPONENT_STRING("We can't just blow up stuff and call it a day. - Roy");
	if (random_text == 8) UI::_ADD_TEXT_COMPONENT_STRING("Shan loves paper, I prefer bank transfers. - Roy");
	if (random_text == 9) UI::_ADD_TEXT_COMPONENT_STRING("We only work at night here. - Roy");
	if (random_text == 10) UI::_ADD_TEXT_COMPONENT_STRING("I hope you can see in the dark, friend. - Roy");
	if (random_text == 11) UI::_ADD_TEXT_COMPONENT_STRING("I pay more, but no explosions alright?. - Roy");
	UI::_DRAW_TEXT(0.25, 0.585);
}

void menu_line()
{
	float scrp;
	if (uiSelected == 0) { scrp = 0.45; }
	if (uiSelected == 1) { scrp = 0.50; }
	if (uiSelected == 2) { scrp = 0.55; }
	GRAPHICS::DRAW_RECT(0.25, scrp, 0.25, 0.05, 0, 20, 120, 80);

	std::string tmp;

	UI::SET_TEXT_FONT(0);
	UI::SET_TEXT_SCALE(0.45f, 0.45f);
	if(isAtRedHouse1) UI::SET_TEXT_COLOUR(120, 0, 0, 220);
	if (isAtRedHouse2) UI::SET_TEXT_COLOUR(200, 200, 255, 220);
	UI::SET_TEXT_WRAP(0.0, 1.0);
	UI::SET_TEXT_CENTRE(true);
	UI::SET_TEXT_DROPSHADOW(2, 2, 0, 0, 0);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY("STRING");

	UI::_ADD_TEXT_COMPONENT_STRING((char*)contracts[0].c_str());
	UI::_DRAW_TEXT(0.25, 0.435);

	// line 2
	UI::SET_TEXT_FONT(0);
	UI::SET_TEXT_SCALE(0.45f, 0.45f);
	if (isAtRedHouse1) UI::SET_TEXT_COLOUR(120, 0, 0, 220);
	if (isAtRedHouse2) UI::SET_TEXT_COLOUR(200, 200, 255, 220);
	UI::SET_TEXT_WRAP(0.0, 1.0);
	UI::SET_TEXT_CENTRE(true);
	UI::SET_TEXT_DROPSHADOW(2, 2, 0, 0, 0);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY("STRING");

	tmp = "";
	tmp += " ~w~(~g~$50,000~w~)";

	UI::_ADD_TEXT_COMPONENT_STRING((char*)contracts[1].c_str());
	UI::_DRAW_TEXT(0.25, 0.485);

	// line 3
	UI::SET_TEXT_FONT(0);
	UI::SET_TEXT_SCALE(0.45f, 0.45f);
	if (isAtRedHouse1) UI::SET_TEXT_COLOUR(120, 0, 0, 220);
	if (isAtRedHouse2) UI::SET_TEXT_COLOUR(200, 200, 255, 220);
	UI::SET_TEXT_WRAP(0.0, 1.0);
	UI::SET_TEXT_CENTRE(true);
	UI::SET_TEXT_DROPSHADOW(2, 2, 0, 0, 0);
	UI::SET_TEXT_EDGE(1, 0, 0, 0, 205);
	UI::_SET_TEXT_ENTRY("STRING");

	tmp = "";
	tmp += " ~w~(~g~$50,000~w~)";

	UI::_ADD_TEXT_COMPONENT_STRING((char*)contracts[2].c_str());
	UI::_DRAW_TEXT(0.25, 0.535);
}

void menuHandler()
{
	// SHITTY ANIMATION
	if (rand() % 2 == 0)blue++;
	if (blue % 2 == 0 && rand() % 2 == 0) blue--;
	if (blue > 20) blue -= 10;


	// BASE RECTANGLE BACKGROUND
	GRAPHICS::DRAW_RECT(0.25, 0.5, 0.25, 0.25, blue / 2, 0, blue, 50);

	GRAPHICS::DRAW_RECT(0.25, 0.40, 0.25, 0.05, 0, 0, 50, 100);
	// SELECTED RECTANGLE 

	if(isAtRedHouse1) menu_title(1, "Contracts available");
	if (isAtRedHouse2) menu_title(1, "Covert operations");
	menu_line();

	if ((GetAsyncKeyState(VK_NUMPAD5) || CONTROLS::IS_CONTROL_JUST_PRESSED(0, 190)) && press_B_tick == 0)
	{
		fiveTick++;
		if (fiveTick > 4){
			fiveTick = 0;
			if (isAtRedHouse1){
				AUDIO::PLAY_SOUND_FRONTEND(-1, "HACKING_CLICK_BAD", 0, 1);
				press_B_tick = 0;
				if (randContract[uiSelected] == 1) { selectedType = 1; setup_Assassination(); }
				if (randContract[uiSelected] == 2) { selectedType = 2; setup_mobClean(); }
				if (randContract[uiSelected] == 3) { selectedType = 3; setup_protect(); }
				if (randContract[uiSelected] == 4) { selectedType = 4; heist_1(); }
				if (randContract[uiSelected] == 5) { selectedType = 5; setup_dealbreak(); }
			}
			if (isAtRedHouse2){
				press_B_tick = 0;
				if (uiSelected == 0) { hours = 23; setup_covertOps(); }
				if (uiSelected == 1) { hours = 23; hardcore = true; setup_covertOps(); }
				if (uiSelected == 2) { hours = 19; setup_covertOps(); }
			}
		}
	}

	if ((GetAsyncKeyState(VK_NUMPAD2) || CONTROLS::IS_CONTROL_JUST_PRESSED(0, 187)) && press_B_tick == 0)
	{
		AUDIO::PLAY_SOUND_FRONTEND(-1, "SELECT", "HUD_FRONTEND_DEFAULT_SOUNDSET", 1);
		press_B_tick = 52;
		uiSelected++;
		if (uiSelected > 2) uiSelected = 0;
	}
	

	if ((GetAsyncKeyState(VK_NUMPAD8) || CONTROLS::IS_CONTROL_JUST_PRESSED(0, 188)) && press_B_tick == 0)
	{
		AUDIO::PLAY_SOUND_FRONTEND(-1, "SELECT", "HUD_FRONTEND_DEFAULT_SOUNDSET", 1);
		press_B_tick = 52;
		uiSelected--;
		if (uiSelected > 2) uiSelected = 0;
		if (uiSelected < 0) uiSelected = 2;
	}


}

void update()
{

	player = PLAYER::PLAYER_ID();
	playerPed = PLAYER::PLAYER_PED_ID();
	position = ENTITY::GET_ENTITY_COORDS(playerPed, 1);
	//GAMEPLAY::SET_MISSION_FLAG(0);
	//randtick++;

	// MENU DISPLAYED IDs
	// 0 :: RED HOUSE 1 CONTRACT ZONE
	// 1 :: RED HOUSE 1 RECRUIT ZONE
	// 2 :: RED HOUSE 2 CONTRACT ZONE
	// 3 :: RED HOUSE 2 RECRUIT ZONE

	if (GetAsyncKeyState('J') && press_B_tick == 0)
	{
		std::string mystr;
		mystr = std::to_string(ENTITY::GET_ENTITY_HEADING(playerPed));
		setTextMsg(0, (char*)mystr.c_str());
		press_B_tick++;
	}



	// TICKS
	if (press_B_tick > 0) press_B_tick++;
	if (press_B_tick > 60) press_B_tick = 0;

	// AIR CHECKS
	isInAir = false;
	if (PED::IS_PED_IN_FLYING_VEHICLE(playerPed)) isInAir = true;
	if (ENTITY::IS_ENTITY_IN_WATER(playerPed)) isInAir = true;
	if (PED::IS_PED_IN_ANY_BOAT(playerPed)) isInAir = true;

	bodyguards_leave();
	check();
	recruit_checks();
	debug_func();

	if (!mission) {
		redhouse_1();
		redhouse_2();
	}
	else {
		// SAVING ON IF STATEMENTS - IF IN MISSION THEN PRESSING O WILL CANCEL MISSION
		// PRESSING O ONLY WHEN POSSIBLE. 
		// MADE SO THAT YOU CAN CANCEL REALLY QUICK WITHOUT IT LAGGING OUT.
		if (canCancel) { if (GetAsyncKeyState('O') || CONTROLS::IS_CONTROL_JUST_PRESSED(0, 190) && CONTROLS::IS_CONTROL_JUST_PRESSED(0, 204)) { cleanupMissions(1); } }
		// IS PLAYER DEAD?
		basic_check();
	}

	


}


void main()
{
	//WAIT(10000);
	while (true)
	{
		update();
		WAIT(0);
	}
}

void ScriptMain()
{
	srand(GetTickCount());
	main();
}

