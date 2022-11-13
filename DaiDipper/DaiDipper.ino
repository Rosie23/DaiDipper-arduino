#include <TimerOne.h>
#include <LiquidCrystal.h>
#include <ClickEncoder.h>
#include <Stepper.h>

//SD 
#include <SPI.h>
#include <SD.h>
File root;

//#define REPRAP_DISCOUNT_SMART_CONTROLLER
//#define KILL_PIN 41 //[RAMPS14-SMART-ADAPTER]  
//#define BEEPER 37 //[RAMPS14-SMART-ADAPTER] / 37 = enabled; -1 = dissabled / (if you don't like the beep sound ;-)

//TO DO ADD RESET TO DEFAULTS OPTION
//REMOVE LIGHT:ON MENU
//MAKE EDIT PROTOCOL OPTION
//Add SD Card support
//pot and height dist calibration
//give protocol options - i.e. quick dip a set number of times with or without shake afterwards

 //lcd pins  
#define LCD_PINS_RS 16 //[RAMPS14-SMART-ADAPTER]  
#define LCD_PINS_ENABLE 17 //[RAMPS14-SMART-ADAPTER]  
#define LCD_PINS_D4 23 //[RAMPS14-SMART-ADAPTER]  
#define LCD_PINS_D5 25 //[RAMPS14-SMART-ADAPTER]  
#define LCD_PINS_D6 27 //[RAMPS14-SMART-ADAPTER]  
#define LCD_PINS_D7 29 //[RAMPS14-SMART-ADAPTER]  

 //encoder pins  
#define BTN_EN1 31 //[RAMPS14-SMART-ADAPTER]  
#define BTN_EN2 33 //[RAMPS14-SMART-ADAPTER]  
#define BTN_ENC 35 //[RAMPS14-SMART-ADAPTER]

  //NEMA Motor pins
// left motor
#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38
//#define X_MIN_PIN           3
//#define X_MAX_PIN           2

// right motor
#define Z_STEP_PIN         46
#define Z_DIR_PIN          48
#define Z_ENABLE_PIN       62
#define Z_CS_PIN           40

//other pins
#define BEEPER 37
#define KILL_PIN 41 //[RAMPS14-SMART-ADAPTER]  
#define SDCARDDETECT 49 //[RAMPS14-SMART-ADAPTER] 53
//boolean card_inserted = SD.begin(SDCARDDETECT);

int stepsPerRev = 200;

///Set up LCD
LiquidCrystal lcd(LCD_PINS_RS, LCD_PINS_ENABLE, LCD_PINS_D4, LCD_PINS_D5, LCD_PINS_D6, LCD_PINS_D7);

//set the pins for the stepper motor
Stepper left_Stepper = Stepper(stepsPerRev, X_DIR_PIN, X_STEP_PIN); 
Stepper right_Stepper = Stepper(stepsPerRev, Z_DIR_PIN, Z_STEP_PIN); 

//Interface strings
String protocol_finish_msg = "Finished running ";
int protocol;

String protocol_ask = "What protocol would you like to run?";
String protocol_running_msg1 = "Running ";
String protocols_avaliable_msg = "Current avaliable protocols;";


//Step distances and speed parameters
int height_steps = 400; //setting_item1 
int pot_distance = 1000; //setting2
int hook_dist = 100; //setting3
int x_stepper_speed = 100; //setting4
int z_stepper_speed = 100;  //setting5
int mini_delay = 100; //setting6
boolean beeper_on = false;

int height_steps_default = height_steps; //setting_item1 
int pot_distance_default = pot_distance; //setting2
int hook_dist_default = hook_dist; //setting3
int x_stepper_speed_default = x_stepper_speed; //setting4
int z_stepper_speed_default = z_stepper_speed;  //setting5
int mini_delay_default = mini_delay; //setting6

String setting_item1 = "Z-height";
String setting_item2 = "Pot dist";
String setting_item3 = "Hook dist";
String setting_item4 = "X-axis Speed";
String setting_item5 = "Z-axis Speed";
String setting_item6 = "Mini delay";

