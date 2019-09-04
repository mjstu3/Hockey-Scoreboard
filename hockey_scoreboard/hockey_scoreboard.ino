// Hockey Scoreboard by Matthew Sturgeon 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <CountUpDownTimer.h>

CountUpDownTimer T(DOWN);

int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
decode_results results;

int current_period = 1;
int num_periods = 2;

int minutes = 35;
int seconds = 0;

int score_home = 0;
int score_away = 0;

bool active = false;
bool paused = false;
bool shootout = false;

long stored_button;

#define ch_min 16753245   // period minus
#define ch 16736925       // period switch
#define ch_max 16769565   // period plus
#define prev 16720605     // time minus 5 mins
#define next 16712445     // time plus 5 mins
#define play 16761405     // time play/pause
#define vol_min 16769055  // time minus 1 min
#define vol_max 16754775  // time plus 1 min
#define eq 16748655       // time reset
#define zero 16738455     // scores reset
#define one 16724175      // home score minus
#define two 16718055      // home score plus
#define four 16716015     // away score minus
#define five 16726215     // away score plus
#define hold 4294967295   // long press

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup()
{
  lcd.init();
  lcd.backlight();
  T.SetTimer(0,minutes,seconds);
  refresh_display();

  pinMode(2, OUTPUT);
  pinMode(7, OUTPUT);
  
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  T.Timer();
  if (T.TimeHasChanged()) {
    refresh_display();
  }
  if (T.TimeCheck(0,0,0) == true && active == true) {
    delay(1000);
    T.PauseTimer();
    active = false;
    paused = false;
    refresh_display();
    digitalWrite(2, HIGH);  
    digitalWrite(7, HIGH);   
    delay(250);              
    digitalWrite(7, LOW);    
    delay(250);    
    digitalWrite(7, HIGH);   
    delay(250);                 
    digitalWrite(7, LOW);    
    delay(250); 
    digitalWrite(7, HIGH);   
    delay(250);               
    digitalWrite(7, LOW);    
    delay(250);    
    digitalWrite(7, HIGH);   
    delay(250);  
    digitalWrite(2, LOW);            
    digitalWrite(7, LOW);
    T.SetTimer(0,minutes,seconds);                
  }
  if (irrecv.decode(&results)) {
    Serial.println(results.value, DEC);
    unsigned long value = results.value;
    if (value == hold) {
      value = stored_button;
    }
    if (active == false) {
      switch (value) {
        case ch_min :
          if (shootout == false) {
            if (current_period > 1) {
              current_period--;
            }
          }
          stored_button = ch_min;
          refresh_display();
          break;
        case ch :
          if (num_periods == 2) {
            num_periods = 3;
            shootout = false;
          } else if (num_periods == 3) {
            num_periods = 4;
            shootout = false;
          } else if (num_periods == 4) {
            num_periods = 5;
            shootout = true;
          } else {
            num_periods = 2;
            shootout = false;
          }
          reset_time();
          current_period = 1;
          stored_button = ch;
          refresh_display();
          break;
        case ch_max :
          if (shootout == false) {
            if (current_period < num_periods) {
              current_period++;
            }
          }
          stored_button = ch_max;
          refresh_display();
          break;
        case prev :
          if (shootout == true) {
            if (seconds > 9) {
              seconds -= 5;
            } else {
              seconds = 5;
            }
            T.SetTimer(0,minutes,seconds);
          } else {
            if (minutes > 9) {
              minutes -= 5;
            } else {
              minutes = 5;
            }
            T.SetTimer(0,minutes,seconds);
          }
          stored_button = prev;
          refresh_display();
          break;
        case next :
          if (shootout == true) {
            if (seconds < 26) {
              seconds += 5;
            } else {
              seconds = 30;
            }
            T.SetTimer(0,minutes,seconds);
          } else {
            if (minutes < 51) {
              minutes += 5;
            } else {
              minutes = 55;
            }
            T.SetTimer(0,minutes,seconds);
          }
          stored_button = next;
          refresh_display();
          break;
        case vol_min :
          if (shootout == true) {
            if (seconds > 1) {
              seconds--;
            }
            T.SetTimer(0,minutes,seconds);
          } else {
            if (minutes > 1) {
              minutes--;
            }
            T.SetTimer(0,minutes,seconds);
          }
          stored_button = vol_min;
          refresh_display();
          break;
        case vol_max :
          if (shootout == true) {
            if (seconds < 30) {
              seconds++;
            }
            T.SetTimer(0,minutes,seconds);
          } else {
            if (minutes < 59) {
              minutes++;
            }
            T.SetTimer(0,minutes,seconds);
          }
          stored_button = vol_max;
          refresh_display();
          break;
      }
    }
    switch (value) {
      case play :
        if (active == false) {
          T.StartTimer();
          active = true;
        } else {
          if (paused == false) {
            T.PauseTimer();
            paused = true;
          } else {
            T.ResumeTimer();
            paused = false;
          }
        }
        stored_button = 00000000;
        break;
      case eq :
        T.PauseTimer();
        active = false;
        paused = false;
        current_period = 1;
        reset_time(); 
        stored_button = 00000000;
        refresh_display();
        break;
      case zero :
        score_home = 0;
        score_away = 0;
        stored_button = zero;
        refresh_display();
        break;
      case one :
        if (score_home > 0) {
          score_home--;
        }
        stored_button = one;
        refresh_display();
        break;
      case two :
        if (score_home < 98) {
          score_home++;
        }
        stored_button = two;
        refresh_display();
        break;
      case four :
        if (score_away > 0) {
          score_away--;
        }
        stored_button = four;
        refresh_display();
        break;
      case five :
        if (score_away < 98) {
          score_away++;
        }
        stored_button = five;
        refresh_display();
        break;
    }
    digitalWrite(2, HIGH);   
    digitalWrite(7, HIGH);   
    delay(100);              
    digitalWrite(2, LOW);
    digitalWrite(7, LOW);
    irrecv.resume(); // Receive the next value
  }
}

