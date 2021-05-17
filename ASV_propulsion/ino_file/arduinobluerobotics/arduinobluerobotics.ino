#include <Servo.h>
#include <ros.h>
#include <std_msgs/UInt16.h>
#include <std_msgs/UInt8.h>

ros::NodeHandle  nh;

/* X8R Receiver arduino pins
const int PIN_X8R_4 = 10; 
const int PIN_X8R_2 = 9;
const int PIN_X8R_5 = 11; */

/* Add LED pins
int red_light_pin= 16;
int green_light_pin = 15;
int blue_light_pin = 14; */

//autonomous navigation ints
int powerR = 1500;
int powerL = 1500;

/* X8R Receiver channels
float channel4; //perpendicular axis lever - rotation
float channel2; //parallel axis lever - displacement
float channel5; //choice lever */

//Thrusters declaration
Servo thrusterRight;
Servo thrusterLeft;

void right_cb( const std_msgs::UInt16& cmd_msg){
  powerR = cmd_msg.data; //1100-1900  
}

void left_cb( const std_msgs::UInt16& cmd_msg){
  powerL = cmd_msg.data; //1100-1900  
}

std_msgs::UInt8 flag;
ros::Subscriber<std_msgs::UInt16> rsub("rpwm", right_cb);
ros::Subscriber<std_msgs::UInt16> lsub("lpwm", left_cb);
ros::Publisher arduino("arduino", &flag);

void setup() {
  /*Pin modes
  pinMode(PIN_X8R_4, INPUT);
  pinMode(PIN_X8R_2, INPUT);
  pinMode(PIN_X8R_5, INPUT); */

  /*Add LED pin modes
  pinMode(red_light_pin, OUTPUT);
  pinMode(green_light_pin, OUTPUT);
  pinMode(blue_light_pin, OUTPUT); */

  //ROS
  nh.initNode();
  nh.subscribe(rsub);
  nh.subscribe(lsub);
  nh.advertise(arduino);

  //PIN Thrusters
  thrusterRight.attach(5);
  thrusterLeft.attach(3);

  //Stop thrusters, required
  thrusterRight.writeMicroseconds(1500);
  thrusterLeft.writeMicroseconds(1500);
  
  //Driver setup
  delay(1000);
}

/* void read_values () {
  //Read channel frequecies
  channel4 = pulseIn(PIN_X8R_4, HIGH);
  //Serial.print("CH 4 ");
  //Serial.println(channel4);
  channel2 = pulseIn(PIN_X8R_2, HIGH);
  //Serial.print("CH 2 ");
  //Serial.println(channel2);
  channel5 = pulseIn(PIN_X8R_5, HIGH);
  //Serial.print("CH 5 ");
  //Serial.println(channel5); */
}

/*void RGB_color(int red_light_value, int green_light_value, int blue_light_value)
 {
  if (red_light_value == 1) {
    digitalWrite(red_light_pin, HIGH);
  }
  else {
    digitalWrite(red_light_pin, LOW);
  }
  
  if (green_light_value == 1) {
    digitalWrite(green_light_pin, HIGH);
  }
  else {
    digitalWrite(green_light_pin, LOW);
  }

  if (blue_light_value == 1) {
    digitalWrite(blue_light_pin, HIGH);
  }
  else {
    digitalWrite(blue_light_pin, LOW);
  }
  
 } */

void select() {
  //Use channel 5 to select current mode
  if (channel5 < 1300) {
     power_Difference();
     //RGB_color(1, 1, 0); // Yellow
  }
  else if ( channel5 > 1600) {
      autonomous_Mode();
      //RGB_color(0, 0, 1); // Blue
  }
  else {
      thrusterRight.writeMicroseconds(1500);
      thrusterLeft.writeMicroseconds(1500);
      //RGB_color(1, 0, 0); // Red
  }
}

void power_Difference() {
//void for controlled movement
  float R;
  float L;

  float Tx;
  float Tz;
  float Tport;
  float Tstbd;  

  if ((channel4 > 1450 & channel4 < 1550) & (channel2 > 1450 & channel2 < 1550)){     //Control stable
    R=1500;
    L=1500;
    thrusterRight.writeMicroseconds(R);
    thrusterLeft.writeMicroseconds(L);      //thrusters at zero
  }
  else if ((channel4 < 1450 || channel4 > 1550) || (channel2 < 1450 || channel2 > 1550)) {    //Control for moving
    Tx = map(channel2, 975, 2025, -70, 70);
    Tz = map(channel4, 975, 2025, -5, 5);
    
    Tstbd = (Tx / 2) - (Tz / 0.41);
    if (Tstbd > 35){
      Tstbd = 35;
    }
    else if (Tstbd < -35){
      Tstbd = -35;
    }

    Tport = (Tx / (2 * 1.27)) + (Tz / (0.41 * 1.27));
    if (Tport > 27){
      Tport = 27;
    }
    else if (Tstbd < -27){
      Tstbd = -27;
    }

    R = round((Tstbd / 35 * 400)+1500);
    L = round((Tport / 35 * 400)+1500);
    thrusterRight.writeMicroseconds(R);
    thrusterLeft.writeMicroseconds(L);
  }

}

void autonomous_Mode() {
  thrusterRight.writeMicroseconds(powerR);
  thrusterLeft.writeMicroseconds(powerL);
}

void loop() {
  read_values();
  select();
  flag.data = 1;
  arduino.publish( &flag );
  nh.spinOnce();
  delay(1);
}
