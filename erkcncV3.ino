//SD card
#include <SPI.h>
#include <SD.h>

//Key pinout
#define upPin 14
#define downPin 16
#define okPin 15
#define backPin 17

//LCD 1602 library
#include <LiquidCrystal.h>
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;//Pin LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


//Key one press
bool OnePressUp = false;
bool OnePressDown = false;
bool OnePressOk = false;
bool OnePressBack = false;

//menu select
bool processingMenu = false;
bool mainMenu = true;
bool SDcardMenu = false;
bool GrblSettingMenu = false;
bool GrblCommandsMenu = false;
//Main menu ID:1 move menu,2 sd card menu ,3 grbl setting,4 grbl commands...
//Move menu ID:10 move x,11 move y,12 move z,13 speed,14 reset zero,15 returun to zero...
//Move menu-->Moving menu ID:100 move x,101 move y,102 move z,103 speed...
//SD card ID:20
//Grbl setting ID:30 Stepper lock,31 Stepper unlock,32 laser mode,33 cnc mode...
//Grbl commands ID:40 $H Run homing cycle,41 $X Kill alarm lock...
int selectMenu = 1;
//back ID:0 no menu,1 move menu,sd card menu,grbl setting,grbl commands, 2 move menu
byte backMenu = 0;
//limit menyu
byte limitMenuMax = 4;
byte limitMenuMin = 1;

//SD card oxunmasi
//sonuncu id yaddasa veririk
int idFaylSave = 0;
//faylin adini qaytarir
String list = "";
//moveXYZ menyusunda olduqda, menyunuda hereketi baglayiriq
bool selectMenuOff = true;
//axtaris id
int idSecim = 10000;

//fayli aciriq ve setir-setir oxuyuruq
bool runList = true;

//koordinatlar
float MoveXYZ[13] = { -100, -50, -10, -5, -1, -0.1, 0, 0.1, 1, 5, 10, 50, 100};
int SelectMoveX = 6;
int SelectMoveY = 6;
int SelectMoveZ = 6;
//sureti yalniz hereket etdirdikde
int CncSpeed = 50;
//taymer suretli reqem deismesiucun
int taymer = 0;

void setup() {

  //serial
  Serial.begin(115200);

  //sd card ss pin
  SD.begin(10);

  //Lcd
  lcd.begin(16, 2);

  //key up,down,ok,back
  pinMode(upPin, INPUT);//up
  pinMode(downPin, INPUT);//down
  pinMode(okPin, INPUT);//ok
  pinMode(backPin, INPUT);//back


  //giris menu
  lcd.setCursor(0, 0);
  lcd.print(F("Grbl v1.1 tested"));
  lcd.setCursor(0, 1);
  lcd.print(F("ERK LCD v3"));
  delay(2000);//2 san gozledikden sonra
  //main menu kecid
  LCDclear();
  menuText(selectMenu);

}

