#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

#define buttonPin  2
#define ledPin 9
#define buzzer 8

int clearButton = 6;
int spaceButton = 4;
int replayButton = 3;


int col = 0;
int lin = 0;

int OK;
int OKclear = 0;
int OKspace = 0;

bool buttonState, lastButtonState, checker = false, lineChecker = false;
int buttonStateClear = 0, buttonStateSpace = 0, buttonStateReplay = 0;
int prestate1, prestate2, prestate3;

String cuvant = "";
int pause_value;
int  signal_length = 0, pause = 0;

String database[37] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-",
                       ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",

                       "-----", ".----", "..---", "...--", "....-", ".....",
                       "-....", "--...", "---..", "----." , "END"
                      };

char* letters[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", // A-I
  ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", // J-R
  "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.." // S-Z
};
char* numbers[] = {
  "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."
}; // 0-9

//array which is latter used for conversion to char
String morse = "";
char dash = '-', dot = '.';

void setup() { //initial values setup
  Serial.begin(9600);
  pinMode(clearButton, INPUT);
  pinMode(spaceButton, INPUT);
  pinMode(replayButton, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzer, OUTPUT);

  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);


  lcd.init();                      // initialize the lcd
  lcd.clear();
  lcd.backlight();
  col = 0;
  lin = 0;

}

void getCode(char* string) // sequences through each string retrieve dot or dash element
{
  int i = 0;
  while (string[i] != '\0')
  {
    sendElement(string[i]);
    i++;
  }
  delay(pause_value * 3); // spacing between letters
}


void sendElement(char dotOrDash) // transmit dot or dash element as morse code
{
  digitalWrite (buzzer, HIGH);
  digitalWrite(ledPin, HIGH);
  if (dotOrDash == '.')
  {
    delay(pause_value); // dot duration
  }
  else

  {
    delay(pause_value * 3); // dash duration
  }
  digitalWrite (buzzer, LOW);
  digitalWrite(ledPin, LOW);
  delay(pause_value); // spacing between dots and dashes
}




void loop() {
  char ch;
  pause_value = 200;

  buttonState = !digitalRead(buttonPin); //inverting the value of buttonpin due to using input pullup with the button

  buttonStateClear = digitalRead(clearButton);
  buttonStateSpace = digitalRead(spaceButton);
  buttonStateReplay = digitalRead(replayButton);

  if (buttonState && lastButtonState) { //when the button is being continuously pressed
    signal_length++;
    if (signal_length < 2 * pause_value) {
      digitalWrite (buzzer, HIGH);  //turning on the buzzer and LED pins
      digitalWrite(ledPin, HIGH);
    }

  }
  else if (!buttonState && lastButtonState) {          //when the button is released
    if (signal_length > 50 && signal_length <  pause_value )
      morse += dot;                 // depending on the length of the click assign either dash or dot to morse variable
    else if (signal_length >  pause_value)
      morse += dash;  //

    signal_length = 0;
    digitalWrite(ledPin, LOW); //turning off the LED pin
    digitalWrite (buzzer, LOW);  //turning the buzzer off
  }

  else if (buttonState && !lastButtonState) { //when the button is just pressed
    pause = 0;   //value resets
    checker = true;
    lineChecker = true;
  }
  else if (!buttonState && !lastButtonState) { //when the button is continuously not pressed
    pause++;

    if (pause > 3 * pause_value && checker) { //checking if the pause between signals is long enough, if so trigger to part of the code which prints from morse to english

      lcd.setCursor(col, lin);
      translate(morse);
      col++;
      checker = false;  //reset of values
      morse = "";
    }
    if ((pause > 15 * pause_value) && lineChecker) {
      lineChecker = false;
    }

  }

  if (buttonStateClear == HIGH && prestate1 == 0 ) {
    Serial.println(buttonStateClear);
    lcd.clear();
    cuvant = "";
    col = 0 ;
    lin = 0;
    delay(10);
    prestate1 = 1;
  }
  else if (buttonStateClear == LOW) prestate1 = 0;

  if (buttonStateReplay == HIGH && prestate3 == 0 ) {
    int i;
    int lin_cuv = 0, col_cuv = 0;

    lcd.noBlink();
    for (i = 0; i <= cuvant.length(); i++)
    {
      if (col_cuv == 16 && lin_cuv == 0)
      {
        col_cuv = 0; lin_cuv = 1;
      }

      ch = cuvant[i];
      lcd.setCursor(col_cuv, lin_cuv);
      lcd.cursor();

      if (ch >= 'A' && ch <= 'Z')
      {
        getCode(letters[ch - 'A']); // 'A'Z= 65
      }
      else if (ch >= '0' && ch <= '9')
      {
        getCode(numbers[ch - '0']); // '0'= 48
      }
      else if (ch == ' ')
      {
        delay(pause_value * 7); // spacing between words
      }
      
      col_cuv++;
      lcd.noCursor();

      if (i == cuvant.length())  lcd.blink();
    }
  }
  else if (buttonStateReplay == LOW) prestate3 = 0;

  if (buttonStateSpace == HIGH && prestate2 == 0 ) {

    OKspace = 1;
    delay(10);
    prestate2 = 1;
  }
  else if (buttonStateSpace == LOW) prestate2 = 0;

  if (OKspace == 1)
  {
    col++;
    cuvant = cuvant + " ";
    OKspace = 0;
    lcd.setCursor(col, lin);
  }
  if (col == 16 && lin == 0)
  {
    col = 0; lin = 1;
    lcd.setCursor(col, lin);
  }
  if (col > 15 && lin == 1)
  {
    lcd.clear();
    cuvant = "";
    col = 0; lin = 0;
    lcd.setCursor(col, lin);
  }

  lastButtonState = buttonState;  //assigning value to lastbuttonstate
  delay(1);
}

void translate(String text) { //more efficient managment of string to letter conversion
  int letters_and_numbers = 0;
  int i = 0;
  while (database[i] != "E")
  {
    if (text == ".-.-.-")
    {
      Serial.print(".");        //for break
    }

    if (text == "*-")
    {
      letters_and_numbers = 'A';
      break;
    }
    if (text == "-----")
    {
      letters_and_numbers = '0';
      break;
    }
    if (text == database[i]) { //comparing ascci values with position in a defined array
      if (i <= 26)      //this if block exists due to the ASCII encoding
        letters_and_numbers = 65 + i;
      else if (i > 26)
      {
        letters_and_numbers = i - 26;
        OK = 1;
      }
    }
    i++;
    if (database[i] == "END")
    {
      lcd.print("");      //if input code doesn't match any letter, print nothing
    }
  }

  lcd.noBlink();
  lcd.blink();

  if (OK == 0) {
    lcd.print((char)letters_and_numbers);
    cuvant = cuvant + (char)letters_and_numbers;
    Serial.println(cuvant);
  }
  else {
    lcd.print(letters_and_numbers);
    cuvant = cuvant + letters_and_numbers; OK = 0;
    Serial.println(cuvant);
  }

}