ClickEncoder *encoder;
int16_t last, value;

int menuitem = 1;
int frame = 1;
int page = 1;
int lastMenuItem = 1;
int item = 1;
int lastItem=1;
boolean YES = true;

String menuItem1 = "Select Protocol";
String menuItem2 = "View Protocols";
String menuItem3 = "Add/Edit Protocol";
String menuItem4 = "Settings";
String menuItem5 = "Open SD Card";
String menuItem6 = "Calibrate X-axis";
String menuItem7 = "Calibrate Z-axis";
String menuItem8 = "Reset";

boolean backlight = true;

//Protocols
String protocols[] = {"Protocol 1", "Protocol 2", "User Protocol"};
int selectedProtocol = 0;
int protocol_list_size = 3;

long protocol1_incubations[] = {5, 6000, 10000, 60000, 10000, 40000}; //5 is number of pots
String protocol1_solutions[] = {"Xylene I","Xylene II","Xylene III","EtOH 100%","EtOH 90%"};

long protocol2_incubations[] = {5, 1000, 1000, 2000, 1000, 4000}; //5 is number of pots
String protocol2_solutions[] = {"Xylene I","Xylene II","Xylene III","EtOH 100%","EtOH 90%"};

long user_incubations[] = {};
String user_solutions[] = {};


String difficulty[2] = { "EASY", "HARD" };
int selectedDifficulty = 0;

boolean up = false;
boolean down = false;
boolean middle = false;
boolean double_click = false;

void setup() {

encoder = new ClickEncoder(33, 31, 35, 4);
//encoder->setAccelerationEnabled(false);

Timer1.initialize(1000);
Timer1.attachInterrupt(timerIsr); 

pinMode(X_STEP_PIN, OUTPUT);
pinMode(X_DIR_PIN, OUTPUT);
pinMode(X_ENABLE_PIN, OUTPUT);

pinMode(Z_STEP_PIN, OUTPUT);
pinMode(Z_DIR_PIN, OUTPUT);
pinMode(Z_ENABLE_PIN, OUTPUT);

pinMode(BEEPER, OUTPUT);
//pinMode(KILL_PIN, INPUT);

right_Stepper.setSpeed(z_stepper_speed);
left_Stepper.setSpeed(x_stepper_speed);

last = 0;
//
//last = encoder->getValue();

lcd.begin(20,4);      
lcd.clear(); 

}

