float previous_RD = 0, previous_RT = 0, previous_LD = 0, previous_LT = 0;

void send_power(float m1,float m2,float m3,float m4){
    String result = "<0,"+ String(m1)+ "," + String(m2) + "," + "1," + String(m3) + "," + String(m4) + ">";
  
    Serial.println(result);
}

float calculate_motor(float v_x, float v_y, float angular, float L,float radius, int motor){
    float vel = 0;
    if (motor == 1) {
        vel = ((2*L*angular) - (sqrt(3)*v_x) + (sqrt(3)*v_y))/(2*r);
    }
    if (motor == 2) {
        vel = ((2*L*angular) - (sqrt(3)*v_x) - (sqrt(3)*v_y))/(2*r);
    }
    if (motor == 3) {
        vel = ((2*L*angular) + (sqrt(3)*v_x) - (sqrt(3)*v_y))/(2*r);
    }  
    if (motor == 4) {
        vel = ((2*L*angular) + (sqrt(3)*v_x) + (sqrt(3)*v_y))/(2*r);
    }
    return vel;
}

float acceleration(float m, float previous_m, float vel_step){
    a = (abs(m-previous_m))/dt;

    if(a > (vel_step)){
        if(m > previous_m){
        m = previous_m + (vel_step * dt);
        }
        
        else{
        m = previous_m - (vel_step * dt);
        }
    }
    return m;
}

void motors_control(float x, float y, float angular) {
    float vel_RD = calculate_motor(x, y, angular, L, r, 1);
    float vel_RT = calculate_motor(x, y, angular, L, r, 2);
    float vel_LD = calculate_motor(x, y, angular, L, r, 3);
    float vel_LT = calculate_motor(x, y, angular, L, r, 4);
  
    vel_RD = acceleration(vel_RD, previous_RD, vel_step);
    vel_RT = acceleration(vel_RT, previous_RT, vel_step);
    vel_LD = acceleration(vel_LD, previous_LD, vel_step);
    vel_LT = acceleration(vel_LT, previous_LT, vel_step);  
  
    send_power(vel_RD, vel_RT, vel_LD, vel_LT);
  
    previous_RD = vel_RD;
    previous_RT = vel_RT;
    previous_LD = vel_LD;
    previous_LT = vel_LT;
}
  
void failSafe(){
    v_l = 0.00;
    v_a = 0.00;
    th = 0.00;
    last_error = 0;
    error_sum = 0;
    stop = true;
}