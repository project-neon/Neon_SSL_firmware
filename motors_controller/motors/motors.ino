#include <SimpleFOC.h>

BLDCMotor motor  = BLDCMotor(11, 10, 60);
BLDCMotor motor2 = BLDCMotor(11, 10, 60);
BLDCDriver3PWM driver = BLDCDriver3PWM(PA0, PA1, PA2, PB10);
BLDCDriver3PWM driver2 = BLDCDriver3PWM(PA6, PA7, PB0, PA5);

// This is de code for the board that is in robots
int robot_id = 1;
int id;
int first_mark = 0, second_mark;

boolean newData = false; 
const byte numChars = 64;
char receivedChars[numChars];
char tempChars[numChars];
int battery_voltage = 15; 

//target variable
float m1 = 0;
float m2 = 0;

void setup() {
  pinMode(PC13,OUTPUT);

  driver.voltage_power_supply = battery_voltage;
  driver.voltage_limit = battery_voltage;
  driver.init();
  driver2.voltage_power_supply = battery_voltage;
  driver2.voltage_limit = battery_voltage;
  driver2.init();

  motor.linkDriver(&driver);
  motor2.linkDriver(&driver2);

  motor.current_limit  = 0.3;  // [Amps]
  motor2.current_limit = 0.3; 
 
  // open loop control config
  motor.controller = MotionControlType::velocity_openloop;
  motor2.controller = MotionControlType::velocity_openloop;
  // init motor hardware
  motor.init();
  motor2.init();

  //motor.initFOC();
  //motor2.initFOC();

  digitalWrite(PC13,LOW);
  
  Serial.begin(115200);
  //Serial.println("Motor ready!");
  //Serial.println("START");
  delay(1000);
}

void loop() {
  recvWithStartEndMarkers();

  if (newData == true){
      strcpy(tempChars, receivedChars);
      digitalWrite(PC13,!digitalRead(PC13));
      //Serial.println(tempChars);
      parseData();
      //Serial.println(m1);
      //Serial.println(m2);
      newData = false;
      first_mark = millis();
  }
  second_mark = millis();

  if (second_mark - first_mark > 300) {
   // driver.setPhaseState(_HIGH_IMPEDANCE , _HIGH_IMPEDANCE, _HIGH_IMPEDANCE);
   // driver2.setPhaseState(_HIGH_IMPEDANCE , _HIGH_IMPEDANCE, _HIGH_IMPEDANCE);
    m1 = 0.00;
    m2 = 0.00;

  }//else{
    //driver.setPhaseState(_HIGH_IMPEDANCE , _HIGH_IMPEDANCE, _HIGH_IMPEDANCE);
    //driver2.setPhaseState(_HIGH_IMPEDANCE , _HIGH_IMPEDANCE, _HIGH_IMPEDANCE);

  //}
  float current_m = min(0.06*m1,0.7);

  motor.current_limit = current_m;
  motor2.current_limit = current_m;
  
  motor.move(m1);
  motor2.move(m2);
  
}

void parseData(){
    char * strtokIndx;
  
    strtokIndx = strtok(tempChars, ",");
    
    while (strtokIndx != NULL){
        id = atoi(strtokIndx);
        //Serial.println(id);
        
        if(id == robot_id){         
          strtokIndx = strtok(NULL, ",");  
          m1 = atof(strtokIndx);       
          strtokIndx = strtok(NULL, ",");         
          m2 = atof(strtokIndx);
          strtokIndx = strtok(NULL, ","); 
       }

       else{
          strtokIndx = strtok(NULL, ",");     
          strtokIndx = strtok(NULL, ",");         
          strtokIndx = strtok(NULL, ","); 
       }
   } 
}

void recvWithStartEndMarkers(){
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char in;

    while (Serial.available()){
        //  Formato da mensagem::
        //  <[id1],[v_l1],[v_a1],[id2],[v_l2],[v_a2],[id3],[v_l3],[v_a3]>
        in = Serial.read();

        if (recvInProgress == true){
            if (in != endMarker){
                receivedChars[ndx] = in;
                ndx++;
                if (ndx >= numChars){
                    ndx = numChars - 1;
                }
            }
            else{
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (in == startMarker){
            recvInProgress = true;
        }
    }
}