void loop() {
//turn stepper motors off when not in use
//digitalWrite(X_STEP_PIN, LOW);
//digitalWrite(X_DIR_PIN, LOW);
//digitalWrite(X_ENABLE_PIN, LOW);
//digitalWrite(Z_STEP_PIN, LOW);

  drawMenu();
  delay(100);
  readRotaryEncoder();
  
     ClickEncoder::Button b = encoder->getButton();
   if (b != ClickEncoder::Open) {
   switch (b) {
    case ClickEncoder::Clicked:
    middle=true;
    break;
    case ClickEncoder::DoubleClicked:
    double_click=true;
    break;
    }
  }

//Move through Main Menu - clockwise
  if (up && page == 1) {
    up = false;
    if(menuitem==2 && frame == 2)
    {
      frame--;
    }
      if(menuitem==3 && frame == 3)
    {
      frame--;
    }
    if(menuitem==4 && frame == 4)
    {
      frame--;
    }
    if(menuitem==5 && frame == 5)
    {
      frame--;
    }
    
    lastMenuItem = menuitem;
    menuitem--;
    if (menuitem==0)
    {
      menuitem=1;
    } 
  }
  
//Move through Main Menu - Anti-clockwise
    if (down && page == 1)
  {
    down = false;
    if(menuitem==3 && lastMenuItem == 2)
    {
      frame ++;
    }
    else  if(menuitem==4 && lastMenuItem == 3)
    {
      frame ++;
    }
        else  if(menuitem==5 && lastMenuItem == 4)
    {
      frame ++;
    }
     else  if(menuitem==6 && lastMenuItem == 5 && frame!=5)
    {
      frame ++;
    }
    lastMenuItem = menuitem;
    menuitem++;  
    if (menuitem==8) 
    {
      menuitem--;
    }
  }

  //Move through Select Protocols
if(page == 2 && menuitem==1){
    if(up)
    {up = false;
    item--;}
    if(down)
    {down = false;
    item++;}
    if (item==4) //stops cursor leaving screen
    {item--;}
    if (item==0) 
    {item++;}
}

  //Move through View Protocols
if(page == 2 && menuitem==2){
    if(up)
    {up = false;
    item--;}
    if(down)
    {down = false;
    item++;}
    if (item==4) 
    {item--;}
    if (item==0) 
    {item++;}
}

//Select through Settings - Clockwise
  if (down && page == 2 && menuitem==4)
  {
    down = false;
    if(item==3 && lastItem == 2)
   {
    frame ++;
   }
    else  if(item==4 && lastItem == 3)
    {
      frame ++;
    }
     else  if(item==5 && lastItem == 4 && frame!=4)
    {
      frame ++;
    }
    lastItem = item;
    item++;  
   if (item==7) 
  {
    item--;
    }
  }

//Select through Settings - Anti-clockwise
    if (up && page == 2 && menuitem==4) { 
    up = false;
    if(item==2 && frame == 2)
    {
      frame--;
    }
      if(item==3 && frame == 3)
    {
      frame--;
    }
    if(item==4 && frame == 4)
    {
      frame--;
    }
    lastItem = item;
    item--;
    if (item==0)
    {
      item=1;
    } 
  }

//Double click to return to previous menu
if(double_click){
  double_click=false;
  if(page==2)
  {
  page=1;
  frame=1;
  menuitem=1;
  }
  if(page==3)
  {
  page=2;
  frame=1;
  }
  
}

//Update Settings
      if (page == 3 && menuitem == 4)
  {
  if(item==1)
  {
    height_steps = UpdateSettingValue(height_steps, 10000000, -10000000, 10);
    PrintSettingValue(setting_item1, height_steps);
    }
  if(item ==2)
  {
    pot_distance = UpdateSettingValue(pot_distance, 1000000, -10000000, 10);
    PrintSettingValue(setting_item2, pot_distance);
  }
  if(item==3)
  {
    hook_dist = UpdateSettingValue(hook_dist, 1000000, -10000000, 10);
    PrintSettingValue(setting_item3, hook_dist);
    }
  if (item ==4)
  {
    x_stepper_speed = UpdateSettingValue(x_stepper_speed, 300, 0, 5);
    PrintSettingValue(setting_item4, x_stepper_speed);
    }
  if (item ==5)
  {
    z_stepper_speed = UpdateSettingValue(z_stepper_speed, 300, 0, 5);
    PrintSettingValue(setting_item5, z_stepper_speed);
    }
  if (item ==6)
  {
    mini_delay = UpdateSettingValue(mini_delay, 10000, 0, 10);
    PrintSettingValue(setting_item6, mini_delay);
    }  
  }

  //Middle Button is Pressed
  if (middle) 
  {
    middle = false;
    
    if(item == 1 && page==2 && menuitem == 1){ //Select Protocol to run menu
      RunProtocol("Protocol 1",protocol1_incubations, protocol1_solutions);
   }
   if(item == 2 && page==2 && menuitem == 1){ //Select Protocol to run menu
      RunProtocol("Protocol 2",protocol2_incubations, protocol2_solutions);
   }
    if(item == 1 && page==2 && menuitem == 2){ //View Protocol Information
      ViewProtocol("Protocol 1",protocol1_incubations, protocol1_solutions);
   }
   if(item == 2 && page==2 && menuitem == 2){ 
      ViewProtocol("Protocol 2",protocol2_incubations, protocol2_solutions);
   }
   if(item == 3 && page==2 && menuitem == 2){
      ViewProtocol("Protocol 3",user_incubations, user_solutions);
   }
    if (page == 2 && menuitem == 4) // Select settings
    {page=3;}

    else if (page == 1 && menuitem<=6) { //Go to next page
      page=2;
      item=1;
      frame=1;
     }
   }
}