void reset_time() {
  if (num_periods == 2) {
    seconds = 0;
    minutes = 35;
  } else if (num_periods == 3) {
    seconds = 0;
    minutes = 20;
  } else if (num_periods == 4) {
    seconds = 0;
    minutes = 15;
  } else if (num_periods == 5) {
    seconds = 8;
    minutes = 0;
  }
  T.SetTimer(0,minutes,seconds);
}

void refresh_display() {
  lcd.setCursor(0,0);
  String line1;
  if (shootout == false) {
    if (score_home < 10) {
      line1 = "HOME  " + String(score_home) + "      " + String(current_period) + "/" + String(num_periods);
    } else if (score_home < 100) {
      line1 = "HOME  " + String(score_home) + "     " + String(current_period) + "/" + String(num_periods);
    }
  } else {
    if (score_home < 10) {
      line1 = "HOME  " + String(score_home) + "      S/O";
    } else if (score_home < 100) {
      line1 = "HOME  " + String(score_home) + "     S/O";
    }
  }
  lcd.print(line1);
  
  lcd.setCursor(0,1);
  String line2;
  String minute_text = String(T.ShowMinutes());
  String second_text = String(T.ShowSeconds());
  String millisecond_text = String(T.ShowMilliSeconds());
  if (T.ShowMinutes() < 10) {
    minute_text = "0" + String(T.ShowMinutes());
  }
  if (T.ShowSeconds() < 10) {
    second_text = "0" + String(T.ShowSeconds());
  }
  if (T.ShowMilliSeconds() < 10) {
    millisecond_text = "00" + String(T.ShowMilliSeconds());
  } else if (T.ShowMilliSeconds() < 100) {
    millisecond_text = "0" + String(T.ShowMilliSeconds());
  }
  if (score_away < 10) {
    line2 = "AWAY  " + String(score_away) + "    " + minute_text + ":" + second_text;
  } else if (score_away < 100) {
    line2 = "AWAY  " + String(score_away) + "   " + minute_text + ":" + second_text;
  }  
  lcd.print(line2);
}