void loop() {

  //----------------------UP key press------------------------
  if (key(upPin) == true and OnePressUp == true) {

    if (selectMenuOff == true) {

      selectMenu--;//select menu

      if (selectMenu <= limitMenuMin) { //menu select, up limit

        selectMenu = limitMenuMin;

      }

    }

    switch (selectMenu) {

      case 100://moveX menu
        SelectMoveX--;
        if (SelectMoveX <= 0) {
          SelectMoveX = 0;
        }
        break;
      case 101://moveY menu
        SelectMoveY--;
        if (SelectMoveY <= 0) {
          SelectMoveY = 0;
        }
        break;
      case 102://moveZ menu
        SelectMoveZ--;
        if (SelectMoveZ <= 0) {
          SelectMoveZ = 0;
        }
        break;

      case 103://Speed
        if (CncSpeed > 100) {
          CncSpeed -= 100;
        } else {
          CncSpeed -= 10;
        }

        if (CncSpeed <= 10) {
          CncSpeed = 10;
        }
        break;

      case 20://SD card menu
        idSecim--;
        if (idSecim <= 10000) {//limit
          idSecim = 10000;
        }

        FileRead(idSecim);
        menuText(20);
        break;

    }

    menuText(selectMenu);

    OnePressUp = false;

  }

  //----------------------DOWN key press---------------------
  if (key(downPin) == true and OnePressDown == true) {

    if (selectMenuOff == true) {

      selectMenu++;//select menu

      if (selectMenu >= limitMenuMax) {//menu select, down limit

        selectMenu = limitMenuMax;

      }

    }

    switch (selectMenu) {

      case 100://moveX menu
        SelectMoveX++;
        if (SelectMoveX >= 12) {
          SelectMoveX = 12;
        }
        break;
      case 101://moveY menu
        SelectMoveY++;
        if (SelectMoveY >= 12) {
          SelectMoveY = 12;
        }
        break;
      case 102://moveZ menu
        SelectMoveZ++;
        if (SelectMoveZ >= 12) {
          SelectMoveZ = 12;
        }
        break;

      case 103://Speed

        if (CncSpeed < 100) {
          CncSpeed += 10;
        } else {
          CncSpeed += 100;
        }

        if (CncSpeed >= 4000) {
          CncSpeed = 4000;
        }
        break;

      case 20://SD card menu
        idSecim++;
        if (idSecim >= idFaylSave) {//limit
          idSecim = idFaylSave;
        }

        FileRead(idSecim);
        menuText(20);
        break;

    }

    menuText(selectMenu);

    OnePressDown = false;

  }

  //----------------------OK key press---------------------
  if (key(okPin) == true and OnePressOk == true) {


    switch (selectMenu) {

      case 1://move menu
        selectMenu = 10;
        limitMenuMax = 15;
        limitMenuMin = 10;
        backMenu = 1;
        break;
      case 2://SD card menu
        FileSave();
        FileRead(idSecim);
        selectMenuOff = false;
        selectMenu = 20;
        backMenu = 2;
        break;
      case 3://Grbl setting menu
        selectMenu = 30;
        limitMenuMax = 33;
        limitMenuMin = 30;
        backMenu = 3;
        break;
      case 4://Grbl commands menu
        selectMenu = 40;
        limitMenuMax = 41;
        limitMenuMin = 40;
        backMenu = 4;
        break;
      case 10://move x menu
        selectMenuOff = false; //hereket menyusunu baglayiriq
        selectMenu = 100;
        limitMenuMax = 100;
        limitMenuMin = 100;
        backMenu = 100;
        break;
      case 11://move y menu
        selectMenuOff = false; //hereket menyusunu baglayiriq
        selectMenu = 101;
        limitMenuMax = 101;
        limitMenuMin = 101;
        backMenu = 101;
        break;
      case 12://move z menu
        selectMenuOff = false; //hereket menyusunu baglayiriq
        selectMenu = 102;
        limitMenuMax = 102;
        limitMenuMin = 102;
        backMenu = 102;
        break;
      case 13://speed menu
        selectMenuOff = false; //hereket menyusunu baglayiriq
        selectMenu = 103;
        limitMenuMax = 103;
        limitMenuMin = 103;
        backMenu = 103;
        break;
      case 14://reset zero
        Serial.print("G10P0L20X0Y0Z0");
        Serial.println();
        ReadOk();
        delay(100);
        break;
      case 15://return to zero
        Serial.println("G90G0X0Y0");
        Serial.println("G90G0Z0");
        ReadOk();
        delay(100);
        break;
      case 20://SD card menu
        FileRun();
        break;
      case 30://grbl setting stepper on
        Serial.print("$1=255");
        Serial.println();
        ReadOk();
        delay(100);
        break;
      case 31://grbl setting stepper off
        Serial.print("$1=25");
        Serial.println();
        ReadOk();
        delay(100);
        break;
      case 32://grbl setting laser mode
        Serial.print("$32=1");
        Serial.println();
        ReadOk();
        delay(100);
        break;
      case 33://grbl setting cnc mode
        Serial.print("$32=0");
        Serial.println();
        ReadOk();
        delay(100);
        break;
      case 40://grbl commands $H home
        Serial.print("$H");
        Serial.println();
        ReadOk();
        delay(100);
        break;
      case 41://grbl commands $X Kill alarm lock
        Serial.print("$X");
        Serial.println();
        ReadOk();
        delay(100);
        break;
      case 100://move X
        Serial.print("$J=G21G91X");//hereket komndasinin gonderilmesi
        Serial.print(MoveXYZ[SelectMoveX]);
        Serial.print("F");
        Serial.print(CncSpeed);
        Serial.println();
        ReadOk();
        delay(100);
        break;
      case 101://move Y
        Serial.print("$J=G21G91Y");//hereket komndasinin gonderilmesi
        Serial.print(MoveXYZ[SelectMoveY]);
        Serial.print("F");
        Serial.print(CncSpeed);
        Serial.println();
        ReadOk();
        delay(100);
        break;
      case 102://move Z
        Serial.print("$J=G21G91Z");//hereket komndasinin gonderilmesi
        Serial.print(MoveXYZ[SelectMoveZ]);
        Serial.print("F");
        Serial.print(CncSpeed);
        Serial.println();
        ReadOk();
        delay(100);
        break;

    }

    menuText(selectMenu);

    OnePressOk = false;

  }

  //----------------------BACK key press---------------------
  if (key(backPin) == true and OnePressBack == true) {

    switch (backMenu) {

      case 1://main menu move
        idSecim = 10000;
        selectMenu = 1;
        limitMenuMax = 4;
        limitMenuMin = 1;
        backMenu = 0;
        break;
      case 2://main menu sd
        idSecim = 10000;
        selectMenu = 2;
        limitMenuMax = 4;
        limitMenuMin = 1;
        backMenu = 0;
        selectMenuOff = true;
        break;
      case 3://main menu grbl setting
        idSecim = 10000;
        selectMenu = 3;
        limitMenuMax = 4;
        limitMenuMin = 1;
        backMenu = 0;
        break;
      case 4://main menu grbl commands
        idSecim = 10000;
        selectMenu = 4;
        limitMenuMax = 4;
        limitMenuMin = 1;
        backMenu = 0;
        break;
      case 100://move menu x
        selectMenuOff = true; //hereket menyusunu aciriq
        selectMenu = 10;
        limitMenuMax = 15;
        limitMenuMin = 10;
        backMenu = 1;
        break;
      case 101://move menu y
        selectMenuOff = true; //hereket menyusunu aciriq
        selectMenu = 11;
        limitMenuMax = 15;
        limitMenuMin = 10;
        backMenu = 1;
        break;
      case 102://move menu z
        selectMenuOff = true; //hereket menyusunu aciriq
        selectMenu = 12;
        limitMenuMax = 15;
        limitMenuMin = 10;
        backMenu = 1;
        break;
      case 103://move menu speed
        selectMenuOff = true; //hereket menyusunu aciriq
        selectMenu = 13;
        limitMenuMax = 15;
        limitMenuMin = 10;
        backMenu = 1;
        break;

    }

    menuText(selectMenu);

    OnePressBack = false;

  }

  //-----------------------NO key-----------------------------
  if (key(downPin) == false and OnePressDown == false) {

    taymer = 0;
    OnePressDown = true;

  }

  if (key(upPin) == false and OnePressUp == false) {

    OnePressUp = true;

  }

  if (key(okPin) == false and OnePressOk == false) {

    OnePressOk = true;

  }

  if (key(backPin) == false and OnePressBack == false) {

    OnePressBack = true;

  }




}//loop end


