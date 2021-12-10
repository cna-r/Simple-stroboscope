
#include <SPI.h>
//#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//* Comment out above, uncomment this block to use hardware SPI
// SCK = 13 --- OLED_CLK
// MOSI = 11 --- OLED_MOSI
//MISO = 12  --- NC
#define OLED_DC     4
#define OLED_CS     3
#define OLED_RESET  5
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  &SPI, OLED_DC, OLED_RESET, OLED_CS);

unsigned int i,p,Row,battery_read,battery_read_before,battery_perc,battery_perc_before,brightness,duty_cycle,bat_counter;
unsigned long freq,temp,presc,rpmppresc,rpm,rpmx,tpm,rpmstep,t_b_Pressed,tbp_diff,time_passed,bat_timer;
unsigned long time_ref1,time_ref11, time_ref12, time_ref2, mean_battery;
unsigned int mean_battery_level_i;
byte a,b,c,d;
bool refresh_oled_flag,battery_charging,switched_off,rpmx_done,last_charging_state;

void setup() {
freq = 16000000;
  
  pinMode(6,INPUT);
  pinMode(7,INPUT);
  pinMode(8,INPUT);

  //pinMode(2,INPUT);
  pinMode(A2, INPUT);
  pinMode(13,OUTPUT);
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(9, OUTPUT);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  // OR EXTERNALVCC
  
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
//    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0,16);
  display.println("STROBE OFF");
display.display();
brightness = 1;
rpmx = 1;
rpm = 60;
time_ref1 = millis();
time_ref2 = millis();
time_ref11 = millis();
time_ref12 = millis();
last_charging_state = 0;
}

void loop() {

bat_counter = bat_counter + 1;
if (bat_counter == 36000) bat_counter = 0;
if (bat_counter == 0) {
battery_read = mean_battery_level();

if (battery_read < 790) {
  battery_perc = 100;
  battery_charging = 1;
}
else {
battery_charging = 0;
if (battery_read > 790) battery_perc = 100;
if (battery_read > 810) battery_perc = 75;
if (battery_read > 822) battery_perc = 50;
if (battery_read > 830) battery_perc = 25;
if (battery_read > 850) low_battery();
}

battery_mon();

}
/*
if (digitalRead(A2) && !last_charging_state) {
  battery_charging = 1; 
  last_charging_state = 1;
  battery_mon();
}
if (!digitalRead(A2) && last_charging_state) {
  battery_charging = 0;
  last_charging_state = 0;
  battery_mon();
}
*/

 





if (digitalRead(6)) time_ref11 = millis();
while (digitalRead(6)) {
    time_ref12 = millis();
    if (!rpmx_done) {
      run_rpmx();
      rpmx_done = 1;
    }
    
    if ((time_ref12 - time_ref11) > 2000) {
      digitalWrite(9, LOW);
      switched_off=1;
      delay(200);
    }
    
    if (digitalRead(7) && switched_off) flash_brightness();
   
   if (!digitalRead(6)) {
    time_ref11 = millis();
    time_ref12 = millis();
    switched_off = 0;
    rpmx_done = 0;
   }
}




if (digitalRead(7)) {
 rpm = rpm + rpmx;
    if (rpm > 20000) {
      rpm = 20000;
    }
 
 refresh_rpm_on_oled();
}

if (digitalRead(8)) {
 rpm = rpm - rpmx;
    if ((rpm < 60) || (rpm > 100000)) {
      rpm = 60;
    }
 
 refresh_rpm_on_oled();

}
}


void refresh_rpm_on_oled(void) {
  
  refresh_oled_flag = 1;
  display.clearDisplay();

  battery_mon();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(94,0);             // Start at top-left corner
  display.print("X");
  display.println(rpmx);

  display.setTextSize(3);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(10,16);
  display.println(rpm);

  display.display();
  delay(200);
  setStrobe();
}

void run_rpmx(){
  switch (rpmx) {
    case 1:
    rpmx = 10;
    break;
    case 10:
    rpmx = 100;
    break;
    case 100:
    rpmx = 1000;
    break;
    case 1000:
    rpmx = 1;
    break;
    //delay(250);
  }
if (!switched_off) {
refresh_rpm_on_oled();
}
else {
  switched_off = 0;
}
}

void setStrobe() {

switch (brightness) {
  case 1:
  duty_cycle = 200;
  break;
  case 2:
  duty_cycle = 150;
  break;
  case 3:
  duty_cycle = 100;
  break;
  case 4:
  duty_cycle = 50;
  break;
  
}
  
  i = 0;

if ((rpm >= 16000) && (rpm <= 20000))  {


rpmppresc = rpm;
TCCR1A = _BV(COM1A1) | _BV(WGM11) ;
TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10) ;
temp = (freq*60/rpmppresc)  ;
i = int(temp);
p = i / duty_cycle;
}



if ((rpm < 16000) && (rpm > 2000))  {

presc = 8L;
rpmppresc = presc * rpm;
TCCR1A = _BV(COM1A1) | _BV(WGM11) ;
TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11) ;
temp = (freq*60/rpmppresc)  ;
i = int(temp);
p = i / duty_cycle;
}

