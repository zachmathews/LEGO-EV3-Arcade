#include "EV3_FileIO.c"

//various constants
const int SCREEN_WIDTH = 178;
const int SCREEN_HEIGHT = 128;
const int INACTIVE_TIME = 30000;

//player values
const int PLAYER_SIZE = 6;
const float PLAYER_SPEED = 0.2;
const int MAX_AMMO = 10;
const int PLAYER_DAMAGE = 1;

//enemy values
const int ENEMY_COUNT = 3;
const int ENEMY_M_HP = 2;
const int ENEMY_SPEED = 2;
const int ENEMY_SIZE = 10;

//struct
typedef struct {
	int x;
	int y;
	int health;
}enemy;

struct enemy enemies[ENEMY_COUNT];

void set_joystick_zero()
{
	eraseDisplay();

	displayCenteredTextLine(5, "Set joystick to the upright");
	displayCenteredTextLine(6, "position and press enter");
	displayCenteredTextLine(7, "when it's there.");

	while(!getButtonPress(buttonEnter))
	{}
	while(getButtonPress(buttonEnter))
	{}

	nMotorEncoder[motorB] = nMotorEncoder[motorC] = 0;
	eraseDisplay();
}

void joystick_input(int &player_x, int &player_y)
{
	int y = nMotorEncoder[motorB];
	int x = nMotorEncoder[motorC];

	if (abs(x) > 10)
	{
		player_x = player_x - x * PLAYER_SPEED;
	}

	if (abs(y) > 10)
	{
		player_y = player_y + y * PLAYER_SPEED;
	}
}

//play around with these values during testing
void stay_in_bounds(int &player_x, int &player_y)
{
	if (player_x > SCREEN_WIDTH)
	{
		player_x = SCREEN_WIDTH;
	}
	else if (player_x < 0)
	{
		player_x = 0;
	}

	if (player_y > SCREEN_HEIGHT)
	{
		player_y = SCREEN_HEIGHT;
	}
	else if (player_y < 0)
	{
		player_y = 0;
	}
}

void display_player(int player_x, int player_y, float barrel_angle)
{
	drawLine(player_x, player_y, player_x + (PLAYER_SIZE * 2 * cosDegrees(barrel_angle)), player_y + (PLAYER_SIZE * 2 * sinDegrees(barrel_angle)));
	fillCircle(player_x, player_y, PLAYER_SIZE);

	//#define drawCircle(Left, Top, Diameter) drawEllipse(Left - Diameter, Top + Diameter, Left + Diameter, Top - Diameter)
	//#define fillCircle(Left, Top, Diameter) fillEllipse(Left - Diameter, Top + Diameter, Left + Diameter, Top - Diameter)
}

void rotate_barrel(float &barrel_angle)
{
	barrel_angle = barrel_angle + SensorValue[S1] * -0.5;

	if (barrel_angle > 180)
	{
		barrel_angle = 180;
	}

	if (barrel_angle < 0)
	{
		barrel_angle = 0;
	}
}

bool reload(int &ammo, bool isRed)
{
	if (SensorValue[S3] != (int)colorRed)
	{
		if (isRed)
		{
			ammo = MAX_AMMO;
		}
		return false;
	}
	return true;
}

bool shoot(int &ammo, bool canShoot, int player_x, int player_y, float barrel_angle)
{
	if(SensorValue[S2] == 1)
	{
		if(ammo > 0 && canShoot)
		{
			ammo -= 1;
			float slope_of_shot = sinDegrees(barrel_angle) / (cosDegrees(barrel_angle) + 0.000001); //that's there so it's never y/0
			for(int count = 0; count < ENEMY_COUNT; count++)
			{
				if(enemies[count].y > player_y - ENEMY_SIZE)
				{
					if(fabs(slope_of_shot * (enemies[count].x - player_x) - enemies[count].y + player_y) / sqrt(pow(slope_of_shot, 2) + 1) <= ENEMY_SIZE)
					{
						enemies[count].health -= PLAYER_DAMAGE;
					}
				}

				drawLine(player_x, player_y, player_x + (200 * cosDegrees(barrel_angle)), player_y + (200 * sinDegrees(barrel_angle)));
			}
		}
		return false;
	}
	return true;
}

void reset_enemy(struct enemy &enem)
{
	enem.health = ENEMY_M_HP;
	enem.y = SCREEN_HEIGHT + 10; //workshop + 10
	enem.x = random(SCREEN_WIDTH); //workshop values
}

void generate_enemies()
{
	for(int count = 0; count < ENEMY_COUNT; count++)
	{
		reset_enemy(enemies[count]);
	}
}

