void kick(int microseconds_time){
    kicker_mark = millis();
    digitalWrite(KICKER_PIN, HIGH);
    delayMicroseconds(microseconds_time);
    digitalWrite(KICKER_PIN, LOW);
    kick_time = 0;
}

//TO DO: dribbler