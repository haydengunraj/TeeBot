#include "EV3_FileIO.c"

// Hayden Gunraj
void writeLogs(TFileHandle & out, string errorPart, string problemDescription)
{
	writeTextPC(out, errorPart);
	writeTextPC(out, ", ");
	writeFloatPC(out, "%.2f", (float)time1[T1]/1000);
	writeTextPC(out, ", ");
	writeTextPC(out, problemDescription);
	writeEndlPC(out);
}

// Hayden Gunraj
bool setupSensors(TFileHandle & errorLog)
{
	bool successful = true; // bool for controlling logic if errors occur
	string sourceOfError, message; // strings for error logging

	// check for proper sensor defaults
	if (getColorName(S4) == 6)
	{
		sourceOfError = "Colour Sensor";
		successful = false;
	}
	else if (SensorValue[S2] == 1)
	{
		sourceOfError = "Tee Touch Sensor";
		successful = false;
	}
	else if (SensorValue[S3] == 1)
	{
		sourceOfError = "Ball Touch Sensor";
		successful = false;
	}
	else if (getUSDistance(S1) < 30)
	{
		sourceOfError = "Ultra. Sensor";
		successful = false;
	}

	message = "Improper default";

	if (!successful)
		writeLogs(errorLog, sourceOfError, message);

	return successful;
}

// Hayden Gunraj
bool setupDispenser(TFileHandle & errorLog)
{
	bool successful = true; // bool for controlling logic if errors occur
	string sourceOfError, message; // strings for error logging

	setMotorBrakeMode(motorA, motorBrake); // set to resist movement if power is 0

	resetMotorEncoder(motorA);

	motor[motorA] = 30;
	wait1Msec(100); // wait 100ms for initial RPM value

	while(getMotorEncoder(motorA) < 180 && getMotorRPM(motorA) != 0)
	{}

	if (getMotorEncoder(motorA) < 180)
	{
		sourceOfError = "Dispenser";
		message = "Motor obstructed";
		successful = false;
	}

	motor[motorA] = 0;

	if (!successful)
		writeLogs(errorLog, sourceOfError, message);

	return successful;
}

// Hayden Gunraj
void setupSmallArm(int smallCount, int & time)
{
	resetMotorEncoder(motorB);
	setMotorBrakeMode(motorB, motorBrake); // set to resist movement if power is 0

	time = time1[T1];
	motor[motorB] = -60;

	while (getMotorEncoder(motorB) > -smallCount)
	{}

	motor[motorB] = 0;

	time = time1[T1] - time + 2000; // record time to rotate small arm to tee

	// set small arm back to initial position
	motor[motorB] = 60;
	while (getMotorEncoder(motorB) < 0)
	{}
	motor[motorB] = 0;
}

// Hayden Gunraj
bool setupLargeArm(TFileHandle & errorLog, int & count, int & time)
{
	bool successful = true; // bool for controlling logic if errors occur
	string sourceOfError, message; // strings for error logging

	resetMotorEncoder(motorC);
	setMotorBrakeMode(motorC, motorBrake); // set to resist movement if power is 0
	resetBumpedValue(S2);

	time = time1[T1];
	motor[motorC] = 100;
	wait1Msec(100); // wait 100ms for initial RPM value

	while(SensorValue[S2] == 0)
	{}

	motor[motorC] = 0;

	if (SensorValue[S2] == 0 && getBumpedValue(S2) == 0)
	{
		sourceOfError = "Large Arm";
		message = "Arm obstructed";
		successful = false;
	}
	else if (time1[T1] - time < 2000)
	{
		sourceOfError = "Tee Touch Sensor";
		message = "False touch";
		successful = false;
	}

	if (successful)
	{
		count = getMotorEncoder(motorC); // record encoder count for later use
		time = time1[T1] - time + 4000; // record time to rotate large arm to tee

		// set arm back to initial position
		motor[motorC] = -100;
		while (getMotorEncoder(motorC) > 0)
		{}
		motor[motorC] = 0;
	}
	else
		writeLogs(errorLog, sourceOfError, message);

	return successful;
}

// Hayden Gunraj
bool setup(TFileHandle & errorLog, int & count, int & smallTime, int & largeTime, int smallCount)
{
	bool successful = true; // bool for controlling logic if errors occur

	successful = setupSensors(errorLog);

	if (successful)
	{
		successful = setupDispenser(errorLog);

		if (successful)
		{
			setupSmallArm(smallCount, smallTime);
			successful = setupLargeArm(errorLog, count, largeTime);
		}
	}

	return successful;
}

