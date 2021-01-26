#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WebSocketsServer.h>
#include <BlynkSimpleEsp8266.h>
#include <SoftwareSerial.h>
SoftwareSerial SIM900(5,4); //D1-->Rx, D2-->Tx
//================AP===============//
String ssid_ap = "QUAN1";
String pass_ap = "123456789";
String cdhd_ap = "1";
String sodt_send="";
String sms_send="";
String cdhdsim900 = "0";
boolean lockstate=0;
boolean canhbaostate=0;
boolean sensorState=LOW;
String bufferSIM900 = "";
int Index_Rxdata = -1;
IPAddress ip_ap(192,168,1,1);
IPAddress gateway_ap(192,168,1,1);
IPAddress subnet_ap(255,255,255,0);
WebSocketsServer webSocket = WebSocketsServer(81);
BlynkTimer timer_update;
BlynkTimer timer_canhbao;
boolean ledconnect;
//==============WEB SERVER=========//
ESP8266WebServer webServer(80);
const char MainPage[] PROGMEM = R"=====(
  <!DOCTYPE html> 
  <html>
   <head> 
       <title>QG-SMART HOME</title> 
       <style> 
          body {
            text-align:center;
            background-color:#222222; 
            color:white
          }
          input {
            height:25px; 
            width:270px;
            font-size:20px;
            margin: 10px auto;
          }
          #cdhdwifi input {
            height:25px; 
            width:25px;
            font-size:15px;
            margin: 10px 10px;
          }
          #content {
            border: white solid 1px; 
            padding:5px;  
            //height:380px; 
            width:330px; 
            border-radius:20px;
            margin: 0 auto;
          }
          #ledconnect{
            outline: none;
            margin: 10px 5px -1px 5px;
            width: 15px;
            height: 15px;
            border: solid 1px #FFFFFF;
            background-color: #00EE00;
            border-radius: 50%;
            -moz-border-radius: 50%;
            -webkit-border-radius: 50%;
          }
          .button_setup {
            height:30px; 
            width:280px; 
            margin: 5px 0;
            border: solid 1px white;
            background-color:#222222;
            border-radius:5px;
            outline:none;
            color:white;
            font-size:15px;
          }
          .button_wifi{
            height:50px; 
            width:90px; 
            margin:5px 0;
            outline:none;
            color:white;
            font-size:15px;
            font-weight: bold;
          }
          #wifisetupap{
            height:410px; 
            font-size:20px; 
            display:none;
          }
          #button_save {
            background-color:#00BB00;
            border-radius:5px;
          }
          #button_restart {
            background-color:#FF9900;
            border-radius:5px;
          }
          #button_reset {
            background-color:#CC3300;
            border-radius:5px;
          }
          #button_lock{
            width: 180px;
            height: 60px;
            border-radius: 10px;
            background-color: blue;
            outline:none;
            font-weight: bold;
            color: white;
            font-size: 25px;
          }
          #iconlock {
            outline: none;
            width: 180px;
            height: 180px;
            background-color: #222222;
            position: relative;
          }
          .iconlock_right{
            outline: none;
            width: 90px;
            height: 180px;
            background-color: yellow;
            border-radius: 0 90px 90px 0;   
            position: absolute;         
            left: 90px;
            top: -10px;
          }
          .iconlock_left{
            outline: none;
            width: 90px;
            height: 180px;
            background-color: #00BB00;
            border-radius: 90px 0 0 90px;
            position: absolute;
            top: -10px;
            left: 0px;
          }
          #iconunlock {
            outline: none;
            width: 180px;
            height: 180px;
            background-color: #222222;
            margin: 0 auto;
            position: relative;
          }
          .iconunlock_right{
            outline: none;
            width: 90px;
            height: 180px;
            background-color: yellow;
            border-radius: 0 90px 90px 0;   
            position: absolute;         
            left: 100px;
            top: -10px;
          }
          .iconunlock_left{
            outline: none;
            width: 90px;
            height: 180px;
            background-color: #00BB00;
            border-radius: 90px 0 0 90px;
            position: absolute;
            top: -10px;
            left: -10px;
          }
       </style>
       <meta name="viewport" content="width=devicQG-width,user-scalable=0" charset="UTF-8">
   </head>
   <body> 
      <div id="content"> 
        <div id="homecontrol" style="height:410px; display: block">
          <div style="font-size: 30px; font-weight: bold">QG-SMART HOME</div>
          <div style="position: relative;width: 180px; height: 160px; margin: 30px auto;">
            <div id="iconlock" style="display: block">
              <input type="button" class="iconlock_right"/>
              <input type="button" class="iconlock_left"/>
            </div>
            <div id="iconunlock" style="display: none">
              <input type="button" class="iconunlock_right"/>
              <input type="button" class="iconunlock_left"/>
            </div>
          </div>
          <div>
            <input type="button" id="button_lock" value="UNLOCK" onclick="lockactive()" />
          </div>
          <div>
            <button class="button_setup" onclick="configurewifiap()">WIFI CONFIGURE</button>
            <button class="button_setup" onclick="configuresim900a()">SIM900A CONFIGURE</button>
          </div>
        </div>
        <div id="wifisetupap" style="display: none">
          <div style="font-size: 30px; font-weight: bold">QG-SMART HOME</div>
          <div style="text-align:left; width:270px; margin:0px 25px">SSID Access point: </div>
          <div><input id="ssidap"/></div>
          <div style="text-align:left; width:270px; margin:0px 25px">Password: </div>
          <div><input id="passap"/></div>
          <div style="text-align:left;width:270px; margin:0px 25px">Operation Mode:</div>
          <div style="text-align:left; height: 50px; margin-left: 20px" id="cdhdwifi">
            <div style="width: 130px; float: left"><input name="cdhdwifi" type="radio" value="1" checked/>show</div>     
            <div style="width: 130px; float: left"><input name="cdhdwifi" type="radio" value="0"/>hide</div>
          </div>
          <div>
            <button id="button_save" class="button_wifi" onclick="writeEEPROMap()">SAVE</button>
            <button id="button_restart" class="button_wifi" onclick="restartESP()">RESTART</button>
            <button id="button_reset" class="button_wifi" onclick="clearEEPROM()">RESET</button>
          </div>
          <div>
            <input class="button_setup" type="button" onclick="backHOME()" value="BACK HOME"/>
          </div>
        </div>
        <div id="sim900asetup" style="height: 410px">
          <div style="font-size: 30px; font-weight: bold">QG-SMART HOME</div>
          <div>
            <div style="text-align:left; width:270px; margin:0px 25px">Phone number: </div>
            <div><input id="sodt_send" type="tel" maxlength='10' /></div>
            <div style="text-align:left; width:270px; margin:10px 25px">Content of the message sent: (<pan id="count"></pan>)</div>
            <div style="width:270px; margin:10px 25px"><textarea id="sms_send" rows="5" cols="34" maxlength='160'></textarea></div>
          </div>
          <div style="width: 270px; height: 50px; margin-left: 30px">
            <div style="text-align: left;">Operation Mode:</div>
            <div style="width: 130px;float: left"><input style="height: 25px; width: 25px;" type="radio" name="cdhdsim900a" checked="checked" value="0">Call Mobile</div>
            <div style="width: 130px;float: left"><input style="height: 25px; width: 25px" type="radio" name="cdhdsim900a" value="1">Send SMS</div>
          </div>
          <div >
            <button style="background-color: #00BB00; height: 50px; width: 135px;border-radius: 5px;color: white;font-weight: bold;margin-right: 5px" onclick="writeEEPROMsim900a()">SAVE CONFIG</button>
            <button style="background-color: blue; height: 50px; width: 135px;border-radius: 5px;color: white;font-weight: bold" onclick="test_sim900a()">TEST SIM900A</button>
          </div>
          <div>
            <input class="button_setup" type="button" onclick="backHOME()" value="BACK HOME"/>
          </div>
        </div>
        <div><input id="ledconnect" type="button"/>Connection status</div>
      </div>
      <div id="footer">
        
          Email: <b>dangquan2909@gmail.com</b></i>
        </p>
      </div>
      <script>
        window.onload = function(){
          init();
          document.getElementById("homecontrol").style.display = "block";
          document.getElementById("wifisetupap").style.display = "none";
          document.getElementById("sim900asetup").style.display = "none";
        }
        //-----------Hàm khởi tạo đối tượng request----------------
        function create_obj(){
          td = navigator.appName;
          if(td == "Microsoft Internet Explorer"){
            obj = new ActiveXObject("Microsoft.XMLHTTP");
          }else{
            obj = new XMLHttpRequest();
          }
          return obj;
        }
        //===========Configure WiFi=====================================
        var xhttp = create_obj();
        function configurewifiap(){
          document.getElementById("homecontrol").style.display = "none";
          document.getElementById("wifisetupap").style.display = "block";
          document.getElementById("ssidap").value = ssid_ap;
          document.getElementById("passap").value = pass_ap;
          var cd_hdwifi = document.getElementsByName("cdhdwifi");
          for (var i = 0, length = cd_hdwifi.length; i < length; i++) {
              if (cd_hdwifi[i].value == cdhd_wifi) {
                  document.getElementsByName("cdhdwifi")[i].checked = "true";   
                  break;
              }
          }
        }
        //-----------Thiết lập dữ liệu và gửi request ssid và password---
        function writeEEPROMap(){
          if(Empty(document.getElementById("ssidap"), "Please enter ssid!")&&Empty(document.getElementById("passap"), "Please enter password")){
            var ssidap = document.getElementById("ssidap").value;
            var passap = document.getElementById("passap").value;
            var cdhdap = document.getElementsByName("cdhdwifi");
            for (var i = 0, length = cdhdap.length; i < length; i++) {
                if (cdhdap[i].checked) {
                    cdhdap=cdhdap[i].value;    
                    break;
                }
            }
            xhttp.open("GET","/writeEEPROMap?ssidap="+ssidap+"&passap="+passap+"&cdhdap="+cdhdap,true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          }
        }
        function clearEEPROM(){
          if(confirm("Do you want to delete all saved wifi configurations?")){
            xhttp.open("GET","/clearEEPROM",true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          }
        }
        function restartESP(){
          if(confirm("Do you want to reboot the device?")){
            xhttp.open("GET","/restartESP",true);
            xhttp.send();
            alert("Device is restarting! If no wifi is found please press reset!");
          }
        }
        //-----------CAU HINH SIM900A---------------------------------------------
        function configuresim900a(){
          document.getElementById("homecontrol").style.display = "none";
          document.getElementById("wifisetupap").style.display = "none";
          document.getElementById("sim900asetup").style.display = "block";
          document.getElementById("sodt_send").value = sodtsend;
          document.getElementById("sms_send").value = smssend;
          var cdhd_sim900a = document.getElementsByName("cdhdsim900a");
          for (var i = 0, length = cdhd_sim900a.length; i < length; i++) {
              if (cdhd_sim900a[i].value == cdhdsim) {
                  document.getElementsByName("cdhdsim900a")[i].checked = "true";   
                  break;
              }
          }
        }
        function writeEEPROMsim900a(){
          if(Empty(document.getElementById("sodt_send"), "Please enter mobile number!")&&Empty(document.getElementById("sms_send"), "Please enter sms content")){
            var sodtgui = document.getElementById("sodt_send").value;
            var smsgui = document.getElementById("sms_send").value;
            var cdhd_sim900a = document.getElementsByName("cdhdsim900a");
            for (var i = 0, length = cdhd_sim900a.length; i < length; i++) {
                if (cdhd_sim900a[i].checked) {
                    cdhd_sim900a=cdhd_sim900a[i].value;    
                    break;
                }
            }
            xhttp.open("GET","/writeEEPROMsim900a?sodtsend="+sodtgui+"&smssend="+smsgui+"&cdhdsim900a="+cdhd_sim900a,true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          }
        }
        //-----------Kiểm tra response -------------------------------------------
        function process(){
          if(xhttp.readyState == 4 && xhttp.status == 200){
            //------Updat data sử dụng javascript----------
            ketqua = xhttp.responseText; 
            alert(ketqua);       
          }
        }
       //============Hàm thực hiện chứ năng khác================================
       //--------Load lại trang để quay về Home control-------------------------
        function backHOME(){
          document.getElementById("homecontrol").style.display = "block";
          document.getElementById("wifisetupap").style.display = "none";
          document.getElementById("sim900asetup").style.display = "none";
        }
       //----------------------------CHECK EMPTY--------------------------------
       function Empty(element, AlertMessage){
          if(element.value.trim()== ""){
            alert(AlertMessage);
            element.focus();
            return false;
          }else{
            return true;
          }
       }
       //=====================WEBSOCKET CLIENT===================================
       var Socket;      //Khai báo biến Socket
       var d4, ssid_ap, pass_ap, cdhd_wifi, led,lockstate, sodtsend, smssend, cdhdsim ;
       function init(){
         //Khởi tạo websocket
         Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
         //Nhận broadcase từ server
         Socket.onmessage = function(event){   
           JSONobj = JSON.parse(event.data);   //Tách dữ liệu json
           d4 = JSONobj.D4;
           ssid_ap = JSONobj.SSIDap;
           pass_ap = JSONobj.PASSap;
           cdhd_wifi = JSONobj.CDHDap;
           lockstate = JSONobj.LOCKOUT;
           led = JSONobj.LED;
           sodtsend = JSONobj.SODTsend;
           smssend = JSONobj.SMSsend;
           cdhdsim = JSONobj.CDHDsim;

           //Kiểm tra trạng thái khóa và hiển thị lên webserver
           if(d4 == "1"){
              document.getElementById("button_lock").value = "LOCK"
              document.getElementById("iconlock").style.display = "none";
              document.getElementById("iconunlock").style.display = "block";
           }else{
              document.getElementById("button_lock").value = "UNLOCK"
              document.getElementById("iconlock").style.display = "block";
              document.getElementById("iconunlock").style.display = "none";
           }
           if(led == "0"){
              document.getElementById("ledconnect").style.background = "#222222";
              //document.getElementById("status_Door").style.color = "#FFFFFF";
           }else{
              document.getElementById("ledconnect").style.background = "#00EE00";
              //document.getElementById("status_Door").style.color = "#222222";
           }
         }
       }45
        function lockactive(){
         if(document.getElementById("button_lock").value == "UNLOCK"){
            document.getElementById("button_lock").value = "LOCK"
            Socket.send("unlock");
          }else{
            document.getElementById("button_lock").value = "UNLOCK"
            Socket.send("lock");
          }
        }
        //----------Gioi han ky tu nhap vao maxlength sms----------
        var smsinput = document.getElementById('sms_send');
        var length = smsinput.getAttribute("maxlength");
        var numberinput = document.getElementById('count');
        numberinput.innerHTML = length;
        smsinput.onkeyup = function () {
          document.getElementById('count').innerHTML = (length - this.value.length);
        };
        function test_sim900a() {
          var cdhd_sim900a = document.getElementsByName("cdhdsim900a");
          for (var i = 0, length = cdhd_sim900a.length; i < length; i++) {
              if (cdhd_sim900a[i].checked) {
                  cdhd_sim900a=cdhd_sim900a[i].value;    
                  break;
              }
          }
          if(cdhd_sim900a=="0"){
            Socket.send("testcall")
          }else if(cdhd_sim900a=="1"){
            Socket.send("testsms")
          }
        }
      </script>
   </body> 
  </html>
)=====";
void setup() {
  Serial.begin(9600);
  SIM900.begin(9600);
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);
  
  EEPROM.begin(512);       //Khởi tạo bộ nhớ EEPROM
  delay(10);

  pinMode(D4,OUTPUT);
  pinMode(D5,INPUT);

  read_EEPROM();
  WiFi.disconnect();
  delay(100);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip_ap, gateway_ap, subnet_ap);
  if(cdhd_ap == "1"){
    WiFi.softAP(ssid_ap,pass_ap,1,false);
  }else{
    WiFi.softAP(ssid_ap,pass_ap,1,true);
  }
  Serial.println("================Thiết lập chế độ hoạt động WiFi===============");
  Serial.println("Soft Access Point mode!");
  Serial.print("Please connect to ");
  Serial.println(ssid_ap);
  Serial.print("Password is: ");
  Serial.println(pass_ap);
  startWebServer();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.print("Web Server IP Address: ");
  Serial.println(ip_ap);
  timer_update.setInterval(500L, updateStateD);
  setupSIM900A();
  attachInterrupt(D5, Interrupt_Sensor, RISING);   //Ngắt khi có cạnh lên
}

