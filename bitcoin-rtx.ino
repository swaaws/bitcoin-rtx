/*
  TODO:
    1. print tx from html get to serial - done 
    2. check tx if valid transaction base64/sha256 - skip base64 lib is a pice of shit
    3. save tx on memory - not now
    4. send tx over 80211 raw frame
    5. recive tx over 80211 raw frame
    6. check reachability


  TODO_2:
    1. optimize
    2. opsec
    3. hardening
    4.
    5. production


 TX Fragmentieren:

  1. count           081         3 byte
  2. identifier     aahh         4 byte
  3. index           001         3 byte
                               --------
                                10 byte


 MNMT Fragmentieren:

  1. count           000         3 byte
  2. identifier     aahh         4 byte
  3. type              1         1 byte
                               --------
                                 8 byte

MNMT Types

  1 header
  2 txack
  3


    1 byte = 4 bit
  250 byte - 10 byte = 240 byte
  248 byte netto

*/
#include <DNSServer.h>
#include <esp_now.h>
#include <SPIFFS.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <WebServer.h>

#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "string.h"

String success;
const byte DNS_PORT = 53;
esp_now_peer_info_t peerInfo;
const char *ssid = "bitcoin.local";
uint8_t peeraddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
String responseHTML = ""
                      "<!DOCTYPE html><html><head><title>Submit Bitcoin Transaction</title></head><body>"
                      "<center><h1>Submit Bitcoin Transaction!</h1><p>V0.0.1-Proposal</p>"
                      "<form action=\"/send\" method=\"post\"><input type=\"text\" id=\"tx\" name=\"tx\" minlength=\"0\" maxlength=\"1000\" size=\"100\"><button>Submit</button></form></center></body></html>";


void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    ESP.restart();
  }
}



//block header 1
//          time     target   nonce   blockhight tx-in-pool
//000aahh1.c7f5d74d.f2b9441a.42a14695.7000000000.2000.

//block header 2 hash of previous block
//000aahh2.b6ff0b1b1680a2862a30ca44d346d9e8.
//000aahh3.910d334beb48ca0c0000000000000000.

//block header 3 merkle root
//000aahh4.9d10aa52ee949386ca9385695f04ede2.
//000aahh5.70dda20810decd12bc9b048aaab31471.



// tx ack
//000aahh6.e92e34d497b2e60f1c979549f4121d8e.
//000aahh7.571fc816270ed183a043bbeb0324d061.



//KfkfkfkfkkkjhgbbnnnnnnnnnnnnnnnnnnnnnnnnnnnNnnnnnnnnnnnnnNnnnB //248 // 4 bit //62 
//000aahh1.kjhfekjh43kjh4kjhfheadermemflkjsdlfkjdlfkjflkjflkjfk. //000 aahh 1 [header]

//006aahh001.01000000010000000000000000000000000000000000000000. //006 aahh 001 [tx]
//006abbh002.000000000000000000000000ffffffff0704ffff001d014dff.
//006abbh003.ffffff0100f2052a01000000434104e70a02f5af48a1989bf6.
//006abbh004.30d92523c9d14c45c75f7d1b998e962bff6ff9995fc5bdb44f.
//006abbh005.1793b37495d80324acba7c8f537caaf8432b8d47987313060c.
//006abbh006.c82d8a93ac00000000.

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
 
}
    
void OnDataRecv(const uint8_t *mac, const uint8_t *data, int data_len) {
   char* buff = (char*) data;
  String buffStr = String(buff);
  Serial.println(buffStr);
  

}


IPAddress apIP(10, 0, 0, 1);
DNSServer dnsServer;
WebServer server(80);
int otherbtcwifi = 0;
int n = 0;
// Register peer