//----------------Key function----------------------
bool key(byte Button) {

  bool buttonRead = digitalRead(Button);
  delay(10);

  if (buttonRead == HIGH ) {

    return true;

  }

  if (buttonRead == LOW ) {

    return false;

  }

}
//---------------Menu text-------------------------
void menuText(int textSelect) {

  LCDclear();

  switch (textSelect) {

    //MAIN MENU TEXT
    case 1:
      lcd.setCursor(0, 0);
      lcd.print(F("Main menu"));
      lcd.setCursor(0, 1);
      lcd.print(F("Move"));
      break;
    case 2:
      lcd.setCursor(0, 0);
      lcd.print(F("Main menu"));
      lcd.setCursor(0, 1);
      lcd.print(F("SD Card"));
      break;
    case 3:
      lcd.setCursor(0, 0);
      lcd.print(F("Main menu"));
      lcd.setCursor(0, 1);
      lcd.print(F("Grbl setting"));
      break;
    case 4:
      lcd.setCursor(0, 0);
      lcd.print(F("Main menu"));
      lcd.setCursor(0, 1);
      lcd.print(F("Grbl commands"));
      break;

    //MOVE MENU TEXT
    case 10:
      lcd.setCursor(0, 0);
      lcd.print(F("Move menu"));
      lcd.setCursor(0, 1);
      lcd.print(F("Move X"));
      break;
    case 11:
      lcd.setCursor(0, 0);
      lcd.print(F("Move menu"));
      lcd.setCursor(0, 1);
      lcd.print(F("Move Y"));
      break;
    case 12:
      lcd.setCursor(0, 0);
      lcd.print(F("Move menu"));
      lcd.setCursor(0, 1);
      lcd.print(F("Move Z"));
      break;
    case 13:
      lcd.setCursor(0, 0);
      lcd.print(F("Move menu"));
      lcd.setCursor(0, 1);
      lcd.print(F("Speed"));
      break;
    case 14:
      lcd.setCursor(0, 0);
      lcd.print(F("Move menu"));
      lcd.setCursor(0, 1);
      lcd.print(F("Reset zero"));
      break;
    case 15:
      lcd.setCursor(0, 0);
      lcd.print(F("Move menu"));
      lcd.setCursor(0, 1);
      lcd.print(F("Return to zero"));
      break;

    //SD CARD MENU
    case 20:
      lcd.setCursor(0, 0);
      lcd.print(F("SD Card"));
      lcd.setCursor(0, 1);
      lcd.print(list);
      break;

    //GRBL SETTING TEXT
    case 30:
      lcd.setCursor(0, 0);
      lcd.print(F("Grbl setting"));
      lcd.setCursor(0, 1);
      lcd.print(F("Stepper on"));
      break;
    case 31:
      lcd.setCursor(0, 0);
      lcd.print(F("Grbl setting"));
      lcd.setCursor(0, 1);
      lcd.print(F("Stepper off"));
      break;
    case 32:
      lcd.setCursor(0, 0);
      lcd.print(F("Grbl setting"));
      lcd.setCursor(0, 1);
      lcd.print(F("Laser mode"));
      break;
    case 33:
      lcd.setCursor(0, 0);
      lcd.print(F("Grbl setting"));
      lcd.setCursor(0, 1);
      lcd.print(F("Cnc mode"));
      break;

    //GRBL COMMANDS
    case 40:
      lcd.setCursor(0, 0);
      lcd.print(F("Grbl commands"));
      lcd.setCursor(0, 1);
      lcd.print(F("$H Home"));
      break;
    case 41:
      lcd.setCursor(0, 0);
      lcd.print(F("Grbl commands"));
      lcd.setCursor(0, 1);
      lcd.print(F("$X alarm lock"));
      break;

    //MOVING XYZ MENU
    case 100:
      lcd.setCursor(0, 0);
      lcd.print(F("Move X mm"));
      lcd.setCursor(0, 1);
      lcd.print(MoveXYZ[SelectMoveX]);
      break;
    case 101:
      lcd.setCursor(0, 0);
      lcd.print(F("Move Y mm"));
      lcd.setCursor(0, 1);
      lcd.print(MoveXYZ[SelectMoveY]);
      break;
    case 102:
      lcd.setCursor(0, 0);
      lcd.print(F("Move Z mm"));
      lcd.setCursor(0, 1);
      lcd.print(MoveXYZ[SelectMoveZ]);
      break;
    case 103:
      lcd.setCursor(0, 0);
      lcd.print(F("Speed mm/min"));
      lcd.setCursor(0, 1);
      lcd.print(CncSpeed);
      break;

  }

}

