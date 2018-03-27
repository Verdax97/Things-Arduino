#define leds "D5"
#define buttonPin "D7"
#define armPin "D2"
#define armLed "D3"

void setup() {
  // put your setup code here, to run once:
  // println("Initialising...\n");
  pinMode(armPin, INPUT);
  pinMode(buttonPin, INPUT);
  pinMode(leds, OUTPUT);
  // println("\nDone!\n");
}

void loop() {
  bool isExploded;

  isExploded = false;
  
  if(digitalRead(armPin) == LOW){
    // Flashbang unarmed
    // println("Flashbang not armed.")
    digitalWrite(armLed, LOW);
  } else {
    // Flashbang armed
    digitalWrite(armLed, HIGH);
    // println("Flashbang armed!");
  }
  
  while(digitalRead(armPin) == HIGH){
    if(digitalRead(buttonPin) == HIGH && isExploded == false){
      // Initialise countdown for exploding (5 seconds total)
      // Led blink effect
      for(int i = 0; i < 25; i++){
        digitalWrite(armLed, LOW);
        delay(100);
        digitalWrite(armLed, HIGH);
        delay(100);
      }
      // Explosion
      for(int i = 0; i < 100; i++){
        digitalWrite(leds, HIGH);
        delay(50);
        digitalWrite(leds, LOW);
        delay(25);
      }
      
      isExploded = true;
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
}
