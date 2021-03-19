#include <dummy.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "Adafruit_MPU6050_Counterfeit.h"

const String device_id = "00000001_IMD";
// #define DEBUG_ACCEL

Adafruit_MPU6050 mpu;
const float x_1G_max = 10.1;
const float x_1G_min = -9.6;
const float y_1G_max = 10.0;
const float y_1G_min = -9.8;
const float z_1G_max = 11.1;
const float z_1G_min = -8.8;

const char* ssid = "huit-hackathon-2021";
const char* password = "ayuminmogitakapiro";

// const char* serverURL = "https://a-2016-backend.herokuapp.com/devices/weight";
const char* serverURL = "http://128.199.0.5:3000/post";

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT); 
    digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  Serial.println("Boot!");
  // pinMode(sda, INPUT_PULLUP);
  // pinMode(scl, INPUT_PULLUP);
  Wire.begin(21,22,400000);

  // Try to initialize MPU6050
  if (!mpu.begin()) {
  // if (!mpu.begin(MPU6050_I2CADDR_DEFAULT,&Wire,0)) {
    Serial.println("Failed to find MPU6050");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
 
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }
 
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }
 
  Serial.println("");
  delay(100);

  setup_wifi();
}

void loop() {
  // put your main code here, to run repeatedly:
  byte error = 0;
  static unsigned int huit_points = 0;
  static unsigned int stable_count = 0;

  // if (WiFi.status() == WL_CONNECTED) {
  //   Serial.print("Wifi OK");
  // }
  
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
 
  // /* Print out the values */
  int accel_squared = square(calb(a.acceleration.x,x_1G_max,x_1G_min))
                        +square(calb(a.acceleration.y,y_1G_max,y_1G_min))
                        +square(calb(a.acceleration.z,z_1G_max,z_1G_min));
#ifdef DEBUG_ACCEL
  Serial.printf("%5d ",accel_squared);
#endif
  accel_squared = accel_squared - 100;
#ifdef DEBUG_ACCEL
  Serial.printf("%5d ",accel_squared);
#endif
  if(accel_squared < 0)accel_squared =-accel_squared;
  if(accel_squared < 10){
    // accel_squared = 0;
    if(stable_count<=30){
      stable_count++;
    }
    // Serial.printf("%7d\n",huit_points);
  }else{
    stable_count=0;
    // Serial.printf("%7d%+5d\n",huit_points,accel_squared);
    huit_points += accel_squared;
    Serial.printf("%7d\n",huit_points);
  }
#ifdef DEBUG_ACCEL
  Serial.printf("X: ",);
  Serial.print(calb(a.acceleration.x,x_1G_max,x_1G_min));
  Serial.print(", Y: ");
  Serial.print(calb(a.acceleration.y,y_1G_max,y_1G_min));
  Serial.print(", Z: ");
  Serial.print(calb(a.acceleration.z,z_1G_max,z_1G_min));
#endif
  if(stable_count==30 && huit_points){
  // if(stable_count==30 || huit_points > 10000){
    if(!send_data(huit_points)){
      // succeed
      huit_points = 0;
    }else{
      stable_count = 0;
    }
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
  }else{
    Serial.flush();
    delay(100);
  }
}

unsigned int square(float value){
  return value * value;
}

// 1G : 10
float calb(float value,float max,float min)
{
  return (value-(max+min)/2)/(max-min)*20;
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

int send_data(int huit_points) {
  digitalWrite(LED_BUILTIN, HIGH);
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");
    String httpRequestData = "{\"device_id\": \"" + device_id + "\",\"huit_points\":" + String(huit_points) + "}";
    int httpResponseCode = http.POST(httpRequestData);
    String res = http.getString();
    Serial.println("req body: " + httpRequestData);
    Serial.println("response: " + res);
    http.end();
    Serial.print("status code: ");
    Serial.println(httpResponseCode);
    digitalWrite(LED_BUILTIN, LOW);
    if(httpResponseCode==201){
      return 0;
    }
    return 1;
  } else {
    Serial.println("NO WIFI NETWORK");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(300);
    digitalWrite(LED_BUILTIN, LOW);
    delay(300);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(300);
  digitalWrite(LED_BUILTIN, LOW);
  }
  return 1;
}
