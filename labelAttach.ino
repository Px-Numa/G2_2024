/*ラベル貼り*/
/*
Arduinoで作成した、ラベル貼りのプログラムを記載する。
本体(CADから実装)・プログラム、共に電情の森田一人で担当。
*/

#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_APDS9960.h"
Adafruit_APDS9960 apds;

//画面のサイズの設定
#define SCREEN_WIDTH    (128)
#define SCREEN_HEIGHT   (64)

//画面のサイズ(データシートから)
#define SCREEN_ADDRESS  (0x3C)

Servo myservo;

// ディスプレイ変数の宣言
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);

const int SV_I =    3;    //サーボ1
const int DIR_I  =  4;    //      ステッピング1
const int STEP_I =  5;    //      ステッピング1
const int SV_II =   6;    //サーボ2
const int RELAY =   7;
const int DIR_II  = 8;    //      ステッピング2
const int STEP_II = 9;    //      ステッピング2
const int SV_III =  10;   //サーボ3
const int SW_I =    11;   //                  スイッチ1
const int SW_II =   12;   //                  スイッチ2
const int LED_I =   13;   //      LED1(オンボード)

const int stickAngle = 60;     //貼り付け角度

int mode = 0;

long duration, cm;
int ocm, oocm, ooocm, oooocm, zcm;

int step_angle = 0;
int step_cnt_I = 0;
int step_cnt_II = 0;
int step_cnt_III = 0;
int step_cnt_IV = 0;
int step_cnt = 0;
int step_true = 0;
int servo_angle = 0;
int servo_mp_sign = 1;  //1:+   0:-
int servo_cnt_I = 0;
int servo_cnt_II = 0;
int servo_cnt_III = 0;
int servo_cnt = 0;
int servo_true = 0;
int servo_angle_sel = 0;
int asv_new_angle_I = 0;
int asv_new_angle_II = 0;
int asv_new_angle_III = 0;
int asv_new_angle = 0;
int asv_old_angle = 0;

float outs = 0;

/*
 *  modeについて
 *
 * 0:モード確認        電源投入後、10秒以内にスイッチが押されたらmode10(スタンドアロンmode)にする。10秒経過後はmode1(シールの原点復帰)にする。
 * 1:シールの原点復帰   シールを少し巻き戻し、センサで最初のシールを検出後、最初のシールが貼れる位置まで移動する。移動後はmode2(待機)にする。
 * 2:USB"1"待ち       USB経由で接続されたPCから"1"と言う文字列を9600[bps]で取得するまで待機。取得後はmode3(シール貼り)にする。
 * 3:シール貼り        シールをサーボモータで巻き取り、シール貼りを実行する。貼り付け後はmode4(結果送信)にする。
 * 4:結果を送信        USB経由で接続されたPCへ"2"と言う文字列を9600[bps]で送信する。送信後、mode1にする。
 * 
 *  スタンドアロンモード
 * スイッチ入力を使用して、PC無しの貼り付け実行や、部品単体の動作確認を行う事を想定して作成。
 */

void fled(int f){
  switch(f){
    case 0:   //消灯
      digitalWrite(LED_I, LOW);
      break;
    case 1:   //PC連動モード
      digitalWrite(LED_I, HIGH);
      delay(100);
      digitalWrite(LED_I, LOW);
      delay(50);
      digitalWrite(LED_I, HIGH);
      delay(50);
      digitalWrite(LED_I, LOW);
      break;
    case 2:   //スタンドアロンモード
      digitalWrite(LED_I, HIGH);
      delay(450);
      digitalWrite(LED_I, LOW);
      delay(100);
      digitalWrite(LED_I, HIGH);
      delay(450);
      digitalWrite(LED_I, LOW);
      break;
  }
}

void steppings(int muki,int kazu){
  digitalWrite(RELAY, HIGH);
  if(muki == 0){
    digitalWrite(DIR_I, HIGH);
    digitalWrite(DIR_II, LOW);
  }else if(muki == 1){
    digitalWrite(DIR_I, LOW);
    digitalWrite(DIR_II, HIGH);
  }else{
    digitalWrite(DIR_I, LOW);
    digitalWrite(DIR_II, LOW);
  }
  
  for (int i=0; i<=kazu; i++) {
    digitalWrite(STEP_I, HIGH);
    digitalWrite(STEP_II, HIGH);
    delayMicroseconds(1000);    //1000
    digitalWrite(STEP_I, LOW);
    digitalWrite(STEP_II, LOW);
    delayMicroseconds(1000);    //4000
  }
  digitalWrite(RELAY, LOW);
}

void servo_I(int motor_speed, int kazu) {
  int pulse_width = 1500-motor_speed*8;
  for(int cnt = 0;cnt <= kazu;cnt++){
    digitalWrite(SV_I, HIGH);
    delayMicroseconds(pulse_width);
    digitalWrite(SV_I, LOW);
    delay(20);    
  } 
}

void servo_II(int motor_speed, int kazu) {
  int pulse_width = 1500-motor_speed*8;
  for(int cnt = 0;cnt <= kazu;cnt++){
    digitalWrite(SV_II, HIGH);
    delayMicroseconds(pulse_width);
    digitalWrite(SV_II, LOW);
    delay(20);    
  } 
}

void servos(int motor_speed, int kazu) {
  int pulse_width = 1500-motor_speed*8;
  for(int cnt = 0;cnt <= kazu;cnt++){
    digitalWrite(SV_I, HIGH);
    digitalWrite(SV_II, HIGH);
    delayMicroseconds(pulse_width);
    digitalWrite(SV_I, LOW);
    digitalWrite(SV_II, LOW);
    delay(20);    
  } 
}

