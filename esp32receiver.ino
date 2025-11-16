#include <WiFi.h>
#include <esp_now.h>
#include <ESP32Servo.h>

// ---------- CẤU HÌNH CẢM BIẾN SÓNG ÂM ----------
#define NUM_SENSORS 5
const int trigPins[NUM_SENSORS] = {13, 11, 9, 8, 6}; // Đã bỏ chân 4
const int echoPins[NUM_SENSORS] = {14, 12, 10, 7, 5}; // Đã bỏ chân 3
int distances[NUM_SENSORS];
// Chân động cơ và servo (cập nhật cho ESP32-S3)
#define RPWM 36   // PWM phải, ví dụ chân GPIO4
#define LPWM 35  // PWM trái, ví dụ chân GPIO5
#define SERVO_PIN 37 // Chân servo, ví dụ chân GPIO6
#define coi 48
#define SERVO_CENTER 90
#define SERVO_LEFT 60
#define SERVO_RIGHT 120
Servo steering;
uint8_t gocquay;

// Cấu trúc dữ liệu (phải khớp với module truyền)
struct struct_message {
    uint8_t speed;      // 0-255
    uint8_t servo;      // 0-180
    uint8_t trimspeed;  // 50-255
    uint8_t trimservo;  // 0-180
    bool ketnoi : 1;
    bool chedo : 1;
} controlData;

// Callback nhận dữ liệu
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
    memcpy(&controlData, incomingData, sizeof(controlData));
    // lastReceiveTime = millis(); // Cập nhật thời gian nhận
    // // Debug
    // Serial.print("Nhận: Tốc độ=");
    // Serial.print(controlData.speed);
    // Serial.print(", Servo=");
    // Serial.print(controlData.servo);
    // Serial.print(", trimServo=");
    // Serial.print(controlData.trimservo);
    // Serial.print(", trimSpeed=");
    // Serial.println(controlData.trimspeed);

    // Điều khiển động cơ
  if (controlData.chedo == false ){
    Serial.println ("AUTO");
      readAllUltrasonics();
      navigate();
    delay(100);
  }
  else {
  // Debug
    Serial.print("Nhận: Tốc độ=");
    Serial.print(controlData.speed);
    Serial.print(", Servo=");
    Serial.print(controlData.servo);
    Serial.print(", trimServo=");
    Serial.print(controlData.trimservo);
    Serial.print(", trimSpeed=");
    Serial.println(controlData.trimspeed);
    if (controlData.ketnoi == false ){
      analogWrite(RPWM, 0);
      analogWrite(LPWM, 0);
      analogWrite(coi, 50);
      digitalWrite ( 48 , HIGH);
    }
    else if (controlData.trimspeed <= 130 && controlData.speed > 70 && controlData.speed < 120) {
        uint8_t tocdo = map(controlData.speed, 60, 120, 0, controlData.trimspeed - 40);
        analogWrite(RPWM, tocdo);
        analogWrite(LPWM, 0);
        analogWrite(coi, 0);
    } else if (controlData.trimspeed <= 130 && controlData.speed >= 120) {
        uint8_t tocdo = map(controlData.speed, 10, 255, controlData.trimspeed - 40, controlData.trimspeed);
        analogWrite(RPWM, tocdo);
        analogWrite(LPWM, 0);
        analogWrite(coi, 0);
    } else if (controlData.trimspeed > 130 && controlData.speed > 70 && controlData.speed < 120) {
        uint8_t tocdo = map(controlData.speed, 90, 120, 0, 130);
        analogWrite(RPWM, tocdo);
        analogWrite(LPWM, 0);
        analogWrite(coi, 0);
    } else if (controlData.trimspeed > 130 && controlData.speed >= 120) {
        uint8_t tocdo = map(controlData.speed, 150, 255, 120, controlData.trimspeed);
        analogWrite(RPWM, tocdo);
        analogWrite(LPWM, 0);
        analogWrite(coi, 0);
    } else if (controlData.speed < 35) {
        uint8_t tocdo = map(controlData.speed, 45, 0, 0, 130); // Tối ưu ánh xạ
        analogWrite(RPWM, 0);
        analogWrite(LPWM, tocdo);
        analogWrite(coi, 0);
    } else {
        analogWrite(RPWM, 0);
        analogWrite(LPWM, 0);
        analogWrite(coi, 0);
    }
  
    // Điều khiển servo
    if (controlData.servo > 72) {
        gocquay = map(controlData.servo, 72, 125, controlData.trimservo, controlData.trimservo + 60);
        steering.write(gocquay);
    } else if (controlData.servo < 63) {
        gocquay = map(controlData.servo, 63, 10, controlData.trimservo, controlData.trimservo - 60);
        steering.write(gocquay);
    } else {
        steering.write(controlData.trimservo);
    }
  }
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.channel(1); // Cố định kênh Wi-Fi (phải khớp với module truyền)
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // Giảm công suất truyền để giảm nhiễu

    // In địa chỉ MAC
    Serial.println("MAC ESP32-S3: " + WiFi.macAddress());

    // Khởi tạo chân
    
    steering.attach(SERVO_PIN); // Cấu hình servo với xung min/max (tùy servo)

    
    

    // Khởi tạo ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Lỗi khởi tạo ESP-NOW");
        return;
    }

    // Đăng ký callback nhận dữ liệu
    esp_now_register_recv_cb(OnDataRecv);

    Serial.println("Bộ nhận ESP32-S3 sẵn sàng");

 Serial.println("=== Khởi động hệ thống ===");

  for (int i = 0; i < NUM_SENSORS; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }
  centerSteering();

  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);

  stopMotor();
}
void loop() {
    // Không cần code trong loop vì ESP-NOW xử lý qua callback
}
// ---------- Đọc khoảng cách ----------
int getDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  unsigned long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 999;
  return int(duration / 2 / 29.412);
}