// Sai Preetham Suryadevara
bool rotateForward(int maxTime)
{
	time1[T2] = 0;
	resetMotorEncoder(motorC);
	resetBumpedValue(S2);
	bool error = false;

  motor[motorC]= 100;

  while (SensorValue[S2] == 0 && time1[T2] < maxTime)
	{}

	if (time1[T2] < 2000 || time1[T2] >= maxTime)
		error = true;

	motor[motorC] = 0;

 	return error;
}

// Nubain Soomro
bool rotateBack(int count, int maxTime)
{
	bool error = false;

	time1[T2] = 0;
	resetMotorEncoder(motorC);

	motor[motorC] = -100;

	while(getMotorEncoder(motorC) > -count && time1[T2] < maxTime)
	{}

	motor[motorC] = 0;

	if (time1[T2] >= maxTime)
		error = true;

	return error;
}

// Fraser Robinson
bool rotateSmallArm(bool toTee, int maxTime, int smallArmCount)
{
	bool error = false;

	//assumes that towards tee is in the forward direction for the motor
	int mPower = 0;
	if (toTee)
		mPower = -60;
	else
		mPower = 60;

	resetMotorEncoder(motorB);
	time1[T2] = 0;

	//waits on both rotation and time incase of blockage/jam
	motor[motorB] = mPower;
	while (fabs(getMotorEncoder(motorB)) < smallArmCount && time1[T2] < maxTime)
	{}

	motor[motorB] = 0;

	//checks to see if time was the stopping factor, returning false (an error) if it was
	if (time1[T2] > maxTime)
		error = true;

	return error;
}

// Fraser Robinson
bool dispenseBall()
{
	bool error = false;

	resetMotorEncoder(motorA);

	//rotates arm until ball is dispensed, to a max of 180 degrees
	motor[motorA] = 20;
	wait1Msec(100); // wait 100ms for initial RPM value

	while (getMotorEncoder(motorA) < 180 && getMotorRPM(motorA) != 0)
	{}

	motor[motorA] = 0;

	if (getMotorEncoder(motorA) < 180)
		error = true;

	return error;
}

// Nubain Soomro
void emptyBalls()
{
	motor[motorA] = 20;

	displayString(6, "Press any button");
	displayString(7, "to finish.");
	while (getButtonPress(buttonAny) == 0)
	{}
	eraseDisplay();

	motor[motorA] = 0;
}

// Hayden Gunraj
void noBallReset(int mCount, int sCount, int t1, int t2)
{
	rotateSmallArm(false, t1, sCount);
	rotateBack(mCount, t2);
}

