#include <Adafruit_HX711.h>
#include <GyverPortal.h>
#include <WiFi.h>  
#include <Wire.h>
#include <Adafruit_BME280.h> //++ Актуальний сенсор BMP280

#define AP_SSID "LEV"
#define AP_PASS "L72V73E08"
#define BME_SCK 13
#define BME_MISO 12  //++ Виводи 0,2,5,12,15 (strapping) потрібно використовувати з обережністю,
#define BME_MOSI 11  //++  краще взагалі уникати їх використання.
#define BME_CS 10
#define DATA_PIN 2
#define CLOCK_PIN 3

GyverPortal ui;
Adafruit_BME280 bme;
Adafruit_HX711 hx711(DATA_PIN, CLOCK_PIN);

uint8_t valSlider;
uint8_t button1;
uint8_t button2;
uint16_t valSpinner;
uint16_t weight;
uint16_t weight_sr;
float speed;
uint16_t pressure;
float pres_sr;        
unsigned long startTime;
bool ramping = 0;
bool ram = 0;
bool tare = 0;
const char *pl1 [] = {"Швидкість потоку"};
const char *pl2 [] = {"Маса"};

void build() {
  GP.BUILD_BEGIN(1000);
  GP.UPDATE("weight, speed");
  GP.TITLE("Аеротруба", "t1", "#7CFC00", 35);
  GP.HR();
  GP.THEME(GP_DARK);
  GP.BUTTON("btn1", "Калібрувати"); GP.BREAK();
  GP.LABEL("Потужність: ");
  GP.SLIDER("sld1", valSlider, 0, 100); GP.BREAK();
  GP.LABEL("Час: ");
  GP.SPINNER("spn1", valSpinner, 0, 300, 5); 
  GP.LABEL("c"); GP.BREAK();
  GP.BUTTON("btn2", "Почати дослід"); GP.BREAK();
  GP.LABEL("Маса: ");
  GP.LABEL("weight", "weight", "#7CFC00", 28);     
  GP.LABEL("г"); GP.BREAK();
  GP.LABEL("Швидкість: ");
  GP.LABEL("speed", "speed", "#7CFC00", 28);     
  GP.LABEL("м/с"); GP.BREAK();
  GP.AJAX_PLOT_DARK("plot1", pl1, 1, 25, 1000);
  GP.AJAX_PLOT_DARK("plot2", pl2, 1, 25, 1000);
  GP.BUILD_END();
}

void setup() {
  Serial.begin(115200);
  hx711.begin();
  bme.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP_SSID, AP_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
  for (uint8_t i = 0; i < 3; i++) {
    hx711.tareA(hx711.readChannelRaw(CHAN_A_GAIN_128));
    hx711.tareA(hx711.readChannelRaw(CHAN_A_GAIN_128));   // тарування
  }
  ui.attachBuild(build);
  ui.attach(action);
  ui.start();
  startTime = millis();
}

void action() {
   if (ui.update()) {
    ui.updateInt("weight", weight_sr);
    ui.updateInt("speed", speed);
  }
  if (ui.update("plot1")) ui.answer(random(200)); // рандом для тесту
  if (ui.update("plot2")) ui.answer(random(150)); // рандом для тесту
  if (ui.clickInt("sld1", valSlider)) {
      Serial.print("Slider: ");
      Serial.println(valSlider);
  }
  if (ui.clickInt("spn1", valSpinner)) {
      Serial.print("Spinner: ");
      Serial.println(valSpinner);
  }
  if (ui.click("btn1")) {
    Serial.println("Button 1 click");
    tare = 1;
  }
  if (ui.click("btn2")) {
    Serial.println("Button 2 click");
    ramping = 1;
    ram = 1;
  }
}

void loop() {
  ui.tick();  //++ Якщо це не блокує виконання, то код нижче виконується з довільною
              //++  частотою, що не є добре. Використати позначку часу, як в прикладі.
  if (ramping) {
    if (ram) {
      startTime = millis();
      ram = 0;
    }
    unsigned long elapsed = millis() - startTime;
    if (elapsed >= valSpinner * 1000UL) {
      ramping = 0;
      return;  //++ Так робити не треба
    }  //++ На контролері слід уникати операцій ділення і взяття кореня, краще їх перенести на бік клієнта
    float progress = float(elapsed) / (valSpinner * 1000.0); // тут ми плавно збільшуємо потужність
    float currentPower = progress * valSlider;
    Serial.println(currentPower);
  }

  if (tare) {
    for (uint8_t i = 0; i < 3; i++) {
      hx711.tareA(hx711.readChannelRaw(CHAN_A_GAIN_128));   // тарування
    }
    tare = 0;
  }

  //++ Читання HX711 є блокуючим. Оновлення даних відбувається 10 разів на секунду.
  //++ Тому доцільно перед читанням перевірити сенсор на готовність: hx711.isBusy()

  weight = 0;
  for(uint8_t i = 0; i < 5; i++) {
    weight += hx711.readChannelBlocking(CHAN_A_GAIN_128); // усереднення по 5 значенням
  }
  weight_sr = pressure/10; //++ weight_sr = weight/5;

  //++ Уникати операцій ділення. Використовувати операції зсуву.
  //++ Наприклад, зробити 8 вимірів і усереднити: weight_sr = weight>>3;

  pressure = 0;
  for(uint8_t i = 0; i < 10; i++) {
    pressure += bme.readPressure(); // усереднення по 10 значенням
  }
  pres_sr = 1,66*(pressure/10);
  speed=sqrt(pres_sr); // швидкість потоку повітря за формуллою для трубки Піто. 1,66 - приблизне значення 2/густину повітря при норм. умовах
  //++ Операцію взяття кореня перенести на бік клієнта.
}