void readAllUltrasonics() {
  for (int i = 0; i < NUM_SENSORS; i++) {
    distances[i] = getDistance(trigPins[i], echoPins[i]);
    Serial.print("Cảm biến ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(distances[i]);
    Serial.println(" cm");
    delay(50);
  }
  Serial.println("---");
}

// ---------- Servo ----------
void turnLeft() {
  Serial.println("👉 Lái trái");
  steering.write(SERVO_LEFT);
}
void turnRight() {
  Serial.println("👉 Lái phải");
  steering.write(SERVO_RIGHT);
}
void centerSteering() {
  Serial.println("🔄 Lái giữa");
  steering.write(SERVO_CENTER);
}

// ---------- Motor ----------
void driveForward(int speed) {
  Serial.print("🚗 Tiến tới với tốc độ: ");
  Serial.println(speed);
  analogWrite(RPWM, speed);
  analogWrite(LPWM, 0);
}

void driveBackward(int speed) {
  Serial.print("🔙 Lùi lại với tốc độ: ");
  Serial.println(speed);
  analogWrite(RPWM, 0);
  analogWrite(LPWM, speed);
}

void stopMotor() {
  Serial.println("⛔ Dừng động cơ");
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
}

// ---------- Xử lý tự hành ----------
// ---------- Xử lý tự hành ----------
void navigate() {
  int frontLeft   = distances[0];  // Cảm biến 1
  int front       = distances[1];  // Cảm biến 2
  int frontRight  = distances[2];  // Cảm biến 3
  int sideLeft    = distances[3];  // Cảm biến 4
  int sideRight   = distances[4];  // Cảm biến 5

  int speed = 30;

  Serial.println("🤖 Đang xử lý điều hướng...");

  // ===== Trường hợp tất cả phía trước đều bị cản (1,2,3) => Lùi =====
  if (frontLeft < 5 && front < 5 && frontRight < 5) {
    Serial.println("🛑 Cản toàn bộ phía trước ➡ Lùi");
    centerSteering();
    driveBackward(speed);
    delay(4000);
    stopMotor();

    readAllUltrasonics();

    frontLeft = distances[0];
    front = distances[1];
    frontRight = distances[2];
    sideLeft = distances[3];
    sideRight = distances[4];

    int leftScore = frontLeft + sideLeft;
    int rightScore = frontRight + sideRight;
    Serial.print("👉 Sau lùi - Score Trái: "); Serial.print(leftScore);
    Serial.print(" | Phải: "); Serial.println(rightScore);

    if (leftScore > rightScore) {
      turnLeft();
    } else {
      turnRight();
    }
    return;
  }

  // ===== TH1: Cảm biến 1 & 2 hoặc 1,2,4 có vật cản => Lái phải =====
  if ((frontLeft < 5 && front < 5) || (frontLeft < 5 && front < 5 && sideLeft < 5)) {
    Serial.println("📌 ⚠️ Vật cản phía trước trái hoặc trái + bên => Đánh lái phải");
    turnRight();
    driveForward(speed);
    return;
  }

  // ===== TH2: Cảm biến 2 & 3 hoặc 2,3,5 có vật cản => Lái trái =====
  if ((front < 5 && frontRight < 5) || (front < 5 && frontRight < 5 && sideRight < 5)) {
    Serial.println("📌 ⚠️ Vật cản phía trước phải hoặc phải + bên => Đánh lái trái");
    turnLeft();
    driveForward(speed);
    return;
  }

  // ===== Không có vật cản trước =====
  if (front > 5 && frontLeft > 5 && frontRight > 5) {
    Serial.println("✅ Không có vật cản phía trước");
    centerSteering();
    driveForward(speed);
    return;
  }

  // ===== Cản riêng phía trước trái =====
  if (frontLeft < 5 && front > 5 && frontRight > 5) {
    Serial.println("⚠️ Vật cản phía trước trái (chỉ 1) => Đánh phải");
    turnRight();
    driveForward(speed);
    return;
  }

  // ===== Cản riêng phía trước phải =====
  if (frontRight < 5 && front > 5 && frontLeft > 5) {
    Serial.println("⚠️ Vật cản phía trước phải (chỉ 1) => Đánh trái");
    turnLeft();
    driveForward(speed);
    return;
  }

  // ===== Cản ngay giữa, chọn bên có điểm tốt hơn =====
  if (front < 5 && frontLeft > 5 && frontRight > 5) {
    Serial.println("⚠️ Vật cản ngay giữa, đang chọn bên để tránh");
    int leftScore = frontLeft + sideLeft;
    int rightScore = frontRight + sideRight;
    Serial.print("👉 Score Trái: "); Serial.print(leftScore);
    Serial.print(" | Phải: "); Serial.println(rightScore);

    if (leftScore > rightScore) {
      turnLeft();
    } else {
      turnRight();
    }
    driveForward(speed);
    return;
  }

  // ===== Trạng thái không xác định =====
  Serial.println("⚠️ Trạng thái không xác định, dừng lại");
  stopMotor();
}