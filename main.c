#pragma config(UART_Usage, UART1, uartUserControl, baudRate1200, IOPins, None, None)
#pragma config(Motor,  port2,           leftMotor,     tmotorServoContinuousRotation, openLoop)
#pragma config(Motor,  port4,           hookServo,     tmotorServoStandard, openLoop)
#pragma config(Motor,  port5,           corpseServo,   tmotorServoStandard, openLoop, reversed)
#pragma config(Motor,  port7,           liftMotor,     tmotorServoContinuousRotation, openLoop)
#pragma config(Motor,  port9,           rightMotor,    tmotorServoContinuousRotation, openLoop, reversed)
/*
	The code above is automatically generated by the ROBOTC motor and sensor setup page.
	It tells the program what port each motor is in, what type of motor it is, and what name we want to give the motor.
*/


/*
	All of the code between this comment and the next comment is used to set the baud rate (communication speed) of the UART port.
	The UART port is the communication port that is used to give instructions to the IR transmitter used to change Squeaky's control scheme.
	I don't really understand this code. It was copied from a tutorial from BEST Robotics.
	Normally, setting the baudRate would be as simple as using `setBaudRate(UART1, <baud rate>);`.
	However, RobotC only allows the baud rate to be set to certain fixed values, such as baudRate1200.
	RobotC does not normally support a baud rate of 600 (the baud rate required to use the IR kit.)
	Because of this, we have to use the complicated code below to make our own method to change the baud rate.
*/

typedef unsigned long  uint32_t;
typedef unsigned short uint16_t;

typedef struct
{
  uint16_t SR;
  uint16_t RESERVED0;
  uint16_t DR;
  uint16_t RESERVED1;
  uint16_t BRR;
  uint16_t RESERVED2;
  uint16_t CR1;
  uint16_t RESERVED3;
  uint16_t CR2;
  uint16_t RESERVED4;
  uint16_t CR3;
  uint16_t RESERVED5;
  uint16_t GTPR;
  uint16_t RESERVED6;
} USART_TypeDef;

#define PERIPH_BASE           ((unsigned long)0x40000000)
#define APB1PERIPH_BASE       PERIPH_BASE
#define USART2_BASE           (APB1PERIPH_BASE + 0x4400)
#define USART3_BASE           (APB1PERIPH_BASE + 0x4800)
#define USART2                ((USART_TypeDef *) USART2_BASE)
#define USART3                ((USART_TypeDef *) USART3_BASE)

void setBaud( const TUARTs nPort, int baudRate ) {
    uint32_t tmpreg = 0x00, apbclock = 0x00;
    uint32_t integerdivider = 0x00;
    uint32_t fractionaldivider = 0x00;

    apbclock = 36000000;

    integerdivider = ((0x19 * apbclock) / (0x04 * (baudRate)));
    tmpreg = (integerdivider / 0x64) << 0x04;

    fractionaldivider = integerdivider - (0x64 * (tmpreg >> 0x04));
    tmpreg |= ((((fractionaldivider * 0x10) + 0x32) / 0x64)) & 0x0F;

    USART_TypeDef *uart = USART2;
    if( nPort == UART2 ) {
      uart = USART3;
    }
    uart->BRR = (uint16_t)tmpreg;
}

/*
	Now we are back to code that I can explain.
	The `main` task contains all of the code that is going to be run when the VEX Cortex is turned on.
	It should also be noted that I refer to Squeaky by many names in this code including but not limited to Squishy, Stinky, and Sparky.
	I apologize for any confusion but I still think it was funny.
*/