void setup() {
  Serial.begin(115200);
  // check <=250bit, check right segmentation, check allowed chars
  // validate tx and move to tx (move also complete validated tx snipets to ready for broadcast tx)
  // 
  //
  // FileStore: TxSnippetsInbox, ValidTx, TxSnippetsOutbox
  // ValideTx: Valide Base64, Valide Signature
  
  xTaskCreate(
                    taskOne,          /* Task function. */
                    "TaskOne",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
 
  xTaskCreate(
                    taskTwo,          /* Task function. */
                    "TaskTwo",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
                    
  n = WiFi.scanNetworks();
  WiFi.softAP(ssid);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  IPAddress myIP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", apIP);
  if (!MDNS.begin("bitcoin")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  MDNS.addService("http", "tcp", 80);
  server.on(F("/"), []() {
    server.send(200, "text/html", responseHTML);
  });
  
  server.on("/send", []() {
    String helloStr = server.arg("tx");
    Serial.println(helloStr);
    // Set values to send
    uint8_t *buffer = (uint8_t*) helloStr.c_str();
    size_t sizeBuff = sizeof(buffer) * helloStr.length();

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(peeraddr, buffer, sizeBuff);

    

    server.send(200, "text/plain", "tx: '" + helloStr + "'");
  });
  server.begin();
  InitESPNow();
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;



  }

  memcpy(peerInfo.peer_addr, peeraddr, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

}

void loop() {
  dnsServer.processNextRequest();


  if (n > 0) {
    for (int i = 0; i < n; ++i) {
      if (WiFi.SSID(i) != "bitcoin.local") {
        otherbtcwifi = 1;
      }
      if (WiFi.SSID(i) == "bitcoin.local") {
        otherbtcwifi = 0;
      }
    }
  } else {
    otherbtcwifi = 1;
  }

  if (otherbtcwifi == 1) {
    server.handleClient();
    delay(2);//1
  }


}

void taskOne( void * parameter )
{
 
    for( int i = 0;i<10;i++ ){
 
        Serial.println("Hello from task 1");
        delay(1000);
    }
 
    Serial.println("Ending task 1");
    vTaskDelete( NULL );
 
}
 
void taskTwo( void * parameter)
{
 
    for( int i = 0;i<10;i++ ){
 
        Serial.println("Hello from task 2");
        delay(1000);
    }
    Serial.println("Ending task 2");
    vTaskDelete( NULL );
 
}



/*


#include <M5StickC.h>
#include <SPIFFS.h>

String file_name = "/M5Stack/notes.txt"; //Sets the location and name of the file to be operated on.  设置被操作的文件位置和名称
bool SPIFFS_FORMAT = true; //Whether to initialize the SPIFFS.  是否初始化SPIFFS
//You don't need to format the flash file system every time you use it.
//无需每次使用闪存都进行格式化

void setup() {
  M5.begin(); //Init M5StickC.  初始化 M5StickC
  M5.Lcd.setRotation(3);  //Rotate the screen.  旋转屏幕
  if(SPIFFS_FORMAT){
    M5.Lcd.println("\nSPIFFS format start...");  //Screen prints format String.  屏幕打印格式化字符串
   // SPIFFS.format();    // Formatting SPIFFS.  格式化SPIFFS
    M5.Lcd.println("SPIFFS format finish");
  }
  if(SPIFFS.begin()){ // Start SPIFFS, return 1 on success.  启动闪存文件系统,若成功返回1
    M5.Lcd.println("\nSPIFFS Started.");
  } else {
    M5.Lcd.println("SPIFFS Failed to Start.");
  }

  if (SPIFFS.exists(file_name)){  //Check whether the file_name file exists in the flash memory.  确认闪存中是否有file_name文件
    M5.Lcd.println("FOUND.");
    M5.Lcd.println(file_name);

    File dataFile = SPIFFS.open(file_name, "a");  // Create a File object dafaFile to add information to file_name in the SPIFFS.  建立File对象dafaFile用于向SPIFFS中的file_name添加信息
    dataFile.println("This is Appended Info."); // Adds string information to dataFile.  向dataFile添加字符串信息
    dataFile.close(); // Close the file when writing is complete.  完成写入后关闭文件
    M5.Lcd.println("Finished Appending data to SPIFFS");
  }else {
    M5.Lcd.println("NOT FOUND.");
    M5.Lcd.print(file_name);
    M5.Lcd.println("is creating.");
    File dataFile = SPIFFS.open(file_name, "w");  // Create aFile object dafaFile to write information to file_name in the SPIFFS.  建立File对象dafaFile用于向SPIFFS中的file_name写入信息
    dataFile.close(); // Close the file when writing is complete.  完成写入后关闭文件
    M5.Lcd.println("Please disable format and Reupload");
  }
}

void loop() {
}



#include <M5StickC.h>
#include <SPIFFS.h>
String file_name = "/M5Stack/notes.txt"; //Sets the location and name of the file to be operated on.  设置被操作的文件位置和名称
bool SPIFFS_FORMAT = false; //Whether to initialize the SPIFFS.  是否初始化SPIFFS
//You don't need to format the flash file system every time you use it.
//无需每次使用闪存都进行格式化

void setup() {
  M5.begin(); //Init M5StickC.  初始化 M5StickC
  M5.Lcd.setRotation(3);  //Rotate the screen.  旋转屏幕
  if(SPIFFS_FORMAT){
    M5.Lcd.println("SPIFFS format start..."); //output format String in LCD.  在屏幕上输出格式化字符串
    SPIFFS.format();    // Formatting SPIFFS.  格式化SPIFFS
    M5.Lcd.println("SPIFFS format finish");
  }

  if(SPIFFS.begin()){ // Start SPIFFS, return 1 on success.  启动闪存文件系统,若成功返回1
    M5.Lcd.println("SPIFFS Begin.");
    //Write operation
    File dataFile = SPIFFS.open(file_name, "w");  // Create a File object dafa  File to write information to file_name in the SPIFFS.  建立File对象dafaFile用于向SPIFFS中的file_name写入信息
    dataFile.println("0100000001c997a5e56e104102fa209c6a852dd90660a20b2d9c352423edce25857fcd3704000000004847304402204e45e16932b8af514961a1d3a1a25fdf3f4f7732e9d624c6c61548ab5fb8cd410220181522ec8eca07de4860a4acdd12909d831cc56cbbac4622082221a8768d1d0901ffffffff0200ca9a3b00000000434104ae1a62fe09c5f51b13905f07f06b99a2f7159b2225f374cd378d71302fa28414e7aab37397f554a7df5f142c21c1b7303b8a0626f1baded5c72a704f7e6cd84cac00286bee0000000043410411db93e1dcdb8a016b49840f8c53bc1eb68a382e97b1482ecad7b148a6909a5cb2e0eaddfb84ccf9744464f82e160bfa9b8b64f9d4c03f999b8643f656b412a3ac00000000"); // Writes string information and newlines to the dataFile.  向dataFile写入字符串信息并换行
    dataFile.close(); // Close the file when writing is complete.  完成写入后关闭文件
    M5.Lcd.println("Finished Writing data to SPIFFS");
  } else {
    M5.Lcd.println("SPIFFS Failed to Begin.\nYou need to Run SPIFFS_Add.ino first");
  }
}

void loop() {
  M5.update();  //Check whether the key is pressed.  检测按键是否按下
  if(M5.BtnA.isPressed()){ //If the button is pressed.  如果按键按下
    
    if (SPIFFS.exists(file_name)){  //Check whether the file_name file exists in the flash memory.  确认闪存中是否有file_name文件
      M5.Lcd.println("FOUND.");
      M5.Lcd.println(file_name);
    } else {
      M5.Lcd.println("NOT FOUND.");
      M5.Lcd.println(file_name);
    }
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    File dataFile = SPIFFS.open(file_name, "r");   // Create aFile object dafaFile to read information to file_name in the SPIFFS.  建立File对象dafaFile用于向SPIFFS中的file_name读取信息
    M5.Lcd.println(dataFile.size());
    for(int i=0; i<dataFile.size(); i++){ //Reads file contents and outputs file information through the M5.Lcd port monitor.  读取文件内容并且通过串口监视器输出文件信息
      M5.Lcd.print((char)dataFile.read());
    }
    dataFile.close(); //Close the file after reading the file.  完成文件读取后关闭文件
    delay(200);
  }
}*/
 */