void loop() {
  webServer.handleClient();
  webSocket.loop();
  timer_update.run();
  if(canhbaostate==1){
    timer_canhbao.run();
  }
  if(SIM900.available()){
    while(SIM900.available()){
      char inChar = (char)SIM900.read();
      bufferSIM900 += inChar;
      if(bufferSIM900.length()>=160){
        bufferSIM900="";
      }
    }
    //Serial.println(bufferSIM900);
    //Khi có cuộc gọi đến thi kiểm tra số gọi đến
    Index_Rxdata = bufferSIM900.indexOf("RING");
    if(Index_Rxdata >= 0){
      Index_Rxdata = -1;
      SIM900.println("ATH");
      if(sodt_send!=""){
        Index_Rxdata = bufferSIM900.indexOf(sodt_send);
        if(Index_Rxdata >= 0){
          Index_Rxdata = -1;
          bufferSIM900="";
          Serial.println("Đã nhận được cuộc gọi từ: " + sodt_send +". Đã tắt cảnh báo!");
          digitalWrite(D4,HIGH);
          lockstate=0;
          canhbaostate=0;
        }
      }
    }
    //Khi có tin nhắn đến thì kiểm tra tin nhắn
    Index_Rxdata = bufferSIM900.indexOf("OFF CB");
    if(Index_Rxdata >= 0){
      Index_Rxdata = -1;
      bufferSIM900="";
      Serial.println("Đã nhận tin nhắn tắt cảnh báo!");
      lockstate=0;
      digitalWrite(D4,HIGH);
      canhbaostate=0;
      EEPROM.write(268, lockstate);
      EEPROM.commit();
      Serial.println("Đã unlock!");
    }
    Index_Rxdata = bufferSIM900.indexOf("ON CB0");
    if(Index_Rxdata >= 0){
      Index_Rxdata = -1;
      bufferSIM900="";
      Serial.println("Đã nhận tin nhắn bật cảnh báo!");
      lockstate=1;
      digitalWrite(D4,LOW);
      canhbaostate=0;
      cdhdsim900="0";
      EEPROM.write(267, cdhdsim900[0]);
      EEPROM.write(268, lockstate);
      EEPROM.commit();
      Serial.println("Đã kích hoạt báo động chế độ Call mobile!");
      
    }
    Index_Rxdata = bufferSIM900.indexOf("ON CB1");
    if(Index_Rxdata >= 0){
      Index_Rxdata = -1;
      bufferSIM900="";
      Serial.println("Đã nhận tin nhắn bật cảnh báo!");
      lockstate=1;
      digitalWrite(D4,LOW);
      canhbaostate=0;
      cdhdsim900="1";
      EEPROM.write(267, cdhdsim900[0]);
      EEPROM.write(268, lockstate);
      EEPROM.commit();
      Serial.println("Đã kích hoạt báo động chế độ Send SMS!");
      
    }
  }
}
//==========CHƯƠNG TRÌNH CON===================================//
//------------Đọc bộ nhớ EEPROM--------------------
void read_EEPROM(){
  Serial.println("Reading EEPROM...");
  Serial.println("==================Thông tin cấu hình Wifi=====================");
  if(EEPROM.read(0)!=0){
    ssid_ap = "";
    pass_ap = "";
    cdhd_ap = "";
    for (int i=0; i<32; ++i){
      ssid_ap += char(EEPROM.read(i));
    }
    Serial.print("SSID AP: ");
    Serial.println(ssid_ap);
    for (int i=32; i<96; ++i){
      pass_ap += char(EEPROM.read(i));
    }
    Serial.print("PASSWORD: ");
    Serial.println(pass_ap);
    cdhd_ap = char(EEPROM.read(96));
    Serial.print("Chế độ Wifi: ");
    if(cdhd_ap=="0"){
      Serial.println("Ẩn");
    }else if(cdhd_ap=="1"){
      Serial.println("Hiện");
    }
    ssid_ap = ssid_ap.c_str();
    pass_ap = pass_ap.c_str();
  }else{
    Serial.println("Data wifi AP mode not found!");
  }
  Serial.println("==============Thông tin cấu hình Module Sim900A===============");
  if(EEPROM.read(98)!=0){
    sodt_send="";
    sms_send="";
    cdhdsim900="";
    for (int i=97; i<107; ++i){
      sodt_send += char(EEPROM.read(i));
    }
    Serial.print("Số điện thoại: ");
    Serial.println(sodt_send);
    for (int i=107; i<267; ++i){
      sms_send += char(EEPROM.read(i));
    }
    Serial.print("Nội dung SMS: ");
    Serial.println(sms_send);
    cdhdsim900 = char(EEPROM.read(267));
    Serial.print("Chế độ cảnh báo: ");
    if(cdhdsim900=="0"){
      Serial.println("Call Mobile");
    }else if(cdhdsim900=="1"){
      Serial.println("Send SMS");
    }
    sodt_send = sodt_send.c_str();
    sms_send = sms_send.c_str();
  }
  lockstate=boolean(EEPROM.read(268)); 
  Serial.print("Trạng thái cảnh báo: ");
  if(lockstate==0){
    digitalWrite(D4,HIGH);
    Serial.println("Chưa kích hoạt");
  }else if(lockstate==1){
    digitalWrite(D4,LOW);
    Serial.println("Đã kích hoạt");
  }
}
//----------------WEB SERVER-----------------------
void startWebServer(){
  webServer.on("/",[]{
    String s = MainPage;
    webServer.send(200,"text/html",s);
  });
  webServer.on("/writeEEPROMap",[]{
    ssid_ap = webServer.arg("ssidap");
    pass_ap = webServer.arg("passap");
    cdhd_ap = webServer.arg("cdhdap");
    Serial.println("Clear EEPROM!");
    for (int i = 0; i < 97; ++i) {
      EEPROM.write(i, 0);           
      delay(10);
    }
    for (int i = 0; i < ssid_ap.length(); ++i) {
      EEPROM.write(i, ssid_ap[i]);
    }
    for (int i = 0; i < pass_ap.length(); ++i) {
      EEPROM.write(32 + i, pass_ap[i]);
    }
    EEPROM.write(96, cdhd_ap[0]);
    EEPROM.commit();
    Serial.println("Writed to EEPROM!");
    Serial.print("SSID AP: ");
    Serial.println(ssid_ap);
    Serial.print("PASS: ");
    Serial.println(pass_ap);
    Serial.print("Chế độ Wifi: ");
    if(cdhd_ap=="0"){
      Serial.println("Ẩn");
    }else if(cdhd_ap=="1"){
      Serial.println("Hiện");
    }
    String s = "Wifi configuration saved!";
    webServer.send(200, "text/html", s);
  });
  webServer.on("/writeEEPROMsim900a",[]{
    sodt_send = webServer.arg("sodtsend");
    sms_send = webServer.arg("smssend");
    cdhdsim900 = webServer.arg("cdhdsim900a");
    Serial.println("Clear EEPROM!");
    for (int i = 97; i < 268; ++i) {
      EEPROM.write(i, 0);           
      delay(10);
    }
    for (int i = 0; i < sodt_send.length(); ++i) {
      EEPROM.write(97 + i, sodt_send[i]);
    }
    for (int i = 0; i < sms_send.length(); ++i) {
      EEPROM.write(107 + i, sms_send[i]);
    }
    EEPROM.write(267, cdhdsim900[0]);
    EEPROM.commit();
    Serial.println("Writed to EEPROM!");
    Serial.print("Số điện thoại: ");
    Serial.println(sodt_send);
    Serial.print("SMS: ");
    Serial.println(sms_send);
    Serial.print("Chế độ cảnh báo: ");
    if(cdhdsim900=="0"){
      Serial.println("Call Mobile");
    }else if(cdhdsim900=="1"){
      Serial.println("Send SMS");
    }
    String s = "Sim900A configuration saved!";
    webServer.send(200, "text/html", s);
  });
  webServer.on("/restartESP",[]{
    ESP.restart();
  });
  webServer.on("/clearEEPROM",[]{
    Serial.println("Clear EEPROM!");
    for (int i = 0; i < 512; ++i) {
      EEPROM.write(i, 0);           
      delay(10);
    }
    EEPROM.commit();
    String s = "Device has been reset!";
    webServer.send(200,"text/html", s);
  });
  webServer.begin();
  Serial.println("Web Server is started!");
}
//---------------------WEBSOCKET---------------------------------/
void webSocketEvent(uint8_t num, WStype_t type,uint8_t * payload,size_t length){
  String payloadString = (const char *)payload;
  Serial.print("payloadString= ");
  Serial.println(payloadString);
  if(payloadString == "unlock"){
    digitalWrite(D4,HIGH);   
    lockstate=0;
    EEPROM.write(268, lockstate);
    EEPROM.commit();
    Serial.println("Đã tắt báo động!");
    canhbaostate=0; 
  }else if(payloadString =="lock"){
    digitalWrite(D4,LOW);
    lockstate=1;
    EEPROM.write(268, lockstate);
    EEPROM.commit();
    Serial.println("Đã kích hoạt báo động!");
  }
  if(payloadString == "testcall"){
    SIM900Call(sodt_send);
  }
  if(payloadString == "testsms"){
    SIM900SMS(sodt_send, sms_send);
  }
}
void updateStateD(){
  String statusD4 = "";
  ledconnect = !ledconnect;
  if(digitalRead(D4)==0){
    statusD4 = "0";
  }else{
    statusD4 = "1";
  }
  String ipconnect = WiFi.localIP().toString();
  String JSONtxt = "{\"D4\": \""+ statusD4 +"\"," +
                    "\"SSIDap\": \""+ ssid_ap + "\"," +
                    "\"PASSap\": \""+ pass_ap + "\"," +
                    "\"CDHDap\": \""+ cdhd_ap + "\"," +
                    "\"LED\": \""+ ledconnect + "\"," +
                    "\"SODTsend\": \""+ sodt_send + "\"," +
                    "\"SMSsend\": \""+ sms_send + "\"," +
                    "\"CDHDsim\": \""+ cdhdsim900 +"\"}";
  webSocket.broadcastTXT(JSONtxt);
  //Serial.println(JSONtxt);
}
void setupSIM900A(){
  SIM900.println("ATE0"); //Tắt chế độ echo khi gửi lệnh đi
  delay(1000);
  SIM900.println("AT+IPR=9600"); //Cài tốc độ baud 9600
  delay(1000);
  SIM900.println("AT+CMGF=1"); //Hiển thị tin nhắn ở chế độ txt
  delay(1000);
  SIM900.println("AT+CLIP=1"); //Hiển thị số điện thoại gọi đến
  delay(1000);
  SIM900.println("AT+CNMI=2,2"); //Hiển thị trực tiếp nội dung tin nhắn gửi đến
  delay(1000);
}
void SIM900Call(String sdt){
  SIM900.println("ATD" + sdt + ";");
  delay(1000);
  Serial.print("Đang thực hiện cuộc gọi đến số: ");
  Serial.println(sdt);
}
void SIM900SMS(String phone, String content){
  SIM900.println("AT+CMGS=\"" + phone + "\"");     // Lenh gui tin nhan
  delay(3000);                                     // Cho ky tu '>' phan hoi ve 
  SIM900.print(content);                           // Gui noi dung
  SIM900.print((char)26);                          // Gui Ctrl+Z hay 26 de ket thuc noi dung tin nhan va gui tin di
  delay(5000);                                     // delay 5s
  Serial.print("Đã gủi tin nhắn đến số: ");
  Serial.println(phone);
  Serial.print("Nội dung tin nhắn: ");
  Serial.println(content);
}
void Interrupt_Sensor(){
  if(lockstate==1){
    if (digitalRead(D5) == HIGH) {
      if (sensorState != HIGH) {
        Serial.println("Phát hiện có đột nhập! Đã kích hoạt báo động!");
        if(canhbaostate==0){
          if(cdhdsim900=="0"){
            SIM900Call(sodt_send);
            timer_canhbao.setInterval(120000L, canhbaoControl); //2 phut canh bao 1 lan
          }else if(cdhdsim900=="1"){
            SIM900SMS(sodt_send, sms_send);
          }
          canhbaostate=1;
        }
      }
      sensorState = HIGH;
    } else {
      sensorState = LOW;
    }
  }else{
    //Serial.println("Phát hiện có đột nhập. Chế độ báo động chưa được kích hoạt!");
    canhbaostate=0;
  }
}
void canhbaoControl(){
  if(cdhdsim900=="0"){
    SIM900Call(sodt_send);
  }
}