void drawMenu()
  {
    
  if (page==1) 
  {    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("     DAI DIPPER");

    if(menuitem==1 && frame ==1)
    {   
      displayMenuItem(menuItem1, 1,true);
      displayMenuItem(menuItem2, 2,false);
      displayMenuItem(menuItem3, 3,false);
    }
    else if(menuitem == 2 && frame == 1)
    {
      displayMenuItem(menuItem1, 1,false);
      displayMenuItem(menuItem2, 2,true);
      displayMenuItem(menuItem3, 3,false);
    }
    else if(menuitem == 3 && frame == 1)
    {
      displayMenuItem(menuItem1, 1,false);
      displayMenuItem(menuItem2, 2,false);
      displayMenuItem(menuItem3, 3,true);
    }
     else if(menuitem == 4 && frame == 2)
    {
      displayMenuItem(menuItem2, 1,false);
      displayMenuItem(menuItem3, 2,false);
      displayMenuItem(menuItem4, 3,true);
    }

      else if(menuitem == 3 && frame == 2)
    {
      displayMenuItem(menuItem2, 1,false);
      displayMenuItem(menuItem3, 2,true);
      displayMenuItem(menuItem4, 3,false);
    }
    else if(menuitem == 2 && frame == 2)
    {
      displayMenuItem(menuItem2, 1,true);
      displayMenuItem(menuItem3, 2,false);
      displayMenuItem(menuItem4, 3,false);
    }
    
    else if(menuitem == 5 && frame == 3)
    {
      displayMenuItem(menuItem3, 1,false);
      displayMenuItem(menuItem4, 2,false);
      displayMenuItem(menuItem5, 3,true);
    }

    else if(menuitem == 6 && frame == 4)
    {
      displayMenuItem(menuItem4, 1,false);
      displayMenuItem(menuItem5, 2,false);
      displayMenuItem(menuItem6, 3,true);
    }
    
      else if(menuitem == 5 && frame == 4)
    {
      displayMenuItem(menuItem4, 1,false);
      displayMenuItem(menuItem5, 2,true);
      displayMenuItem(menuItem6, 3,false);
    }
      else if(menuitem == 4 && frame == 4)
    {
      displayMenuItem(menuItem4, 1,true);
      displayMenuItem(menuItem5, 2,false);
      displayMenuItem(menuItem6, 3,false);
    }
    else if(menuitem == 3 && frame == 3)
    {
      displayMenuItem(menuItem3, 1,true);
      displayMenuItem(menuItem4, 2,false);
      displayMenuItem(menuItem5, 3,false);
    }
     else if(menuitem == 2 && frame == 2)
    {
      displayMenuItem(menuItem2, 1,true);
      displayMenuItem(menuItem3, 2,false);
      displayMenuItem(menuItem4, 3,false);
    }
    else if(menuitem == 4 && frame == 3)
    {
      displayMenuItem(menuItem3, 1,false);
      displayMenuItem(menuItem4, 2,true);
      displayMenuItem(menuItem5, 3,false);
    }
    else if(menuitem == 5 && frame == 5)
    {
      displayMenuItem(menuItem5, 1,true);
      displayMenuItem(menuItem6, 2,false);
      displayMenuItem(menuItem7, 3,false);
    }   
    else if(menuitem == 6 && frame == 5)
    {
      displayMenuItem(menuItem5, 1,false);
      displayMenuItem(menuItem6, 2,true);
      displayMenuItem(menuItem7, 3,false);
    }
    else if(menuitem == 7 && frame == 5)
    {
      displayMenuItem(menuItem5, 1,false);
      displayMenuItem(menuItem6, 2,false);
      displayMenuItem(menuItem7, 3,true);
    } 
    lcd.display();
  }
  
  if (page==2)
  {
   if(menuitem == 1)
  {    
   displayProtocolPage(item, "Select Protocol;");
  }
  else if (menuitem == 2) 
  {
   displayProtocolPage(item, "View Protocol;");
  }
   else if (menuitem == 3) 
  {
    //Add/Edit Protocols
  }
  if(menuitem==4){  //Edit Settings
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Settings");
    
  if (frame==1 && item == 1) 
  {
    displayMenuItem(setting_item1, 1,true);
    displayMenuItem(setting_item2, 2,false);
    displayMenuItem(setting_item3, 3,false);
  }
  else if (frame==1 && item == 2) 
  {
    displayMenuItem(setting_item1, 1,false);
    displayMenuItem(setting_item2, 2,true);
    displayMenuItem(setting_item3, 3,false);
  }
    else if (frame==1 && item == 3) 
  {
    displayMenuItem(setting_item1, 1,false);
    displayMenuItem(setting_item2, 2,false);
    displayMenuItem(setting_item3, 3,true);
  }
  else if (frame==2 && item == 4) 
  {
    displayMenuItem(setting_item2, 1,false);
    displayMenuItem(setting_item3, 2,false);
    displayMenuItem(setting_item4, 3,true);
  }
  else if (frame==2 && item == 3) 
  {
    displayMenuItem(setting_item2, 1,false);
    displayMenuItem(setting_item3, 2,true);
    displayMenuItem(setting_item4, 3,false);
  }
    else if (frame==2 && item == 2) 
  {
    displayMenuItem(setting_item2, 1,true);
    displayMenuItem(setting_item3, 2,false);
    displayMenuItem(setting_item4, 3,false);
  }
      else if (frame==3 && item == 3) 
  {
    displayMenuItem(setting_item3, 1,true);
    displayMenuItem(setting_item4, 2,false);
    displayMenuItem(setting_item5, 3,false);
  }
        else if (frame==3 && item == 4) 
  {
    displayMenuItem(setting_item3, 1,false);
    displayMenuItem(setting_item4, 2,true);
    displayMenuItem(setting_item5, 3,false);
  }
  else if ( frame==3 && item == 5) 
  {
    displayMenuItem(setting_item3, 1,false);
    displayMenuItem(setting_item4, 2,false);
    displayMenuItem(setting_item5, 3,true);
  }
        else if ( frame==4 && item == 4) 
  {
    displayMenuItem(setting_item4, 1,true);
    displayMenuItem(setting_item5, 2,false);
    displayMenuItem(setting_item6, 3,false);
  }
          else if (frame==4 && item == 5) 
  {
    displayMenuItem(setting_item4, 1,false);
    displayMenuItem(setting_item5, 2,true);
    displayMenuItem(setting_item6, 3,false);
  }
          else if (frame==4 && item == 6) 
  {
    displayMenuItem(setting_item4, 1,false);
    displayMenuItem(setting_item5, 2,false);
    displayMenuItem(setting_item6, 3,true);
  }
    }
    
    if(menuitem == 5) //Open SD Card
  {
      if(!SD.begin(SDCARDDETECT)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SD Card read failed");
        lcd.setCursor(0, 2);
        lcd.print("Is a card inserted?");
        lcd.setCursor(0, 3);
        lcd.print("<-- Check Port");

      } else{
        SD.begin(SDCARDDETECT);
        //dataFile.read();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Card read success!");
       //myFile = SD.read(SDCARDDETECT)
         root = SD.open("/");
         printDirectory(root, 0);
      }
    
  }
  if(menuitem == 6 ) //Calibrate-X axis
  {
     //ReturnHome();
     pot_distance = 0;
     pot_distance = CalibrateX();
  } 
    if(menuitem == 7 ) //Calibrate-Z axis
  {
     //ReturnHome();
     left_Stepper.step(pot_distance);
     if(YES){
      lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Is hook directly over first pot?");
     lcd.setCursor(0,2);
     lcd.print("->YES");
     lcd.setCursor(0,3);
     lcd.print("NO");
     } else if(!YES){
      lcd.setCursor(0,2);
     lcd.print("YES");
     lcd.setCursor(0,3);
     lcd.print("->NO");
     }
     if(YES && middle){
      height_steps = 0;
      height_steps = CalibrateZ();
     }
     
  } 
  
  }
  }

  void timerIsr() {
  encoder->service();
}