void servo_Winding(int kazu){
  int pulse_width = 1500-8*8;
  for(int cnt = 0;cnt < kazu;cnt++){
    if(cnt % 3 == 0){ digitalWrite(SV_I, HIGH);
    digitalWrite(SV_II, HIGH); }
    delayMicroseconds(pulse_width);
    if(cnt % 3 == 0){ digitalWrite(SV_I, LOW);
    digitalWrite(SV_II, LOW); }
  }
}

//準備をする
//1シールの巻き戻し
//2シールの端面検出
//3準備(角度指定サーボの角度を指定。ステッピングモータを原点復帰。)
void preparation(){
  //create some variables to store the color data in

  int k = 0;

  while(1){
    uint16_t r, g, b, c;
    
    //wait for color data to be ready
    while(!apds.colorDataReady()){
      delay(5);
    }
    
    apds.getColorData(&r, &g, &b, &c);
    //Serial.print("c:");
    //Serial.println(c);

    if(c > 1000){
      break;
    }else{
      servo_I(-8,2);

      if(++k > 2){
        k = 0;
        servo_II(-8,1);
      }
    }

    delay(100);
  }
}




//ラベル貼りを実行する
void stickLabels(){
  servo_II(-8, 8);
  servo_I(-10, 8);
  
  uint16_t r, g, b, c;
    
  asv_new_angle = stickAngle;
  for(int i=0;i<asv_new_angle;i++){
    myservo.write(i);
    delay(30);
  }

  myservo.write(asv_new_angle);
  for(int i=0;i<650;i++){
    steppings(0,1);
    if(i%20 == 0){ servo_I(-8, 2); }
    if(i%40 == 0){ servo_II(-8, 2); }
  }
  steppings(0,600);

  for(int i=0;i < 100;i++){
    while(!apds.colorDataReady()){
      delay(5);
    }
    
    if(i%20 == 0){ servo_I(-8, 2); }
    if(i%40 == 0){ servo_II(-8, 2); }

    apds.getColorData(&r, &g, &b, &c);
    if(c > 1000){
      i = 1000;
    }
  }
  
  asv_new_angle = 0;
  for(int i=30;i > 1;i--){
    myservo.write(i);
    delay(50);
  }
  
  myservo.write(asv_new_angle);
  steppings(1,400);

}




赤外線測距センサ
int distance(){
  int inps = analogRead(6);
  if(inps < 82){ inps = 82; }
  float l_t = 0.7;
  outs = l_t * outs + (1 - l_t) * 25391 * pow(inps, -1.136);
  int conv = (int)outs;
  if(conv > 40){ conv = 40; }
  return conv;  
}

//原点復帰用
void returnST(){
  myservo.write(0);
  for(int ai = 0;ai < 10;ai++){
    int kari = distance();
  }

  while(1){
    int kazu_ave = distance();
    if(kazu_ave <= 26){
      break;
    }else{
      steppings(1,10);
    }
  }
}