//-----------------LCD clear------------------------
void LCDclear() {

  lcd.setCursor(0, 0);
  lcd.print(F("                "));
  lcd.setCursor(0, 1);
  lcd.print(F("                "));

}

//--------------------------------------------Kartda olan fayllari yaddasa verir--------------------------------------
//Papkalari ve ya esas directoriya "/" acir ve save.erk fayla yazir
void FileSave() {

  int idFayl = 10000;//katalogdaki fayllari ve papkalari nomereleyirik, axtaris ucun

  //Sd card yeni fayl atdiqda ve ya papkalara daxil olduqda melumatlarin yeniden yazilmasi ucun
  if (SD.exists("save.erk") == true) {//eger yaddasda belebir fayl varsa pozuruq

    SD.remove("save.erk");

  }

  File fileOpen = SD.open("/", FILE_READ); //kataloqu aciriq "/" esas kataloq

  while (true) {//dovr yaradiriq

    File fileEnter =  fileOpen.openNextFile();//kataloqda olan fayl ve papkalari secirik.

    if (fileEnter != true) { //secim bitdikde false qaytarir

      break;

    }

    String text = fileEnter.name();//fayllarin ve pakalarin adini yadda saxlayiriq

    text.toLowerCase();//asagi reqistrda yaziriq

    if (text != "save.erk") {//save.erk fayla yazilmir

      File textSave = SD.open ("save.erk", FILE_WRITE); //fayli aciriq yazmaq ucun
      //Qeyd:println() fayllari yaddasa yazir ve sonu (ASCII 13) /r novbeti setir,setrin sonu /0 ile bitirir
      textSave.print(idFayl); //katalogdaki fayllari ve papkalari nomereleyirik, axtaris ucun
      textSave.println(text); //fayllarin ve papkalarin adini yaziriq

      //QEYD File baslayan klassi .close(); baglamaq lazimdi eks halda sram dolur ve
      //arduino donur
      textSave.close();//fayli baglayiriq
      //Serial.println(text);//yoxlamaq ucun
    }

    fileEnter.close();//fayli baglayiriq
    idFaylSave = idFayl;//sonuncu id yaddasa veririk
    idFayl++;//ardicil nomreleme

  }

  fileOpen.close();//fayli baglayiriq

}