String StringMinSec(unsigned long millisec)
{
unsigned long seconds = millisec / 1000;
unsigned long minutes = seconds / 60;
seconds %= 60;
if(seconds == 0){
String MinSec_string = (String(minutes) + ":00");
return MinSec_string;
}
else if(seconds <10){
  String MinSec_string = (String(minutes) + ":" + "0" + String(seconds));
  return MinSec_string;
  }
  else {
    String MinSec_string = (String(minutes) + ":" + String(seconds));
    return MinSec_string;
    }
}


void displayMenuItem(String item, int position, boolean selected)
{
    if(selected)
    {
    lcd.setCursor(0, position);
    lcd.print(">"+item);
    }else
   {
    lcd.setCursor(0, position);
    lcd.print(item);
    }
}

void displayProtocolPage(int item, String heading)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(heading);
    for(int n=1; n<=protocol_list_size; n++){  //max 3 over 3 need new page
      if(item==n)
    {
      lcd.setCursor(0, n);
      lcd.print(">"+protocols[n-1]);
    }else
   {
      lcd.setCursor(0, n);
      lcd.print(protocols[n-1]);
    }
      } 
}

void ViewProtocol(String protocol_name, long protocol_incubations[], String protocol_solutions[])
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(protocol_name);
  for(int n=0; n<protocol_incubations[0];)
  {
    lcd.setCursor(0, 0);
    lcd.print(protocol_name);
     
    for(int row=1; row<=3; row++, n++)
    {
    lcd.setCursor(0,row);
    lcd.print(protocol_solutions[n]);
    lcd.setCursor(16,row);
          
    unsigned long millisec = protocol_incubations[n+1];
    String Min_sec;
    Min_sec = StringMinSec(millisec);
    lcd.print(Min_sec);
    }
    delay(5000);
    lcd.clear();
  }
}