int op0 = 0;
int opc0 = 9;
void oled_disp_op1(){
  switch(op0){
    case 0:
      display.drawPixel(107, 23, WHITE);
      display.drawPixel(108, 23, WHITE);
      display.drawPixel(109, 23, WHITE);
      break;
    case 1:
      display.drawPixel(110, 23, WHITE);
      display.drawPixel(111, 24, WHITE);
      display.drawPixel(112, 24, WHITE);
      break;
    case 2:
      display.drawPixel(113, 24, WHITE);
      display.drawPixel(114, 25, WHITE);
      display.drawPixel(115, 25, WHITE);
      break;
    case 3:
      display.drawPixel(116, 26, WHITE);
      display.drawPixel(117, 27, WHITE);
      display.drawPixel(118, 28, WHITE);
      break;
    case 4:
      display.drawPixel(119, 29, WHITE);
      display.drawPixel(120, 30, WHITE);
      display.drawPixel(120, 31, WHITE);
      break;
    case 5:
      display.drawPixel(121, 32, WHITE);
      display.drawPixel(121, 33, WHITE);
      display.drawPixel(121, 34, WHITE);
      break;
    case 6:
      display.drawPixel(122, 35, WHITE);
      display.drawPixel(122, 36, WHITE);
      display.drawPixel(122, 37, WHITE);
      break;
    case 7:
      display.drawPixel(122, 38, WHITE);
      display.drawPixel(122, 39, WHITE);
      display.drawPixel(122, 40, WHITE);
      break;
    case 8:
      display.drawPixel(122, 41, WHITE);
      display.drawPixel(121, 42, WHITE);
      display.drawPixel(121, 43, WHITE);
      break;
    case 9:
      display.drawPixel(121, 44, WHITE);
      display.drawPixel(120, 45, WHITE);
      display.drawPixel(120, 46, WHITE);
      break;
    case 10:
      display.drawPixel(119, 47, WHITE);
      display.drawPixel(118, 48, WHITE);
      display.drawPixel(117, 49, WHITE);
      break;
    case 11:
      display.drawPixel(116, 50, WHITE);
      display.drawPixel(115, 51, WHITE);
      display.drawPixel(114, 51, WHITE);
      break;
    case 12:
      display.drawPixel(113, 52, WHITE);
      display.drawPixel(112, 52, WHITE);
      display.drawPixel(111, 52, WHITE);
      break;
    case 13:
      display.drawPixel(110, 53, WHITE);
      display.drawPixel(109, 53, WHITE);
      display.drawPixel(108, 53, WHITE);
      break;
    case 14:
      display.drawPixel(107, 53, WHITE);
      display.drawPixel(106, 53, WHITE);
      display.drawPixel(105, 53, WHITE);
      break;
    case 15:
      display.drawPixel(104, 53, WHITE);
      display.drawPixel(103, 52, WHITE);
      display.drawPixel(102, 52, WHITE);
      break;
    case 16:
      display.drawPixel(101, 52, WHITE);
      display.drawPixel(100, 51, WHITE);
      display.drawPixel(99, 51, WHITE);
      break;
    case 17:
      display.drawPixel(98, 50, WHITE);
      display.drawPixel(97, 49, WHITE);
      display.drawPixel(96, 48, WHITE);
      break;
    case 18:
      display.drawPixel(95, 47, WHITE);
      display.drawPixel(94, 46, WHITE);
      display.drawPixel(94, 45, WHITE);
      break;
    case 19:
      display.drawPixel(93, 44, WHITE);
      display.drawPixel(93, 43, WHITE);
      display.drawPixel(93, 42, WHITE);
      break;
    case 20:
      display.drawPixel(92, 41, WHITE);
      display.drawPixel(92, 40, WHITE);
      display.drawPixel(92, 39, WHITE);
      break;
    case 21:
      display.drawPixel(92, 38, WHITE);
      display.drawPixel(92, 37, WHITE);
      display.drawPixel(92, 36, WHITE);
      break;
    case 22:
      display.drawPixel(92, 35, WHITE);
      display.drawPixel(93, 34, WHITE);
      display.drawPixel(93, 33, WHITE);
      break;
    case 23:
      display.drawPixel(93, 32, WHITE);
      display.drawPixel(94, 31, WHITE);
      display.drawPixel(94, 30, WHITE);
      break;
    case 24:
      display.drawPixel(95, 29, WHITE);
      display.drawPixel(96, 28, WHITE);
      display.drawPixel(97, 27, WHITE);
      break;
    case 25:
      display.drawPixel(98, 26, WHITE);
      display.drawPixel(99, 25, WHITE);
      display.drawPixel(100, 25, WHITE);
      break;
    case 26:
      display.drawPixel(101, 24, WHITE);
      display.drawPixel(102, 24, WHITE);
      display.drawPixel(103, 24, WHITE);
      break;
    case 27:
      display.drawPixel(104, 23, WHITE);
      display.drawPixel(105, 23, WHITE);
      display.drawPixel(106, 23, WHITE);
      display.display();
      delay(5);
      opc0 -= 1;
      op0 = -1;
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("+ start  +");
      display.setTextSize(1);
      display.setCursor(85, 7);
      display.println("...");
      display.setTextSize(3);
      display.setCursor(100, 28);
      display.println(opc0);
      display.setTextSize(1);
      display.setCursor(0, 28);
      display.println("If use");
      display.setCursor(0, 35);
      display.println("standalone mode");
      display.setCursor(0, 42);
      display.println("press the");
      display.setCursor(0, 49);
      display.println("two switches.");
      break;
  }
  display.display();
  delay(1);
  op0 += 1;
}