// Hayden Gunraj
task main()
{
	time1[T1] = 0; // main timer for setup operations and overall program

	const int SMALL_ARM_COUNT = 1100;

	int count = 0, errorCount = 0, iterations = 0, smallTime = 0, largeTime = 0;
	bool error = false, setupSuccess = false, notEmptied = true, noBalls = false;
	string errorSource, errorMsg;

	// initialize logging file
	TFileHandle errorLog;
	openWritePC(errorLog, "errorLog.txt");

	// set headers for output files
	writeTextPC(errorLog, "SOURCE OF ERROR, TIME SINCE START, DESCRIPTION");
	writeEndlPC(errorLog);

	// configure sensors
	SensorType[S1] = sensorEV3_Ultrasonic;
	SensorType[S2] = sensorEV3_Touch; // tee
	SensorType[S3] = sensorEV3_Touch; // dispenser
	SensorType[S4] = sensorEV3_Color;

	while (iterations <= 50 && notEmptied)
	{
		if (iterations == 0 || noBalls)
		{
			if (iterations == 0)
			{
				while (!setupSuccess)
				{
					setupSuccess = setup(errorLog, count, smallTime, largeTime, SMALL_ARM_COUNT);

					if (!setupSuccess)
					{
						displayString(3, "Setup has failed.");
						displayString(4, "Please check all");
						displayString(5, "parts and sensors,");
						displayString(6, "as well as the");
						displayString(7, "error logs, then");
						displayString(8, "press any button");
						displayString(9, "to continue.");
						while (getButtonPress(buttonAny) == 0)
						{}
						eraseDisplay();
					}
				}
			}
			displayString(5, "Please place balls");
			displayString(6, "in the dispenser, ");
			displayString(7, "then press any");
			displayString(8, "button to continue.");
			while (getButtonPress(buttonAny) == 0)
			{}
			eraseDisplay();
			noBalls = false;
		}

		if (setupSuccess)
		{
			while (getUSDistance(S1) > 30)
			{}
			time1[T2] = 0;
			while (getUSDistance(S1) < 30 && time1[T2] < 3000)
			{}

			if (time1[T2] >= 3000)
			{
				emptyBalls();
				notEmptied = false;
			}

			if (notEmptied)
			{
				error = rotateForward(largeTime);
				errorSource = "Large Arm";
				errorMsg = "Arm obstructed";

				if (!error)
				{
					error = rotateSmallArm(true, smallTime, SMALL_ARM_COUNT);
					errorSource = "Small Arm";
					errorMsg = "Arm obstructed";

					if (!error)
					{
						if (getColorName(S4) != 6) // check for ball on tee
						{
							resetBumpedValue(S3); // reset bumps before dispense

							error = dispenseBall();
							errorSource = "Dispenser";
							errorMsg = "Empty or obstructed";

							wait1Msec(750); // wait for ball to hit sensor

							if (!error)
							{
								if (getBumpedValue(S3) != 0)
								{
									//assume that 5s is the max time from the ball being dispensed to the ball arriving at the end of the arm
									time1[T2] = 0;
									while (getColorName(S4) != 6 && time1[T2] < 5000)
									{}

									//checks to see if time was the stopping factor, yielding true (an error) if it was
									if (time1[T2] >= 5000)
									{
										error = true;
										errorSource = "Rail";
										errorMsg = "Fell/stopped";
									}

									if (!error)
									{
										wait1Msec(500); // give ball a half second to settle on the tee
										error = rotateSmallArm(false, smallTime, SMALL_ARM_COUNT);
										errorSource = "Small Arm";
										errorMsg = "Arm obstructed";

										if (!error)
										{
											error = rotateBack(count, largeTime);
											errorSource = "Large Arm";
											errorMsg = "Arm obstructed";
										}
									}
								}
								else
								{
									noBallReset(count, SMALL_ARM_COUNT, smallTime, largeTime);
									noBalls = true;
								}
							}
						}
						else
						{
							noBallReset(count, SMALL_ARM_COUNT, smallTime, largeTime);
						}
					}
				}
			}
		}
		else
		{
			displayString(3, "Setup has failed.");
			displayString(4, "Please check the");
			displayString(5, "%s,", errorSource);
			displayString(6, "as well as the");
			displayString(7, "error logs, then");
			displayString(8, "press any button");
			displayString(9, "to continue.");
			while (getButtonPress(buttonAny) == 0)
			{}
			eraseDisplay();
		}

		if (error)
		{
			writeLogs(errorLog, errorSource, errorMsg);
			errorCount += 1;

			if (errorSource == "Dispenser" || errorSource == "Rail")
				noBallReset(count, SMALL_ARM_COUNT, smallTime, largeTime);

			displayString(5, "Please inspect the");
			displayString(6, "%s,", errorSource);
			displayString(7, "then press any");
			displayString(8, "button to continue.");
			while (getButtonPress(buttonAny) == 0)
			{}
			eraseDisplay();

			error = false;

			if (errorSource == "Dispenser")
				setupSuccess = setupDispenser(errorLog);
			else if (errorSource == "Small Arm")
				setupSmallArm(SMALL_ARM_COUNT, smallTime);
			else if (errorSource == "Large Arm")
				setupSuccess = setupLargeArm(errorLog, count, largeTime);
		}

		iterations += 1;
	}
	writeEndlPC(errorLog);
	writeTextPC(errorLog, "TOTAL TIME: ");
	writeFloatPC(errorLog, "%.2f", (float)time1[T1]/1000);
	writeEndlPC(errorLog);
	writeTextPC(errorLog, "TOTAL ERRORS: ");
	writeLongPC(errorLog, errorCount);
	writeEndlPC(errorLog);
	writeTextPC(errorLog, "ITERATIONS: ");
	writeLongPC(errorLog, iterations);
	writeEndlPC(errorLog);
	writeTextPC(errorLog, "ERROR RATE: ");
	writeFloatPC(errorLog, "%.2f", (float)errorCount/iterations);
	writeEndlPC(errorLog);
	closeFilePC(errorLog);
}