void readRotaryEncoder()
{
  value += encoder->getValue();
  
  if (value > last) {
    last = value;
    down = true;
    delay(150);
  }else   if (value < last) {
    last = value;
    up = true;
    delay(150);
  }
}

void GoLeft(int steps)
{
  for(int n=1; n<steps; n++){
left_Stepper.step(-1);
delay(1);
right_Stepper.step(-1);
delay(1);
}
}

void GoRight(int steps)
{
  for(int n=1; n<steps; n++){
left_Stepper.step(1);
delay(1);
right_Stepper.step(1);
delay(1);
}
}

void GoUp(int steps)
{
  for(int n=1; n<steps; n++){
left_Stepper.step(-1);
delay(1);
right_Stepper.step(1);
delay(1);
}
}

void GoDown(int steps)
{
  for(int n=1; n<steps; n++){
left_Stepper.step(1);
delay(1);
right_Stepper.step(-1);
delay(1);
}
}

void RunProtocol(String selected_protocol_name, long selected_protocol[], String protocol_solutions[])
{
lcd.setCursor(0,0);
lcd.clear();
//lcd.print(selected_protocol_name);

  for(int pot=1; pot<selected_protocol[0]; pot++)
{
  lcd.print(protocol_running_msg1 + selected_protocol_name);
  //1. Move down and pick up slides and return
GoDown(height_steps);
delay(mini_delay);
GoRight(hook_dist);
delay(mini_delay);
GoUp(height_steps);

//2. Move and drop slides to next pot and unhook
GoRight(pot_distance); //move to pot
delay(mini_delay);
GoDown(height_steps); //drop down to pot
delay(mini_delay);
GoLeft(hook_dist); //unhook
GoUp(height_steps); //return to up

//3. incubation
lcd.setCursor(0,1);
lcd.print("Incubating in");
lcd.setCursor(0,2);
lcd.print(protocol_solutions[pot-1]);
lcd.setCursor(0,3);

unsigned long millisec = selected_protocol[pot];
String Min_sec;
Min_sec = StringMinSec(millisec);
lcd.print("Duration = " + Min_sec);
delay(selected_protocol[pot]);
lcd.clear();
  }
  
lcd.clear();
lcd.setCursor(0,0);
lcd.print(protocol_finish_msg);
lcd.setCursor(0,1);
lcd.print(selected_protocol_name);

//Beeper when protocol is finished
if(beeper_on == true)
{
tone(BEEPER, 500); // Send 500KHz sound signal...
delay(100);
noTone(BEEPER);     // Stop sound...
delay(100);
tone(BEEPER, 500); 
delay(100);  
noTone(BEEPER);     
delay(100);
tone(BEEPER, 500); 
delay(100);        
noTone(BEEPER);  
  }

}

