#include "WPILib.h"
#include <vector>
using namespace std;

/**
 * This is a demo program showing the use of the RobotBase class.
 * The SimpleRobot class is the base of a robot application that will automatically call your
 * Autonomous and OperatorControl methods at the right time as controlled by the switches on
 * the driver station or the field controls.
 */ 
const double WATCHDOG_EXPIRATION = 0.1;

class RobotDemo : public SimpleRobot
{
	Jaguar *LeftDriveJaguar;
	Jaguar *RightDriveJaguar;
	Joystick *LeftJoystickDrive;
	Joystick *RightJoystickDrive;
	//DriverStationLCD *dsLCD;
	//DriverStation *ds;
	//DriverStationEnhancedIO *dsIO;
	AxisCamera *Camera;


	//Joystick stick; // only joystick

public:
	RobotDemo(void)
		//myRobot(6, 7)	// these must be initialized in the same order
				// as they are declared above.
	{
		LeftDriveJaguar = new Jaguar(6);
		RightDriveJaguar = new Jaguar(7);
		LeftJoystickDrive = new Joystick(1);
		RightJoystickDrive = new Joystick(2);
	}
	/**
	 * Drive left & right motors for 2 seconds then stop
	 */
	void Autonomous(void)
	{
		//dsLCD = DriverStationLCD::GetInstance();
		//dsLCD->Clear();

		/*ds = DriverStation::GetInstance();
		switch(ds->GetLocation())
		{
		case 1:
			//Execute Autonomous code #1
			dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Executing Autonomous 1");
			break;
		case 2:
			dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Executing Autonomous 2");
			//Execute Autonomous code #2
			break;
		case 3:
			dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Executing Autonomous 3");
			//Execute Autonomous code #3
			break;
		}*/
		//dsLCD->UpdateLCD();
		DriveAutonomously(1.0, 0.0);
		Wait(1); 		
		delete LeftDriveJaguar;
		delete RightDriveJaguar;
		delete LeftJoystickDrive;
		delete RightJoystickDrive;
		//delete dsLCD;
		//delete ds;
	}

	/**
	 * Runs the motors with arcade steering. 
	 */
	void OperatorControl(void)
	{
		HSLImage *Himage;
		Threshold targetThreshold(247, 255, 60, 140, 10, 50);
		BinaryImage *matchingPixels;
		vector<ParticleAnalysisReport> *pReport;

		AxisCamera &mycam = AxisCamera::GetInstance("10.15.10.11");

		mycam.WriteResolution(AxisCamera::kResolution_640x480);
		mycam.WriteCompression(20);
		mycam.WriteBrightness(25);
         
		//dsLCD = DriverStationLCD::GetInstance();
		//dsLCD->Clear();

		while(IsOperatorControl())
		{
			UpdateJoystickDrive();
			if (mycam.IsFreshImage())
			{
				Himage = mycam.GetImage();

				matchingPixels = Himage->ThresholdHSL(targetThreshold);
				pReport = matchingPixels->GetOrderedParticleAnalysisReports();

				for (unsigned int i = 0; i < pReport->size(); i++)
				{
					printf("Index: %d X Center: %d Y Center: %d \n", i, (*pReport)[i].center_mass_x, (*pReport)[i].center_mass_y);
				}
			}
			Wait(0.005);
		}	
		delete Himage;
		delete matchingPixels;
		delete pReport;
	}
	
	void DriveAutonomously(float Speed, float Curve)
	{
		if(Curve < 0) //Right Turn
		{
			LeftDriveJaguar->Set(Speed);
			RightDriveJaguar->Set(Speed - (Curve * Speed));
		}
		if(Curve < 0) //Left Turn
		{
			LeftDriveJaguar->Set(Speed - (Curve * Speed));
			RightDriveJaguar->Set(Speed);
		}
		if(Curve == 0) //Straight
		{
			LeftDriveJaguar->Set(Speed);
			RightDriveJaguar->Set(Speed);
		}
	}
	void UpdateJoystickDrive() //Gets the value of the left and right joystick and sets it to the power of the left and right jaguars
	{	
		LeftDriveJaguar->Set(LeftJoystickDrive->GetY());
		RightDriveJaguar->Set(RightJoystickDrive->GetY());
	}
	
	~RobotDemo()
	{
		delete LeftDriveJaguar;
		delete RightDriveJaguar;
	}
};

START_ROBOT_CLASS(RobotDemo);
