/*  CMPUT 296/114 - Assignment 1 Part 2 - Due 2012-10-03
 
 Version 1.2 2012-10-01
 
 By: Monir Imamverdi
 Michael Nicholson
 
 This assignment has been done under the full collaboration model,
 and any extra resources are cited in the code below.
 
 Note on wiring:
 
 grnd->grnd
 digital 10 -> digital 11
 digitall 11-> digital 10
 
 tx1 -> rx1
 rx1 -> tx1
 
 Also, you may need to press the reset button once after opening the serial monitor in 
 order to sync the arduinos due to compilation speed inconsistencies between the two pcs.
 
 */

//public variable representing the shared secret key.
uint32_t k; 
//prime number 
const uint32_t prime = 2147483647;
//generator
const uint32_t generator = 16807;  

//generates our 8 bit private secret 'a'.
uint32_t keyGen(){
  //Seed the random number generator with a reading from an unconnected pin, I think this on analog pin 2
  randomSeed(analogRead(2));

  //return a random number between 1 and our prime .
  return random(1,prime);
}

//code to compute the remainder of two numbers multiplied together.
uint32_t mul_mod(uint32_t a, uint32_t b, uint32_t m){


  uint32_t result = 0; //variable to store the result
  uint32_t runningCount = b % m; //holds the value of b*2^i

  for(int i = 0 ; i < 32 ; i++){

    if(i > 0) runningCount = (runningCount << 1) % m;
    if(bitRead(a,i)){
      result = (result%m + runningCount%m) % m; 

    } 

  }
  return result;
}

//The pow_mod function to compute (b^e) % m that was given in the class files  
uint32_t pow_mod(uint32_t b, uint32_t e, uint32_t m)
{
  uint32_t r;  // result of this function

  uint32_t pow;
  uint32_t e_i = e;
  // current bit position being processed of e, not used except for debugging
  uint8_t i;

  // if b = 0 or m = 0 then result is always 0
  if ( b == 0 || m == 0 ) { 
    return 0; 
  }

  // if e = 0 then result is 1
  if ( e == 0 ) { 
    return 1; 
  }

  // reduce b mod m 
  b = b % m;

  // initialize pow, it satisfies
  //    pow = (b ** (2 ** i)) % m
  pow = b;

  r = 1;

  // stop the moment no bits left in e to be processed
  while ( e_i ) {
    // At this point pow = (b ** (2 ** i)) % m

    // and we need to ensure that  r = (b ** e_[i..0] ) % m
    // is the current bit of e set?
    if ( e_i & 1 ) {
      // this will overflow if numbits(b) + numbits(pow) > 32
      r= mul_mod(r,pow,m);//(r * pow) % m; 
    }

    // now square and move to next bit of e
    // this will overflow if 2 * numbits(pow) > 32
    pow = mul_mod(pow,pow,m);//(pow * pow) % m;

    e_i = e_i >> 1;
    i++;
  }

  // at this point r = (b ** e) % m, provided no overflow occurred
  return r;
}

void setup(){



  //Initialize the serial ports that will be in communication
  Serial.begin(9600);
  Serial1.begin(9600);
  //Serial port for the random number generator
  Serial2.begin(9600);

  //When the arduino is reset everything is set to low.

  //initialize pins
  pinMode(11, OUTPUT); //sets pin 11 to output
  pinMode(10, INPUT);  //sets pin 10 to input


  //turn on pin to signal that we are online
  digitalWrite(11, HIGH);

  //wait for other arduino to come online
  while(digitalRead(10)==LOW){
  }

  //This is our secret key
  uint32_t a = keyGen();

  //This is our shared index 'A'
  uint32_t A = pow_mod(generator, a, prime);

  Serial.print("Shared index is: ");
  Serial.println(A);

  uint8_t tempBits = 0;

  uint32_t receivedBits;

  uint32_t B = 0;

  digitalWrite(11,LOW);

  for(int i = 0 ; i < 4 ; i++){

    //WAIT FOR THE OTHER HERE
    digitalWrite(11,HIGH);
    while(digitalRead(10)==LOW){
    }

    //shifting from the end to the beggining    
    tempBits = A >> 8*(3-i); //shift by one byte at a time
    tempBits = tempBits & 0xff;
    Serial1.write(tempBits);

    receivedBits = 0;

    while(Serial1.available() == 0){
    } //wait until the there are bits in the serial
    receivedBits = Serial1.read();

    receivedBits = receivedBits<<8*(3-i);
    B=B | receivedBits; 
    digitalWrite(11,LOW);
  }

  Serial.print("Received shared index is: ");
  Serial.println(B);

  //This is our shared secret encryption key.
  k = pow_mod(B, a, prime);

  //reseed the random number generator with the shared secret key k
  randomSeed(k);

}

//Now lets send characters back and forth. This is encrypted with our secret key 'k' in conjuction with the xor function.

//This was taken from the "Multi Serial Mega" Arduino example and modifified with the XOR function.
void loop(){

  // read from serial port 1, and send to serial monitor on port 0
  if (Serial1.available()) {
    // returns one byte of unsigned serial data, or -1 if none available
    int16_t inByte = Serial1.read();  //mask out lower 8 bits from k
    Serial.write(inByte^(random(256)));
    if(inByte = '\n')
      randomSeed(k);
  }

  // read from serial monitor on port 0, and send to port 1
  if (Serial.available()) {
    // returns one byte of unsigned serial data, or -1 if none available
    int16_t inByte = Serial.read();   //mask out lower 8 bits from k
    Serial1.write(inByte^(random(256)));
    if(inByte ='\n')
      randomSeed(k);
  }

}