void displaySettings()
{
lcd.clear();
lcd.setCursor(0,0);
lcd.print("Settings");
lcd.setCursor(0,1);
lcd.print("Z-height = "+String(height_steps));
lcd.setCursor(0,2);
lcd.print("Pot dist = "+String(pot_distance));
lcd.setCursor(0,3);
lcd.print("Hook dist = "+String(hook_dist));
delay(5000);

lcd.clear();
lcd.setCursor(0,0);
lcd.print("Settings");
lcd.setCursor(0,1);
lcd.print("X-axis speed = "+String(x_stepper_speed));
lcd.setCursor(0,2);
lcd.print("Z-axis Speed = "+String(z_stepper_speed));
lcd.setCursor(0,3);
lcd.print("Mini delay = "+String(mini_delay));
delay(5000);
}

void PrintSettingValue(String settingName, long settingValue)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Settings");
  lcd.setCursor(0,1);
  lcd.print(settingName);
  lcd.setCursor(10,3);
  lcd.print(settingValue);
}

int UpdateSettingValue(int settingValue, int minValue, int maxValue, int increment)
{
  int new_settingvalue = settingValue;
  if(up && settingValue == maxValue){
    up = false;
    return new_settingvalue;
}
  else if(down && settingValue == minValue){
    down=false;
    return new_settingvalue;
}
  else if(down){
    down=false;
    new_settingvalue = settingValue+increment;
    return new_settingvalue;
}
  else if(up){
    up=false;
    new_settingvalue = settingValue-increment;
    return new_settingvalue;
}
}

void printDirectory(File dir, int numTabs) {
  while (true) {
    lcd.clear();
    lcd.setCursor(0,0);
  }
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      //lcd.print('\t');
      lcd.setCursor(0,i);
    }
    lcd.print(entry.name());
    if (entry.isDirectory()) {
      //lcd.print("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      //lcd.print("\t\t");
      lcd.print(entry.size(), DEC);
    }
    entry.close();
  }

// void ReturnHome()

int CalibrateX(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibrate X-axis");
  lcd.setCursor(0,1);
  lcd.print("Move until directly above pot");
  lcd.setCursor(10,3);
  lcd.print(pot_distance);
  int new_settingvalue = pot_distance;
  if(down && pot_distance == 0){
    down=false;
    return new_settingvalue;
}
  else if(down){
    down=false;
    GoRight(10);
    new_settingvalue = pot_distance+10;
    return new_settingvalue;
}
  else if(up){
    GoLeft(10);
    up=false;
    new_settingvalue = pot_distance-10;
    return new_settingvalue;
}
}

int CalibrateZ(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Calibrate Z-axis");
  lcd.setCursor(0,1);
  lcd.print("Move until hook directly next to pot");
  lcd.setCursor(10,3);
  lcd.print(height_steps);
  int new_settingvalue = height_steps;
  if(down && height_steps == 0){
    down=false;
    return new_settingvalue;
}
  else if(down){
    down=false;
    GoDown(10);
    new_settingvalue = height_steps+10;
    return new_settingvalue;
}
  else if(up){
    GoUp(10);
    up=false;
    new_settingvalue = height_steps-10;
    return new_settingvalue;
}
}

//void EditProtocol(int pot_number, String protocol_solutions[], int protocol_incubations[])
//{
//}