task main()
{
	// Set the UART commmunication speed for IR sensor 600
	setBaud(UART1, 600);

	// Contains if currently squarky control scheme or drive control scheme
	// Drive mode is the default.
	bool isSquishyMode = false;

	// Current control mode for Stinky control box
	// Drive mode is the default.
	string sparkyCurrent = "drive";

	// Loop that runs forever to keep the robot running
	while (true){

		// Check if button 5U is pressed, and if it is, switch into drive mode.
		if (vexRT[Btn5U]){
			isSquishyMode = false;
			/*
				writeDebugStreamLine outputs a message to the RobotC software window that can be read by humans.
				It does not affect what the robot does.
				It is commonly used to make sure that the code is working as desired before all of the motors are hooked up to the Cortex.
				I have also used it in the past to get control data from the robot for the autonomous task.
			*/
			writeDebugStreamLine("Drive Mode");
		}

		// Check if button 5D is pressed, and if it is, switch to Sqeuaky mode.
		if (vexRT[Btn5D]){
			isSquishyMode = true;
			writeDebugStreamLine("Smoshy Mode");
		}

		// Spoopy Controls
		// Check if in squeaky mode, and if so, use the Squeaky control scheme.
		if (isSquishyMode){

			// Most of this code is repeated, so I will only explain it once here.
			// Set lever order (left to right) to Drive ? Lift ? Rotate.
			if (vexRT[Btn7L]){
				// Set the current control mode to lift.
				sparkyCurrent = "lift";
				writeDebugStreamLine("Middle Lift Mode");
				/*
					sendChar is used to send data over a communications port.
					UART1 is the port that is used on the Cortex.
					0x55 is the hexadecimal code that is sent.
					The hexadecimal codes that I used are documented in the rules of the game. (See pinned messages in #general on Discord for rules.)
				*/
				sendChar(UART1, 0x55);
				/*
					wait1Msec makes the program pause for a set amount of time in milliseconds.
					In this case, it is pausing for 200 milliseconds, or 0.2 seconds.
					This delay is used to prevent the UART port from getting spammed, and it can also prevent misinputs.
				*/
				wait1Msec(200);
			}

			// Set lever order (left to right) to Rotate ? Drive ? Lift.
			if (vexRT[Btn7D]){
				sparkyCurrent = "drive";
				writeDebugStreamLine("Middle Drive Mode");
				sendChar(UART1, 0x66);
				wait1Msec(200);
			}

			// Set lever order (left to right) to Lift ? Rotate ? Drive.
			if (vexRT[Btn7R]){
				sparkyCurrent = "rotate";
				writeDebugStreamLine("Middle Rotate Mode");
				sendChar(UART1, 0xCC);
				wait1Msec(200);
			}

			// Set speed to low
			if (vexRT[Btn8L]){
				if (sparkyCurrent == "drive"){
						// If Squeaky is in drive mode, use the code that sets drive speed to low.
						sendChar(UART1, 0x99);
						writeDebugStreamLine("Drive Low Mode");
						wait1Msec(200);
				}
				if (sparkyCurrent == "rotate"){
						// If Squeaky is in rotate mode, use the code that sets rotate speed to low.
						sendChar(UART1, 0x69);
						writeDebugStreamLine("Rotate Low Mode");
						wait1Msec(200);
				}
			}

			// Set speed to medium
			if (vexRT[Btn8D]){
				if (sparkyCurrent == "drive"){
						// If Squeaky is in drive mode, use the code that sets drive speed to medium.
						sendChar(UART1, 0xA5);
						writeDebugStreamLine("Drive Medium Mode");
						wait1Msec(200);
				}
				if (sparkyCurrent == "rotate"){
						// If Squeaky is in rotate mode, use the code that sets rotate speed to medium.
						sendChar(UART1, 0x96);
						writeDebugStreamLine("Rotate Medium Mode");
						wait1Msec(200);
				}
			}

			// Set speed to high
			if (vexRT[Btn8R]){
				if (sparkyCurrent == "drive"){
						// If Squeaky is in drive mode, use the code that sets drive speed to high.
						sendChar(UART1, 0xC3);
						writeDebugStreamLine("Drive High Mode");
						wait1Msec(200);
				}
				if (sparkyCurrent == "rotate"){
						// If Squeaky is in rotate mode, use the code that sets rotate speed to high.
						sendChar(UART1, 0x0F);
						writeDebugStreamLine("Rotate High Mode");
						wait1Msec(200);
				}
			}

			// Test IR Connection
			if (vexRT[Btn6D]){
				sendChar(UART1, 0xF0);
				writeDebugStreamLine("Test IR Mode");
				wait1Msec(200);
			}

			/*
				The VEX Joystick controller is prone to controller drift, so I use this if statement to create a deadzone.
				If the controller reports values between 20 and -20 (out of a range of 127 to -127), it ignores them and assumes it is controller drift.
				Ch3 is the left thumbstick Y axis.
			*/
			if (vexRT[Ch3] > 20 || vexRT[Ch3] < -20){
				/*
					motor[motorName] sets the amount of power the given motor will recieve.
					It also accepts a range of 127 to -127.
					In the case of a DC (large or small drive) motor, the motor value indicates how fast the motor spins.
					In the case of a servo motor, the motor value indicates the position the motor is turned to.
					In this case, we are multiplying the speed by 0.4 so that the motors move slowly during Squeaky mode.
					We are also setting both the left and right motor to the same value so that the robot moves straight forward and straight back.
				*/
				motor[leftMotor] = vexRT[Ch3] * 0.4;
				motor[rightMotor] = vexRT[Ch3] * 0.4;
			}
			else{
				// If the thumbstick is not being pushed, stop the motors.
				motor[leftMotor] = 0;
				motor[rightMotor] = 0;
			}

		}
		// This is the code that runs when the robot is not in Squeaky mode.
		else {
			// This is the same code as above except it is not multiplied by 0.4 to run slowly.
			// This code also only sets one motor per thumbstick, allowing for turning.
			if (vexRT[Ch3] > 20 || vexRT[Ch3] < -20){
				motor[leftMotor] = vexRT[Ch3];
			}
			else {
				motor[leftMotor] = 0;
			}

			// Ch2 is the right thumbstick Y value.
			if (vexRT[Ch2] > 20 || vexRT[Ch2] < -20){
				motor[rightMotor] = vexRT[Ch2];
			}
			else {
				motor[rightMotor] = 0;
			}

			// If 8D is pressed, set lift motor speed to 50.
			if (vexRT[Btn8D]){
				motor[liftMotor] = 50;
			}
			// If 8U is pressed, but not 8D, set lift motor speed to -50.
			else if (vexRT[Btn8U]){
				motor[liftMotor] = -50;
			}
			// If neither 8D nor 8U is pressed, turn off the lift motor.
			else {
				motor[liftMotor] = 0;
			}

			/*
				This code controls the scoliosis.
				I do not know why I called the motor "corpseServo".
				I probably just didn't want to spell scoliosis.
			*/
			if (vexRT[Btn8R]){
				// If 8R is pressed, push out with the scoliosis.
				motor[corpseServo] = 65;
			}
			else {
				// Otherwise, keep the scoliosis pulled in.
				motor[corpseServo] = -127;
			}


			if (vexRT[Btn7D]){
				if (motor[hookServo] >= 15){
					/*
						motor[hookServo] >= 15 makes sure that the hook motor does not turn too far and start to push into other components.
						Pushhing into objects causes the servo to overheat and could even damage the servo.
					*/
					// Subtract five from servo position.
					motor[hookServo] -= 5;
					// Delay to prevent the servo from turning many times instantly.
					wait1Msec(25);
				}
			}
			if (vexRT[Btn7U]){
				// Add five to servo position.
				motor[hookServo] += 5;
				wait1Msec(25);
			}
		}

	}
		// I don't know why the next comment is here but I'm leaving it because I like it.
		// maetrpice
}
