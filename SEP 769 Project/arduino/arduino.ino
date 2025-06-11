// 引脚定义
#define PIN_Motor_PWMA 5    // 右电机速度
#define PIN_Motor_PWMB 6    // 左电机速度
#define PIN_Motor_AIN_1 7   // 右电机方向
#define PIN_Motor_BIN_1 8   // 左电机方向
#define PIN_Motor_STBY 3

// 运行参数
#define SPEED 50    // 正常速度（0-255）

// 电机控制函数
void setMotors(int STEER) {
  // 右电机控制
  digitalWrite(PIN_Motor_AIN_1, HIGH);
  analogWrite(PIN_Motor_PWMA, abs(SPEED - STEER));
  
  // 左电机控制
  digitalWrite(PIN_Motor_BIN_1, HIGH);
  analogWrite(PIN_Motor_PWMB, abs(SPEED + STEER));
}

void setup() {
  // 初始化电机控制引脚
  pinMode(PIN_Motor_PWMA, OUTPUT);
  pinMode(PIN_Motor_AIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMB, OUTPUT);
  pinMode(PIN_Motor_BIN_1, OUTPUT);
  pinMode(PIN_Motor_STBY, OUTPUT);
  Serial.begin(115200); // 与ESP32-CAM的串口波特率一致
}

unsigned long lastDataTime = 0;  // 记录最后收到数据的时间戳

void loop() {
  if (Serial.available()) {
    // 收到数据时记录最新时间戳
    lastDataTime = millis();
    
    digitalWrite(PIN_Motor_STBY, HIGH);
    String input = Serial.readStringUntil('\n');
    // 转换为整数
    int result = input.toInt();
    Serial.print("收到预测结果: ");
    Serial.println(result);
    setMotors(result);
  }
  else {
    // 检查是否超过1秒无数据
    if (millis() - lastDataTime > 1000) {
      digitalWrite(PIN_Motor_STBY, LOW);
    }
  }
}