//液晶の表示内容部
void oled_disp(){
  display.clearDisplay();
  switch(mode){
    case 1:
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("Set up now");
      display.setTextSize(1);
      display.setCursor(7, 28);
      display.println("please wait");
      display.setCursor(7, 38);
      display.println("for complete move.");
      break;
    case 2:
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("+PC  MODE+");
      display.setTextSize(1);
      display.setCursor(7, 28);
      display.println("Waiting for");
      display.setCursor(7, 38);
      display.println("input ""\x22""1""\x22");
      display.setCursor(7, 48);
      display.println("By USB Connected PC.");
      break;
    case 3:
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("+MOVE NOW+");
      display.setTextSize(1);
      display.setCursor(7, 28);
      display.println("Confirmed input \x22""1""\x22");
      display.setCursor(7, 38);
      display.println("Machine in operation");
      break;
    case 4:
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("+MOVE END+");
      display.setTextSize(1);
      display.setCursor(7, 28);
      display.println("for complete move.");
      display.setCursor(7, 38);
      display.println("send \x22""2""\x22");
      display.setCursor(7, 48);
      display.println("for USB Connected PC");
      break;
    case 10:
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("+-Select-+");
      display.setTextSize(1);
      display.setCursor(7, 28);
      display.println("SW1:Automation Mode");
      display.setCursor(7, 38);
      display.println("SW2:Manual Set Mode");
      display.setCursor(7, 52);
      display.println("SW1:Green SW2:Blue");
      break;
    case 11:  //自動貼り付けモード
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      display.println("+--Auto--+");
      display.setTextSize(1);
      display.setCursor(7, 28);
      display.println("SW1:Start Move");
      display.setCursor(7, 42);
      display.println("SW2:Back Menu");
      break;
    case 21:  //手動移動モード   for ステッピング
      display.setTextSize(1);
      display.fillRect(0, 0, 75, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 6);
      display.println("Stepping");
      
      display.drawRect(0, 22, 75, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("Seal Servo");
      
      display.drawRect(0, 44, 75, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Angle Servo");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 22:  //手動移動モード   for サーボ1
      display.setTextSize(1);
      display.drawRect(0, 0, 75, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Stepping");
      
      display.fillRect(0, 22, 75, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 28);
      display.println("Seal Servo");
      
      display.drawRect(0, 44, 75, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Angle Servo");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 23:  //手動移動モード   for サーボ2
      display.setTextSize(1);
      display.drawRect(0, 0, 75, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Stepping");
      
      display.drawRect(0, 22, 75, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("Seal Servo");
      
      display.fillRect(0, 44, 75, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 50);
      display.println("Angle Servo");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 24:  //手動移動モード   for サーボ2
      display.setTextSize(1);
      display.fillRect(0, 0, 75, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 6);
      display.println("Back Menu");
      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 31:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.fillRect(0, 0, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 6);
      display.println("Angle");
      display.setTextColor(WHITE);
      if(step_angle == 0){
        display.setCursor(50, 6);
        display.println("<");
      }else{
        display.setCursor(50, 6);
        display.println(">");
      }
      
      display.drawRect(0, 22, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("Count");
      
      display.drawRect(48, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(50, 28);
      display.println(step_cnt_I);
      display.drawRect(58, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(step_cnt_II);
      display.drawRect(68, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      display.println(step_cnt_III);
      display.drawRect(78, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80, 28);
      display.println(step_cnt_IV);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 32:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.drawRect(0, 0, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Angle");
      display.setTextColor(WHITE);
      if(step_angle == 0){
        display.setCursor(50, 6);
        display.println("<");
      }else{
        display.setCursor(50, 6);
        display.println(">");
      }
      
      display.fillRect(0, 22, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 28);
      display.println("Count");
      
      display.fillRect(48, 26, 9, 11, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(50, 28);
      display.println(step_cnt_I);
      display.drawRect(58, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(step_cnt_II);
      display.drawRect(68, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      display.println(step_cnt_III);
      display.drawRect(78, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80, 28);
      display.println(step_cnt_IV);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 33:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.drawRect(0, 0, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Angle");
      display.setTextColor(WHITE);
      if(step_angle == 0){
        display.setCursor(50, 6);
        display.println("<");
      }else{
        display.setCursor(50, 6);
        display.println(">");
      }
      
      display.fillRect(0, 22, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 28);
      display.println("Count");
      
      display.drawRect(48, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(50, 28);
      display.println(step_cnt_I);
      display.fillRect(58, 26, 9, 11, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(60, 28);
      display.println(step_cnt_II);
      display.drawRect(68, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      display.println(step_cnt_III);
      display.drawRect(78, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80, 28);
      display.println(step_cnt_IV);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 34:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.drawRect(0, 0, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Angle");
      display.setTextColor(WHITE);
      if(step_angle == 0){
        display.setCursor(50, 6);
        display.println("<");
      }else{
        display.setCursor(50, 6);
        display.println(">");
      }
      
      display.fillRect(0, 22, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 28);
      display.println("Count");
      
      display.drawRect(48, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(50, 28);
      display.println(step_cnt_I);
      display.drawRect(58, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(step_cnt_II);
      display.fillRect(68, 26, 9, 11, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(70, 28);
      display.println(step_cnt_III);
      display.drawRect(78, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80, 28);
      display.println(step_cnt_IV);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 35:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.drawRect(0, 0, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Angle");
      display.setTextColor(WHITE);
      if(step_angle == 0){
        display.setCursor(50, 6);
        display.println("<");
      }else{
        display.setCursor(50, 6);
        display.println(">");
      }
      
      display.fillRect(0, 22, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 28);
      display.println("Count");
      
      display.drawRect(48, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(50, 28);
      display.println(step_cnt_I);
      display.drawRect(58, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(step_cnt_II);
      display.drawRect(68, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      display.println(step_cnt_III);
      display.fillRect(78, 26, 9, 11, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(80, 28);
      display.println(step_cnt_IV);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 36:  //手動移動モード   実行 サーボ
      display.setTextSize(1);
      display.drawRect(0, 0, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Angle");
      display.setTextColor(WHITE);
      if(step_angle == 0){
        display.setCursor(50, 6);
        display.println("<");
      }else{
        display.setCursor(50, 6);
        display.println(">");
      }

      display.drawRect(48, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(50, 28);
      display.println(step_cnt_I);
      display.drawRect(58, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(step_cnt_II);
      display.drawRect(68, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      display.println(step_cnt_III);
      display.drawRect(78, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80, 28);
      display.println(step_cnt_IV);
      display.drawRect(0, 22, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("Count");
      
      display.fillRect(0, 44, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 37:  //手動移動モード   実行 サーボ
      display.setTextSize(1);
      display.drawRect(0, 0, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Angle");
      display.setTextColor(WHITE);
      if(step_angle == 0){
        display.setCursor(50, 6);
        display.println("<");
      }else{
        display.setCursor(50, 6);
        display.println(">");
      }

      display.drawRect(48, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(50, 28);
      display.println(step_cnt_I);
      display.drawRect(58, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(step_cnt_II);
      display.drawRect(68, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      display.println(step_cnt_III);
      display.drawRect(78, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80, 28);
      display.println(step_cnt_IV);
      display.drawRect(0, 22, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("Count");
      
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.fillRect(37, 44, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 41:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.fillRect(0, 0, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 6);
      display.println("Count");
      display.setTextColor(WHITE);
      if(servo_angle == 0){
        display.setCursor(50, 6);
        display.println("^ _");
      }else if(servo_angle == 1){
        display.setCursor(50, 6);
        display.println("_ ^");
      }else{
        display.setCursor(50, 6);
        display.println("^ ^");
      }
      
      display.drawRect(0, 22, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("Pulse");
      display.drawRect(48, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(50, 28);
      if(servo_mp_sign == 0){
        display.println("-");
      }else{
        display.println("+");
      }
      display.drawRect(58, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(servo_cnt_I);
      display.drawRect(68, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      display.println(servo_cnt_II);
      display.drawRect(78, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80, 28);
      display.println(servo_cnt_III);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 42:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.drawRect(0, 0, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Count");
      display.setTextColor(WHITE);
      if(servo_angle == 0){
        display.setCursor(50, 6);
        display.println("^ _");
      }else if(servo_angle == 1){
        display.setCursor(50, 6);
        display.println("_ ^");
      }else{
        display.setCursor(50, 6);
        display.println("^ ^");
      }
      
      display.fillRect(0, 22, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 28);
      display.println("Pulse");
      display.fillRect(48, 26, 9, 11, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(50, 28);
      if(servo_mp_sign == 0){
        display.println("-");
      }else{
        display.println("+");
      }
      display.drawRect(58, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(servo_cnt_I);
      display.drawRect(68, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      display.println(servo_cnt_II);
      display.drawRect(78, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80, 28);
      display.println(servo_cnt_III);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 43:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.drawRect(0, 0, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Count");
      display.setTextColor(WHITE);
      if(servo_angle == 0){
        display.setCursor(50, 6);
        display.println("^ _");
      }else if(servo_angle == 1){
        display.setCursor(50, 6);
        display.println("_ ^");
      }else{
        display.setCursor(50, 6);
        display.println("^ ^");
      }
      
      display.fillRect(0, 22, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 28);
      display.println("Pulse");
      display.drawRect(48, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(50, 28);
      if(servo_mp_sign == 0){
        display.println("-");
      }else{
        display.println("+");
      }
      display.fillRect(58, 26, 9, 11, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(60, 28);
      display.println(servo_cnt_I);
      display.drawRect(68, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      display.println(servo_cnt_II);
      display.drawRect(78, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80, 28);
      display.println(servo_cnt_III);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 44:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.drawRect(0, 0, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Count");
      display.setTextColor(WHITE);
      if(servo_angle == 0){
        display.setCursor(50, 6);
        display.println("^ _");
      }else if(servo_angle == 1){
        display.setCursor(50, 6);
        display.println("_ ^");
      }else{
        display.setCursor(50, 6);
        display.println("^ ^");
      }
      
      display.fillRect(0, 22, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 28);
      display.println("Pulse");
      display.drawRect(48, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(50, 28);
      if(servo_mp_sign == 0){
        display.println("-");
      }else{
        display.println("+");
      }
      display.drawRect(58, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(servo_cnt_I);
      display.fillRect(68, 26, 9, 11, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(70, 28);
      display.println(servo_cnt_II);
      display.drawRect(78, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80, 28);
      display.println(servo_cnt_III);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 45:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.drawRect(0, 0, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Count");
      display.setTextColor(WHITE);
      if(servo_angle == 0){
        display.setCursor(50, 6);
        display.println("^ _");
      }else if(servo_angle == 1){
        display.setCursor(50, 6);
        display.println("_ ^");
      }else{
        display.setCursor(50, 6);
        display.println("^ ^");
      }
      
      display.fillRect(0, 22, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 28);
      display.println("Pulse");
      display.drawRect(48, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(50, 28);
      if(servo_mp_sign == 0){
        display.println("-");
      }else{
        display.println("+");
      }
      display.drawRect(58, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(servo_cnt_I);
      display.drawRect(68, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      display.println(servo_cnt_II);
      display.fillRect(78, 26, 9, 11, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(80, 28);
      display.println(servo_cnt_III);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 46:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.drawRect(0, 0, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Count");
      display.setTextColor(WHITE);
      if(servo_angle == 0){
        display.setCursor(50, 6);
        display.println("^ _");
      }else if(servo_angle == 1){
        display.setCursor(50, 6);
        display.println("_ ^");
      }else{
        display.setCursor(50, 6);
        display.println("^ ^");
      }
      
      display.drawRect(0, 22, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("Pulse");
      display.drawRect(48, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(50, 28);
      if(servo_mp_sign == 0){
        display.println("-");
      }else{
        display.println("+");
      }
      display.drawRect(58, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(servo_cnt_I);
      display.drawRect(68, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      display.println(servo_cnt_II);
      display.drawRect(78, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80, 28);
      display.println(servo_cnt_III);
      display.fillRect(0, 44, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 47:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.drawRect(0, 0, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("Count");
      display.setTextColor(WHITE);
      if(servo_angle == 0){
        display.setCursor(50, 6);
        display.println("^ _");
      }else if(servo_angle == 1){
        display.setCursor(50, 6);
        display.println("_ ^");
      }else{
        display.setCursor(50, 6);
        display.println("^ ^");
      }
      
      display.drawRect(0, 22, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("Pulse");
      display.drawRect(48, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(50, 28);
      if(servo_mp_sign == 0){
        display.println("-");
      }else{
        display.println("+");
      }
      display.drawRect(58, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(servo_cnt_I);
      display.drawRect(68, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      display.println(servo_cnt_II);
      display.drawRect(78, 26, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80, 28);
      display.println(servo_cnt_III);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.fillRect(37, 44, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 51:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.fillRect(0, 0, 59, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 6);
      display.println("NEW Angle");
      display.fillRect(58+6, 4, 9, 11, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(60+6, 6);
      display.println(asv_new_angle_I);
      display.drawRect(68+6, 4, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70+6, 6);
      display.println(asv_new_angle_II);
      display.drawRect(78+6, 4, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80+6, 6);
      display.println(asv_new_angle_III);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("OLD Angle");
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(":");
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      if(asv_old_angle < 100){
        display.print("0");
        if(asv_old_angle < 10){
          display.print("0");
          if(asv_old_angle < 0){
            display.print("0");
          }
        }
      }
      display.println(asv_old_angle);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 52:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.fillRect(0, 0, 59, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 6);
      display.println("NEW Angle");
      display.drawRect(58+6, 4, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60+6, 6);
      display.println(asv_new_angle_I);
      display.fillRect(68+6, 4, 9, 11, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(70+6, 6);
      display.println(asv_new_angle_II);
      display.drawRect(78+6, 4, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80+6, 6);
      display.println(asv_new_angle_III);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("OLD Angle");
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(":");
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      if(asv_old_angle < 100){
        display.print("0");
        if(asv_old_angle < 10){
          display.print("0");
          if(asv_old_angle < 0){
            display.print("0");
          }
        }
      }
      display.println(asv_old_angle);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 53:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.fillRect(0, 0, 59, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 6);
      display.println("NEW Angle");
      display.drawRect(58+6, 4, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60+6, 6);
      display.println(asv_new_angle_I);
      display.drawRect(68+6, 4, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70+6, 6);
      display.println(asv_new_angle_II);
      display.fillRect(78+6, 4, 9, 11, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(80+6, 6);
      display.println(asv_new_angle_III);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("OLD Angle");
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(":");
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      if(asv_old_angle < 100){
        display.print("0");
        if(asv_old_angle < 10){
          display.print("0");
          if(asv_old_angle < 0){
            display.print("0");
          }
        }
      }
      display.println(asv_old_angle);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 54:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.drawRect(0, 0, 59, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("NEW Angle");
      display.drawRect(58+6, 4, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60+6, 6);
      display.println(asv_new_angle_I);
      display.drawRect(68+6, 4, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70+6, 6);
      display.println(asv_new_angle_II);
      display.drawRect(78+6, 4, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80+6, 6);
      display.println(asv_new_angle_III);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("OLD Angle");
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(":");
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      if(asv_old_angle < 100){
        display.print("0");
        if(asv_old_angle < 10){
          display.print("0");
          if(asv_old_angle < 0){
            display.print("0");
          }
        }
      }
      display.println(asv_old_angle);
      display.fillRect(0, 44, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(3, 50);
      display.println("Start");
      display.drawRect(37, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
    case 55:  //手動移動モード   実行 ステッピング
      display.setTextSize(1);
      display.drawRect(0, 0, 59, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 6);
      display.println("NEW Angle");
      display.drawRect(58+6, 4, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(60+6, 6);
      display.println(asv_new_angle_I);
      display.drawRect(68+6, 4, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(70+6, 6);
      display.println(asv_new_angle_II);
      display.drawRect(78+6, 4, 9, 11, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(80+6, 6);
      display.println(asv_new_angle_III);
      display.setTextColor(WHITE);
      display.setCursor(3, 28);
      display.println("OLD Angle");
      display.setTextColor(WHITE);
      display.setCursor(60, 28);
      display.println(":");
      display.setTextColor(WHITE);
      display.setCursor(70, 28);
      if(asv_old_angle < 100){
        display.print("0");
        if(asv_old_angle < 10){
          display.print("0");
          if(asv_old_angle < 0){
            display.print("0");
          }
        }
      }
      display.println(asv_old_angle);
      display.drawRect(0, 44, 35, 20, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(3, 50);
      display.println("Start");
      display.fillRect(37, 44, 35, 20, WHITE);
      display.setTextColor(BLACK);
      display.setCursor(46, 50);
      display.println("END");

      display.drawRect(97, 0, 29, 30, WHITE);
      display.setTextColor(WHITE);
      display.setCursor(100, 7);
      display.println("Sele");
      display.setCursor(105, 15);
      display.println("ct");
      display.drawRect(97, 31, 29, 30, WHITE);
      display.setCursor(100, 42);
      display.println("Next");
      break;
  }
  display.display();
}





void setup() {
  pinMode(DIR_I, OUTPUT);
  pinMode(DIR_II, OUTPUT);
  pinMode(STEP_I, OUTPUT);
  pinMode(STEP_II, OUTPUT);
  pinMode(SV_I, OUTPUT);
  pinMode(SV_II, OUTPUT);
  pinMode(SW_I, INPUT);
  pinMode(SW_II, INPUT);
  pinMode(LED_I, OUTPUT);
  pinMode(RELAY, OUTPUT);
  digitalWrite(DIR_I, LOW);
  digitalWrite(DIR_II, LOW);
  digitalWrite(STEP_I, LOW);
  digitalWrite(STEP_II, LOW);
  digitalWrite(LED_I, LOW);
  digitalWrite(RELAY, LOW);
  
  myservo.attach(SV_III, 500, 2400);
  myservo.write(asv_old_angle);

  Serial.begin(9600);

  if(!apds.begin()){
    //Serial.println("failed to initialize device! Please check your wiring.");
  }
  else //Serial.println("Device initialized!");
  //enable color sensign mode
  apds.enableColor(true);
  

  //oled
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  //Serial.println(F("SSD1306 can not allocate memory!"));
  return;
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("+ start  +");
  display.setTextSize(1);
  display.setCursor(85, 7);
  display.println("...");
  display.setTextSize(3);
  display.setCursor(100, 28);
  display.println("9");
  display.setTextSize(1);
  display.setCursor(0, 28);
  display.println("If use");
  display.setCursor(0, 35);
  display.println("standalone mode");
  display.setCursor(0, 42);
  display.println("press the");
  display.setCursor(0, 49);
  display.println("two switches.");
  display.display();
}


void loop() {
  //Serial.print("loop now,mode = ");
  //Serial.println(mode);
  
  switch(mode){
    case 0: //接続待機中.
      while(mode == 0){
        if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){  //SW_I,SW_IIが押
          mode = 10;
          oled_disp();
        }else{
          delay(1);
          oled_disp_op1();
          
          if(opc0 == 0){
            mode = 1;
            oled_disp();
            delay(3000);
          }
        }
      }
      if(mode == 1){ //PC連動モードの場合
        fled(1);
        mode = 1;
      }else{        //スタンドアロンモードの場合
        fled(2);
        mode = 10;
        delay(500);
      }
      break;
    case 1: //ラベルの原点復帰
      //シールを少し巻き戻し、シールの原点を検出する。
      servo_Winding(1000);
      
      
      //1シールの巻き戻し
      //2シールの端面検出
      //3準備(角度指定サーボの角度を指定。ステッピングモータを原点復帰。)
      preparation();
      returnST();
      
      mode = 2;
      oled_disp();
      
      break;
    case 2: //USB待ち
      //PCからシリアル通信経由で"1"が送信されるまで、待機する。取得後はmode3にする。
      char kazu;
      while(mode == 2){
        if ( Serial.available() ) {       // 受信データがあるか？
          Serial.flush();
          kazu = Serial.read();            // 1文字だけ読み込む
          if(kazu == '1'){
            mode = 3;
            oled_disp();
            digitalWrite(13,HIGH);
            delay(1000);
          }else if(kazu == '3'){
            returnST();
          }else if(kazu == '5'){
            steppings(0,200);
          }else if(kazu == '6'){
            steppings(1,200);
          }else if(kazu == '7'){
            asv_new_angle = stickAngle;
            for(int i=0;i<asv_new_angle;i++){
              myservo.write(i);
              delay(30);
            }
          }else if(kazu == '8'){
            asv_new_angle = 0;
            for(int i=55;i>1;i--){
              myservo.write(i);
              delay(30);
            }
          }
        }
      }
      break;
    case 3: //ラベル貼り
      //ラベル貼りを実行する。ラベルが無くなった(=シール貼りできた)事を確認後、mode4に変更
      
      stickLabels();
       
      mode = 4;
      oled_disp(); 
      break;
    case 4:
      //PCに対し、シリアル通信で結果送信("2")を行う。送信後、mode1に戻す。
      delay(1000);
      Serial.println("\n2"); 
      digitalWrite(13,LOW);
      mode = 1;
      break;
    case 10:
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 10;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){    //自動貼り付けモード
        while(digitalRead(SW_I) != LOW)
        delay(200);
        mode = 11;
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){   //手動移動モード
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 21;
        oled_disp(); 
      }else{
        mode = 10;
      }      
      break;
    case 11:
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 11;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);

        //準備
        returnST();
        myservo.write(0);
        servo_Winding(1200);
        preparation();

        stickLabels();
        delay(1000);

        returnST();
         
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 10;
        oled_disp(); 
      }else{
        mode = 11;
      }      
      break;
    case 21:  //Manual設定 - Stepping
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 21;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        mode = 31;
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 22;
        oled_disp(); 
      }else{
        mode = 21;
      }      
      break;
    case 22:  //Manual設定 - Seal Servo
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 22;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        mode = 41;
        servo_true = 0;
        oled_disp();
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 23;
        oled_disp(); 
      }else{
        mode = 22;
      }
      break;
    case 23:  //Manual設定 - Angle Servo
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 23;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        mode = 51;
        oled_disp();
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 24;
        oled_disp(); 
      }else{
        mode = 23;
      }
      break;
    case 24:  //Manual設定 - Angle Servo
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 24;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        mode = 10;
        oled_disp();
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 21;
        oled_disp(); 
      }else{
        mode = 24;
      }
      break;
    case 31:  //ステッピングモータの手動送り出し。指定した向きとパルス数でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 31;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        step_angle ^= 1;
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 32;
        oled_disp(); 
      }else{
        mode = 31;
      }
      break;
    case 32:  //ステッピングモータの手動送り出し。指定した向きとパルス数でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 32;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        if(++step_cnt_I > 9){ step_cnt_I=0; }
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 33;
        oled_disp(); 
      }else{
        mode = 32;
      }
      break;
    case 33:  //ステッピングモータの手動送り出し。指定した向きとパルス数でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 33;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        if(++step_cnt_II > 9){ step_cnt_II=0; }
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 34;
        oled_disp(); 
      }else{
        mode = 33;
      }
      break;
    case 34:  //ステッピングモータの手動送り出し。指定した向きとパルス数でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 34;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        if(++step_cnt_III > 9){ step_cnt_III=0; }
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 35;
        oled_disp(); 
      }else{
        mode = 34;
      }
      break;
    case 35:  //ステッピングモータの手動送り出し。指定した向きとパルス数でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 35;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        if(++step_cnt_IV > 9){ step_cnt_IV=0; }
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 36;
        oled_disp(); 
      }else{
        mode = 35;
      }
      break;
    case 36:  //ステッピングモータの手動送り出し。指定した向きとパルス数でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 36;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        mode = 38;
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 37;
        oled_disp(); 
      }else{
        mode = 36;
      }
      break;
    case 37:  //ステッピングモータの手動送り出し。指定した向きとパルス数でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 37;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        mode = 21;
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 31;
        oled_disp(); 
      }else{
        mode = 37;
      }
      break;
    case 38:  //ステッピングモータの手動送り出し。指定した向きとパルス数でモータ移動、実行
      if(step_cnt == 0){
        if(step_true == 1){
          mode = 31;
          step_true = 0;
          oled_disp(); 
          break;
        }else if(step_true == 0){
          step_cnt = ((step_cnt_I * 1000)+(step_cnt_II * 100)+(step_cnt_III * 10)+(step_cnt_IV * 1));
          step_true = 1;
          //Serial.println(step_cnt);
        }
      }else if(step_cnt != 0){
        step_cnt -= 1;
        steppings(step_angle,1);
      }
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 31;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        mode = 31;
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 31;
        oled_disp(); 
      }else{
        mode = 38;
      }
      break;
    case 41:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 41;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        if(++servo_angle > 2){ servo_angle=0; }
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 42;
        oled_disp(); 
      }else{
        mode = 41;
      }
      break;
    case 42:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 42;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        servo_mp_sign ^= 1;
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 43;
        oled_disp(); 
      }else{
        mode = 42;
      }
      break;
    case 43:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 43;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        if(++servo_cnt_I > 9){ servo_cnt_I=0; }
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 44;
        oled_disp(); 
      }else{
        mode = 43;
      }
      break;
    case 44:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 44;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        if(++servo_cnt_II > 9){ servo_cnt_II=0; }
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 45;
        oled_disp(); 
      }else{
        mode = 44;
      }
      break;
    case 45:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 45;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        if(++servo_cnt_III > 9){ servo_cnt_III=0; }
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 46;
        oled_disp(); 
      }else{
        mode = 45;
      }
      break;
    case 46:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 46;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        mode = 48;
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 47;
        oled_disp(); 
      }else{
        mode = 46;
      }
      break;
    case 47:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 47;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        mode = 22;
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 41;
        oled_disp(); 
      }else{
        mode = 47;
      }
      break;
    case 48:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((servo_cnt == 0) && (servo_true == 1)){
        mode = 41;
        servo_true = 0;
        oled_disp();
        break;
      }else if((servo_cnt == 0) && (servo_true == 0)){
        servo_cnt = ((servo_cnt_I * 100)+(servo_cnt_II * 10)+(servo_cnt_III * 1));
        if(servo_mp_sign == 0){ //マイナスの場合
          servo_cnt = ((servo_cnt) * (-1));
        }
        servo_true = 1;
      }else if(servo_cnt != 0){
        if(servo_mp_sign == 1){
          servo_cnt -= 1;
          servo_angle_sel = -10;
        }else{
          servo_cnt += 1;
          servo_angle_sel = 10;
        }

        if(servo_angle == 0){
          servo_I(servo_angle_sel, 1);
        }else if(servo_angle == 1){
          servo_II(servo_angle_sel, 1);
        }else{
          servos(servo_angle_sel, 1);
        }
      }
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 41;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        mode = 41;
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 41;
        oled_disp(); 
      }else{
        mode = 48;
      }
      break;
    case 51:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 51;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        if(++asv_new_angle_I > 1){ asv_new_angle_I=0; }
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        if((asv_new_angle_I == 1) && (asv_new_angle_II > 8)){
          asv_new_angle_II = 0;
        }
        delay(200);
        mode = 52;
        oled_disp(); 
      }else{
        mode = 51;
      }
      break;
    case 52:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 52;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        if(asv_new_angle_I == 1){
          if(++asv_new_angle_II > 8){ asv_new_angle_II=0; }
        }else{
          if(++asv_new_angle_II > 9){ asv_new_angle_II=0; }
        }
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        if((asv_new_angle_I == 1) && (asv_new_angle_II == 8)){
          asv_new_angle_III = 0;
        }
        mode = 53;
        oled_disp(); 
      }else{
        mode = 52;
      }
      break;
    case 53:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 53;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        if((asv_new_angle_I == 1) && (asv_new_angle_II == 8)){
          asv_new_angle_III = 0;
        }else{
          if(++asv_new_angle_III > 9){ asv_new_angle_III = 0; }
        }
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 54;
        oled_disp(); 
      }else{
        mode = 53;
      }
      break;
    case 54:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 54;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        asv_new_angle = ((asv_new_angle_I*100) + (asv_new_angle_II*10) + (asv_new_angle_III));
        myservo.write(asv_new_angle);
        asv_old_angle = asv_new_angle;
        asv_new_angle = 0;
        asv_new_angle_I = 0;
        asv_new_angle_II = 0;
        asv_new_angle_III = 0;
        mode = 51;
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 55;
        oled_disp(); 
      }else{
        mode = 54;
      }
      break;
    case 55:  //サーボモータの手動送り出し。指定した向き(パルス)と時間でモータ移動
      if((digitalRead(SW_I) == HIGH) && (digitalRead(SW_II) == HIGH)){
        mode = 55;
        delay(500);
      }else if(digitalRead(SW_I) == HIGH){
        while(digitalRead(SW_I) != LOW)
        delay(200);
        mode = 23;
        oled_disp(); 
      }else if(digitalRead(SW_II) == HIGH){
        while(digitalRead(SW_II) != LOW)
        delay(200);
        mode = 51;
        oled_disp(); 
      }else{
        mode = 55;
      }
      break;
  }
}
