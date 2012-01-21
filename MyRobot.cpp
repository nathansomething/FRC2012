#include "WPILib.h"
#include <vector>
#include "ImageProcessing.h"
using namespace std;

/**
 * This is a demo program showing the use of the RobotBase class.
 * The SimpleRobot class is the base of a robot application that will automatically call your
 * Autonomous and OperatorControl methods at the right time as controlled by the switches on
 * the driver station or the field controls.
 */
const double WATCHDOG_EXPIRATION = 0.1;

class RobotDemo : public SimpleRobot {
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
	
	void DriveAutonomously(float Speed, float Curve) {
		if (Curve < 0) //Right Turn
		{
			LeftDriveJaguar->Set(Speed);
			RightDriveJaguar->Set(Speed - (Curve * Speed));
		}
		if (Curve < 0) //Left Turn
		{
			LeftDriveJaguar->Set(Speed - (Curve * Speed));
			RightDriveJaguar->Set(Speed);
		}
		if (Curve == 0) //Straight
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

	void ImageProcess() {
		HSLImage *Himage;
		IVA_Data *mydata;

		AxisCamera &mycam = AxisCamera::GetInstance("10.15.10.11");

		if (mycam.IsFreshImage()) {
			Himage = mycam.GetImage();

			IVA_ProcessImage(Himage->GetImaqImage(), mydata);
			for (int i = 0; i < mydata->numSteps; i++) {
				if (i == 5) //This represents the fifth stage of the target detection, which gives data on the rectangle that we want to track.
				{
					for (int j = 0; j < mydata->stepResults[i].numResults; j++) {
						IVA_Result & tmpresult =
								mydata->stepResults[i].results[j];

						switch (tmpresult.type) {
						case IVA_NUMERIC:
							printf("%d, %d, %s, %f\n", i, j,
									tmpresult.resultName,
									tmpresult.resultVal.numVal);
							break;
						case IVA_BOOLEAN:
							printf(
									"%d, %d, %s, %s\n",
									i,
									j,
									tmpresult.resultName,
									(tmpresult.resultVal.boolVal == TRUE) ? "true"
											: "false");
							break;
						case IVA_STRING:
							printf("%d, %d, %s, %s\n", i, j,
									tmpresult.resultName,
									tmpresult.resultVal.strVal);
							break;
						}
					}
				}
			}
		}
		Wait(1.0);
		delete Himage;
		IVA_DisposeData(mydata);
	}
	
	/**
	 * Drive left & right motors for 2 seconds then stop
	 */
	void Autonomous(void) {
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
	void OperatorControl(void) {
		AxisCamera &mycam = AxisCamera::GetInstance("10.15.10.11");

		mycam.WriteResolution(AxisCamera::kResolution_640x480);
		mycam.WriteBrightness(40);
		mycam.WriteColorLevel(50);
		mycam.WriteCompression(20);

		//dsLCD = DriverStationLCD::GetInstance();
		//dsLCD->Clear();

		while (IsOperatorControl()) {
			UpdateJoystickDrive();
			ImageProcess();
		}
	}
	
	~RobotDemo() {
		delete LeftDriveJaguar;
		delete RightDriveJaguar;
	}
};

START_ROBOT_CLASS(RobotDemo)
;