//---------------------------------------------------yaddasa verilmis fayillarin adini qaytarir----------------------------------------------
void FileRead(int idFaylSecim) {

  File LsdText = SD.open("save.erk", FILE_READ);//fayli aciriq

  if (LsdText == true) {

    while (true) {//dovr yaradiriq

      int setir;
      setir = LsdText.parseInt();

      if (setir == idFaylSecim) {

        list = LsdText.readStringUntil('\r');

        break;

      }

    }

  }

  LsdText.close();

}

//----------------------------------------------ok cavabi qaytarir ekrana vermek ucun-------------------------------

void ReadOk() {

  char getOk = ' ';

  while (true) {//dovr yaradiriq

    getOk = Serial.read();//ok yoxlayiriq grbl her yerine yeten komandada ok gonderir

    if (getOk == 'o') {
      getOk = ' ';
      lcd.setCursor(14, 0);
      lcd.print(F("ok"));
      delay(100);
      break;//ok gordukde dovrden cixiriq

    }

  }

}

//----------------------------------------------ok cavabi qaytarir---------------------------------------------------

void fun_ok() {

  char get_ok = "";

  while (true) {//dovr yaradiriq

    get_ok = Serial.read();//ok yoxlayiriq grbl her yerine yeten komandada ok gonderir

    if (get_ok == 'o') {
      get_ok = ' ';
      runList = true;
      break;//ok gordukde dovrden cixiriq

    }

  }

}



//---------------------------------------------------------secilmis fayli aciriq ve ise saliriq----------------------------------------
void FileRun() {

  File FileReadRun = SD.open(list);//fayli aciriq

  unsigned long vaxt_save = FileReadRun.size();
  unsigned long vaxt = vaxt_save;
  unsigned long  vaxt_bitme = 0;
  int setir_l = 0;
  String l_line = "";
  selectMenu = 200;//Ok menyulari baglayiriq
  selectMenuOff = false; //hereket menyusunu baglayiriq
  backMenu = 0; //geri hereket baglayiriq
  LCDclear();//ekrani temizleyirem

  if (FileReadRun.available() > 0) { //faylda melumat varsa

    lcd.setCursor(0, 0);
    lcd.print(list);//faylin adi
    lcd.setCursor(0, 1);
    lcd.print(F("Preparing..."));//hazir olma vaxti
    delay(4000);
    runList = true;
    lcd.setCursor(0, 1);
    lcd.print(F("Processing.."));//is gedir

    while (FileReadRun.available()) { //faylda melumatlar bitene kimi

      if (runList == true) {//setiri oxuyuruk

        l_line = FileReadRun.readStringUntil('\n');
        Serial.print(l_line);//melumatlarin grbl gonderilmesi
        l_line.trim();//setrin evvelinde ve axrinda probelleri gotururuk

        //faylin hecmi ile simvollarin sayi duz gelmir (simvollarin sayi= faylin hecmi-(setirlerinsayi*2))
        //faizi duzgun hesablamaq ucun
        setir_l = int( l_line.length()); //her setirde olan simvol sayini qaytarir
        vaxt = vaxt - (setir_l + 2);

        vaxt_bitme = 100 - ((vaxt * 100) / vaxt_save); //faizle
        lcd.setCursor(12, 1);
        lcd.print(vaxt_bitme);
        lcd.print(F("%"));


        runList = false;//serti baglayiriq ve novbeti ok gozleyirik

      }

      fun_ok();

    }

    lcd.setCursor(0, 1);
    lcd.print(F("Please wait...  "));//son
    delay(2000);

    //go to main menu
    idSecim = 10000;
    limitMenuMax = 4;
    limitMenuMin = 1;
    backMenu = 0;
    selectMenuOff = true; //hereket menyusunu aciriq
    selectMenu = 1;

  } else {

    lcd.setCursor(0, 0);
    lcd.print(F("File is empty   "));//fayl bosdu
    lcd.setCursor(0, 1);
    lcd.print(F("Please wait...  "));//son
    delay(2000);

    //go to main menu
    idSecim = 10000;
    limitMenuMax = 4;
    limitMenuMin = 1;
    backMenu = 0;
    selectMenuOff = true; //hereket menyusunu aciriq
    selectMenu = 1;

  }

  FileReadRun.close();//fayli baglayiriq

}
