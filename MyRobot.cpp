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
	//RobotDrive *myRobot; // robot drive system
	Jaguar *LeftDriveJaguar;
	Jaguar *RightDriveJaguar;
	Watchdog *Saftey;
	Joystick *LeftDrive;
	Joystick *RightDrive;
	DriverStationLCD *dsLCD;
	DriverStation *ds;
	DriverStationEnhancedIO *dsIO;
	AxisCamera *Camera;
	
	
	//Joystick stick; // only joystick

public:
	RobotDemo(void)
		//myRobot(6, 7)	// these must be initialized in the same order
				// as they are declared above.
	{
		//myRobot = new RobotDrive(6,7);
		LeftDriveJaguar = new Jaguar(6);
		RightDriveJaguar = new Jaguar(7);
		Saftey = new Watchdog;
		LeftDrive = new Joystick(1);
		RightDrive = new Joystick(2);
		//dsIO = new DriverStationEnhancedIO;
		//dsLCD = new DriverStationLCD;
		//myRobot->SetExpiration(0.1);
	}
	/**
	 * Drive left & right motors for 2 seconds then stop
	 */
	void Autonomous(void)
	{
		Saftey->SetEnabled(false);
		//myRobot->SetSafetyEnabled(false);
		//myRobot->Drive(0.5, 0.0); 	// drive forwards half speed
		dsLCD = DriverStationLCD::GetInstance();
		dsLCD->Clear();
		//dsLCD->Printf(DriverStationLCD::kUser_Line1, 1, "Hello World" );
		//dsLCD->UpdateLCD();
		
		Wait(0.5);
		
		ds = DriverStation::GetInstance();
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
			
		}
		dsLCD->UpdateLCD();
		
		Saftey->SetEnabled(false);
		Wait(0.5); 				//    for 2 seconds
		delete LeftDriveJaguar;
		delete RightDriveJaguar;
		delete Saftey;
		delete dsLCD;
		delete ds;
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
		
		//myRobot->SetSafetyEnabled(true);
		Saftey->SetEnabled(false);
		AxisCamera &mycam = AxisCamera::GetInstance("10.15.10.11");
		
		mycam.WriteResolution(AxisCamera::kResolution_640x480);
		mycam.WriteCompression(20);
		mycam.WriteBrightness(25);
		
         
		dsLCD = DriverStationLCD::GetInstance();
		dsLCD->Clear();
		
		while(IsOperatorControl())
		{
			UpdateDrive();
			Wait(0.005);
			if (mycam.IsFreshImage())
				{
					Himage = mycam.GetImage();
					
					matchingPixels = Himage->ThresholdHSL(targetThreshold);
					pReport = matchingPixels->GetOrderedParticleAnalysisReports();
					
					for (unsigned int i = 0; i < pReport->size(); i++)
					{
						printf("Index: %d X Center: %d Y Center: %d \n", i, (*pReport)[i].center_mass_x, (*pReport)[i].center_mass_y);
						
					}
					
					delete Himage;
					delete matchingPixels;
					delete pReport;
				}
		}	
	}
	
	void UpdateDrive() //Gets the value of the left and right joystick and sets it to the power of the left and right jaguars
	{	
		LeftDriveJaguar->Set(LeftDrive->GetY());
		RightDriveJaguar->Set(RightDrive->GetY());
	}
	
};

START_ROBOT_CLASS(RobotDemo);

