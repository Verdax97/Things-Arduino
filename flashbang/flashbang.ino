#define leds 5
#define buttonPin 7
#define armPin 2
#define armLed 3

void setup() {
  Serial.begin(9600);
  Serial.println("Initialising...\n");
  pinMode(armPin, INPUT);
  pinMode(buttonPin, INPUT);
  pinMode(armLed, OUTPUT);
  pinMode(leds, OUTPUT);
  Serial.println("\nDone!\n");
}

void loop() {
  boolean isExploded;

  isExploded = false;

  Serial.println("Flashbang initiated. Ready to explode >:) \n");
  
  if(digitalRead(armPin) == LOW){
    // Flashbang unarmed
    Serial.println("Flashbang not armed.");
    digitalWrite(armLed, LOW);
  } else {
    // Flashbang armed
    digitalWrite(armLed, HIGH);
    Serial.println("Flashbang armed!");
  }
  
  while(digitalRead(armPin) == HIGH){
    if(digitalRead(buttonPin) == HIGH && isExploded == false){
      // Initialise countdown for exploding (5 seconds total)
      // Led blink effect
      Serial.println("Ignition sequence started!");
      for(int i = 0; i < 25; i++){
        digitalWrite(armLed, LOW);
        delay(100);
        digitalWrite(armLed, HIGH);
        delay(100);
      }
      // Explosion
      Serial.println("BOOM!");
      for(int i = 0; i < 100; i++){
        digitalWrite(leds, HIGH);
        delay(50);
        digitalWrite(leds, LOW);
        delay(25);
      }
      
      isExploded = true;
      Serial.begin("Flash is now exploded!");
      break;
    }
  }
  
  while(isExploded){
    digitalWrite(armLed, HIGH);
    delay(2000);
    if(digitalRead(armPin) == LOW){
      break;
    }
  }
  Serial.println("Sequence terminated. Flashbang is reinitiated.");
}