bool enemy_update(int &points, int player_x, int player_y, int deg_to_turn)
{
	for(int count = 0; count < ENEMY_COUNT; count++)
	{
		if (enemies[count].health <= 0)
		{
			reset_enemy(enemies[count]);
			motor[motorA] = 50;
			while ((nMotorEncoder[motorA] < deg_to_turn) && (nMotorEncoder[motorA] < 180))
			{}
			motor[motorA] = 0;

			points += 40;
		}
		else
		{
			if (enemies[count].y < -10) //workshop -10
			{
				reset_enemy(enemies[count]);
			}
			else
			{
				enemies[count].y -= ENEMY_SPEED * (count + 1) / 3;
				enemies[count].x += random(51) - 25; //workshop values

				if(enemies[count].x > SCREEN_WIDTH)
				{
					enemies[count].x = SCREEN_WIDTH;
				}
				else if(enemies[count].x < 0)
				{
					enemies[count].x = 0;
				}

				drawCircle(enemies[count].x, enemies[count].y, ENEMY_SIZE);
			}
		}

		if (sqrt(pow(enemies[count].x - player_x, 2)+ pow(enemies[count].y - player_y, 2)) <= PLAYER_SIZE + ENEMY_SIZE) //not sure if rad or di
		{
			return true;
		}
	}
	return false;
}

void enter_initials(int points)
{
	setMotorBrakeMode(motorD, motorCoast);

	//change the file here
	string name[15];
	int scores[15];

	//get high scores
	TFileHandle fin;
	openReadPC(fin, "high scores.txt");
	for(int count = 0; count < 15; count++)
	{
		readTextPC(fin, name[count]);
		readIntPC(fin, scores[count]);
	}
	closeFilePC(fin);

	if (points > scores[14])
	{
		char alphabet[27] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
			'W', 'X', 'Y', 'Z', ' '};
		char initials[3] = {'_', '_', '_'};

		for(int count = 0; count < 3; count++)
		{
			nMotorEncoder[motorD] = 0;
			while(!getButtonPress(buttonEnter))
			{
				int rotation = nMotorEncoder[motorD];
				while(rotation < 0 || rotation > 720)
				{
					if (rotation < 0)
					{
						rotation += 720;
					}
					else
					{
						rotation -= 720;
					}
				}

				initials[count] = alphabet[rotation * 13 / 360];
				displayCenteredBigTextLine(5, "Initials");
				displayCenteredBigTextLine(7, "%c %c %c", initials[0], initials[1], initials[2]);
			}
			while(getButtonPress(buttonEnter))
			{}
		}

		scores[14] = points;
		sprintf(name[14], "%c%c%c", initials[0], initials[1], initials[2]);

		for(int count = 14; count > 0; count--)
		{
			if(scores[count] > scores[count - 1])
			{
				int temp_s =  scores[count];
				scores[count] = scores[count - 1];
				scores[count - 1] = temp_s;

				string temp_n = name[count];
				name[count] = name[count - 1];
				name[count - 1] = temp_n;
			}
		}

		TFileHandle fout;
		openWritePC(fout, "high scores.txt"); //maybe use bool?

		for(int count = 0; count < 15; count++)
		{
			writeTextPC(fout, name[count]);
			writeEndlPC(fout);
			writeLongPC(fout, scores[count]);
			writeEndlPC(fout);
		}
		closeFilePC(fout);
	}
	wait1Msec(1500);
	eraseDisplay();

}

void display_high_scores()
{
	string initials[15];
	int scores[15];

	//get high scores
	TFileHandle fin;
	openReadPC(fin, "high scores.txt");

	for(int count = 0; count < 15; count++)
	{
		readTextPC(fin, initials[count]);
		readIntPC(fin, scores[count]);
	}

	closeFilePC(fin);

	displayCenteredTextLine(1, "HIGH SCORES");
	for(int count = 0; count < 15; count++)
	{
		displayCenteredTextLine(count + 2, "%s - %d", initials[count], scores[count]);
	}

	while(!getButtonPress(buttonEnter))
	{}
	while(!getButtonPress(buttonEnter))
	{}
	eraseDisplay();
}

int get_highest_score()
{
	TFileHandle fin;
	openReadPC(fin, "high scores.txt");

	string name = "Bruh";
	int highest_score = 0;

	readTextPC(fin, name);
	readIntPC(fin, highest_score);

	closeFilePC(fin);

	return highest_score;
}

