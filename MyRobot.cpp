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
	Jaguar *Jaguar1;
	Jaguar *Jaguar2;
	Watchdog *Saftey;
	Joystick *Stick1;
	Joystick *Stick2;
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
		Jaguar1 = new Jaguar(6);
		Jaguar2 = new Jaguar(7);
		Saftey = new Watchdog;
		Stick1 = new Joystick(1);
		Stick2 = new Joystick(2);
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
		delete Jaguar1;
		delete Jaguar2;
		delete Saftey;
		delete dsLCD;
		delete ds;
	}

	/**
	 * Runs the motors with arcade steering. 
	 */
	void OperatorControl(void)
	{
		//HSLImage *Himage;
		//Threshold targetThreshold(247, 255, 60, 140, 10, 50);
		//BinaryImage *matchingPixels;
		//vector<ParticleAnalysisReport> *pReport;
		
		//myRobot->SetSafetyEnabled(true);
		Saftey->SetEnabled(false);
		//AxisCamera &mycam = AxisCamera::GetInstance("10.15.10.11");
		
		//Camera->AxisCamera("10.15.10.11");
		//mycam.WriteResolution(AxisCamera::kResolution_640x480);
		//mycam.WriteCompression(20);
		//mycam.WriteBrightness(25);
		Wait(3.0);
         
		dsLCD = DriverStationLCD::GetInstance();
		dsLCD->Clear();
		
		float X[2];
		float Y[2];
		float Z[2];
		
		while(IsOperatorControl())
		{
		X[1] = Stick1->GetX();
		X[2] = Stick2->GetX();
		Y[1] = Stick1->GetY();
		Y[2] = Stick2->GetY();
		Z[1] = Stick1->GetZ();
		Z[2] = Stick2->GetZ();
		
		Jaguar1->Set(Y[1]);
		Jaguar2->Set(Y[2]);
		
		Wait(0.005);
		}
		/*while (IsOperatorControl())
		{
			Wait(1.0);
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
			}*/
			
			/*Y1 = Stick1->GetY();
			Y2 = Stick2->GetY();
			printf("%f, %f\n", Y1, Y2);
			Jaguar1->Set(Y1);
			Jaguar2->Set(Y2);*/
			
			//myRobot->ArcadeDrive(stick); // drive with arcade style (use right stick)
			//Wait(0.005);				// wait for a motor update time
	}
	
	/*void AutonomousRobotDrive(float Magnitude, float Curve, float Time)
	{
		for(double i=0; i<Time; i = (i + (WATCHDOG_EXPIRATION - 0.01)))
		{
			Jaguar1->Set(Magnitude + (Curve * Magnitude));
			Jaguar2->Set(-Magnitude - (Curve * Magnitude));
			Wait(WATCHDOG_EXPIRATION - 0.01);
			Saftey->Feed();
		}
		//execute drive code here
	}*/
	
	/*void TeleopRobotDrive(void)
	{
		
		}
	}*/
	//~RobotDrive()
	//{
		//delete stick;
	//}
};

START_ROBOT_CLASS(RobotDemo);