if (rpm <= 2000) {
presc = 256L;
rpmppresc = presc * rpm ;

TCCR1A = _BV(COM1A1) | _BV(WGM11) ;
TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12) ;
temp = (freq*60/rpmppresc)  ;
i = int(temp);
p = i / 50;
  
}


if (i > 0) {
a = highByte(i);
b = lowByte(i);
c = highByte(p);
d = lowByte(p);

ICR1H = a;
ICR1L = b;
OCR1AH = c;
OCR1AL = d;

}

}

unsigned int mean_battery_level() {
  
  int i=0;
  mean_battery = analogRead(A0);
  for (i=0;i<250;i++) {
  mean_battery = (mean_battery + analogRead(A0)) / 2;
  
  }
  return mean_battery;
  
  }

void battery_mon() {
//this to test:
//battery_charging = 0;

if (battery_charging) {
        display.fillRect(2, 0, 4, 2, WHITE);

        display.drawLine(10, 11, 15, 5 , WHITE);
        display.drawLine(15, 6, 15, 9 , WHITE);
        display.drawLine(15, 9, 20, 1 , WHITE);
        display.drawRect(0, 2,8,12, WHITE);
        display.fillRect(0, 2, 8, 12, WHITE);

        display.display();
}

if (!battery_charging) {
display.fillRect(10, 0, 11, 14, BLACK);
display.display();

if ((battery_perc != battery_perc_before) || refresh_oled_flag) {

if ((battery_perc > battery_perc_before) && (battery_perc_before != 0)) battery_perc = battery_perc_before;

       display.fillRect(0, 56, 120, 8, BLACK);
        display.setCursor(0,56);
        display.setTextSize(1);             // Draw 2X-scale text
        display.print(battery_read);
        display.display(); 

    switch (battery_perc) {
    case 100:
        display.fillRect(2, 0, 4, 2, WHITE);

        display.fillRect(0, 2, 8, 12, WHITE);
        display.fillRect(10, 0, 11, 14, BLACK);

        display.drawRect(0, 2,8,12, WHITE);

        display.display();
        break;
    case 75:
        display.fillRect(2, 0, 4, 2, WHITE);

        display.fillRect(10, 0, 11, 14, BLACK);

        display.fillRect(0, 5, 8, 9, WHITE);
        display.fillRect(0, 2, 8, 3, BLACK);

        display.drawRect(0, 2,8,12, WHITE);

        display.display();
        break;
    case 50:
        display.fillRect(2, 0, 4, 2, WHITE);

        display.fillRect(10, 0, 11, 14, BLACK);

        
        display.fillRect(0, 8, 8, 6, WHITE);

        display.fillRect(0, 2, 8, 4, BLACK);

        
        display.drawRect(0, 2,8,12, WHITE);

        display.display();
        break;
    case 25:
        display.fillRect(2, 0, 4, 2, WHITE);

        display.fillRect(10, 0, 11, 14, BLACK);
        display.fillRect(0, 2, 8, 9, BLACK);

        
        display.fillRect(0, 11, 8, 3, WHITE);
        display.drawRect(0, 2,8,12, WHITE);

        display.display();
        break;
  }
battery_perc_before = battery_perc; 
refresh_oled_flag = 0;
}
  
}
}

void low_battery() {
  digitalWrite(9,LOW);
  display.clearDisplay();
  display.setTextSize(2);             // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("    LOW   ");
  display.println("  Battery ");
  display.setTextSize(1);             // Draw 2X-scale text
  display.println("");
  display.println("    Switching Off");
  display.display();
  delay(6000);
  display.clearDisplay();
  display.display();
while (1) {
  
}

}
void flash_brightness(void) {
  bool loop_out = 0;
  
  display.setCursor(0,48);
  display.setTextSize(1);             // Draw 2X-scale text
  display.print("Flash duty cycle:");
  display.setCursor(0,56);
  display.setTextSize(1);             // Draw 2X-scale text
  display.print("0.5");
  display.print("%");
  display.display(); 
  
  delay(1000);
  while (!loop_out) {
    if (digitalRead(6)) {
      //refresh_rpm_on_oled();
    loop_out = 1;
    }

    if (digitalRead(7)) {
       
       delay(200);

       
       switch (brightness) {
       case 1:
       brightness = 2;
       display.fillRect(0, 56, 48, 8, BLACK);
        display.setCursor(0,56);
        display.setTextSize(1);             // Draw 2X-scale text
        display.print("1");
        display.print("%");
        display.display(); 
         
       break;
       case 2:
       brightness = 3;
       display.fillRect(0, 56, 48, 8, BLACK);
        display.setCursor(0,56);
        display.setTextSize(1);             // Draw 2X-scale text
        display.print("1.5");
        display.print("%");
        display.display(); 
       
       break;
       case 3:
       brightness = 4;
       display.fillRect(0, 56, 48, 8, BLACK);
        display.setCursor(0,56);
        display.setTextSize(1);             // Draw 2X-scale text
        display.print("2");
        display.print("%");
        display.display(); 
       
       break;

       case 4:
       brightness = 1;
       display.fillRect(0, 56, 48, 8, BLACK);
        display.setCursor(0,56);
        display.setTextSize(1);             // Draw 2X-scale text
        display.print("0.5");
        display.print("%");
        display.display(); 
       
       break;

       
       }
      
    }
     }
if (loop_out) refresh_rpm_on_oled();
}