void how_to_play()
{
	while(getButtonPress(buttonDown))
	{}

	displayBigTextLine(1, "Input   Action");

	displayString(3,"Joystick     Move Around");
	displayString(5,"Rotate Gun   Change Direction");
	displayString(7,"Touch        Shoot Enemy");

	displayString(9,"The aim is to avoid the");
	displayString(11,"circles while shooting them");
	displayString(13,"to get points. You die when");
	displayString(15,"a circle hits you. Good luck!");

	while(!getButtonPress(buttonDown))
	{}
	while(getButtonPress(buttonDown))
	{}
}

void death_screen(int &points)
{
	displayCenteredBigTextLine(3, "YOU DIED!");
	wait1Msec(3000);
	displayCenteredBigTextLine(5, "POINTS SCORED:");
	wait1Msec(1500);
	displayCenteredBigTextLine(7, "%d", points);
	wait1Msec(5000);
	eraseDisplay();
}

void check_if_paused()
{
	if(getButtonPress(buttonEnter))
	{
		while(getButtonPress(buttonEnter))
		{}

		while(!getButtonPress(buttonEnter))
		{
			eraseDisplay();
			displayCenteredBigTextLine(7, "Paused");
			displayCenteredTextLine(9, "Reset Joystick (Left Button)");

			if(getButtonPress(buttonLeft))
			{
				while(getButtonPress(buttonLeft))
				{}
				set_joystick_zero();
			}
		}
		while(getButtonPress(buttonEnter))
		{}
		eraseDisplay();
	}
}

int inactivity()
{
	if(SensorValue[S2] == 1 || getButtonPress(buttonEnter))
	{
		clearTimer(T1);
	}
	return getTimerValue(T1);
}

void configure_inputs()
{
	//set up gyro
	SensorType[S1] = sensorEV3_Gyro;
	wait1Msec(50);
	SensorMode[S1] = modeEV3Gyro_Calibration;
	wait1Msec(50);
	SensorMode[S1] = modeEV3Gyro_Rate;
	wait1Msec(50);

	//set up touch sensor
	SensorType[S2] = sensorEV3_Touch;
	wait1Msec(50);

	//set up color sensor
	SensorType[S3] = sensorEV3_Color;
	wait1Msec(50);
	SensorMode[S3] = modeEV3Color_Color;
	wait1Msec(100);

	//set up joystick
	setMotorBrakeMode(motorB, motorCoast);
	setMotorBrakeMode(motorC, motorCoast);
}


/*	This is a sample Program
Use File->Load C Prog to
load a different Program
*/

task main()
{
	configure_inputs();
	setMotorBrakeMode(motorA, motorCoast); //tell them to set the meter back
	time1[T1] = 0;
	while(inactivity() < INACTIVE_TIME)
	{
		eraseDisplay();
		displayCenteredBigTextLine(3, "Ball with Gun");
		displayCenteredBigTextLine(8, "Play Game");
		displayCenteredBigTextLine(11, "How To Play");

		while(!getButtonPress(buttonUp) && !getButtonPress(buttonDown))
		{}
		eraseDisplay();

		setMotorBrakeMode(motorA, motorBrake); //perhaps a function would be better

		if(getButtonPress(buttonDown))
		{
			how_to_play();
		}
		else
		{
			while(getButtonPress(buttonUp))
			{}
			eraseDisplay();

			set_joystick_zero();

			//player values
			int points = 0;
			int ammo = MAX_AMMO;
			int player_x = SCREEN_WIDTH/2;
			int player_y = SCREEN_HEIGHT/2;
			float barrel_angle = 90;

			//high score stuff
			nMotorEncoder[motorA] = 0;
			displayTextLine(4, "%d", get_highest_score());
			int points_deg = (10 / (get_highest_score() + 0.001)) * 180; //fix this

			generate_enemies();

			bool isRed = false;
			bool canShoot = true;
			bool collision = false;
			while(!collision && inactivity() < INACTIVE_TIME)
			{
				joystick_input(player_x, player_y);
				rotate_barrel(barrel_angle);
				stay_in_bounds(player_x, player_y);
				isRed = reload(ammo, isRed);
				displayBigTextLine(1, "AMMO: %d", ammo);
				collision = enemy_update(points, player_x, player_y, points_deg);
				canShoot = shoot(ammo, canShoot, player_x, player_y, barrel_angle);
				display_player(player_x, player_y, barrel_angle);
				wait1Msec(333);
				eraseDisplay();
				check_if_paused();
			}

			motor[motorA] = -50;
			while (nMotorEncoder[motorA] > 0)
			{}
			motor[motorA] = 0;

			if(inactivity() < INACTIVE_TIME)
			{
				//end of round screen
				death_screen(points);

				//scores
				enter_initials(points);
				display_high_scores();
			}
		}
	}
}
