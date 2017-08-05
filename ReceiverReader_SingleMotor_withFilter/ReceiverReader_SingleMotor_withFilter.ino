//======== Includes ========


//======== Configs ========
#define CH0_THRESHOLD 40
#define CH0_In_Duty_MAX 2100
#define CH0_In_Duty_MIN 800


//======== CONSTs ========
#define LED 13
#define RECEIVER_IN_CH0 2
#define DRIVER_OUT_CH0 5
#define DIR_CH0_L 8
#define DIR_CH0_H 9
#define CH0_In_Duty_MID (CH0_In_Duty_MAX + CH0_In_Duty_MIN) / 2
#define CW 1
#define STOP 0
#define CCW -1

void GPIO_Init()
{
  pinMode(RECEIVER_IN_CH0, INPUT);
  pinMode(DRIVER_OUT_CH0, OUTPUT);
  pinMode(DIR_CH0_L, OUTPUT);
  pinMode(DIR_CH0_H, OUTPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
}

volatile unsigned long CurrentMicros[1] = {
  0
};
volatile unsigned int In_Duty[1] = {
  0
};
unsigned char Out_Duty[1] = {
  0
};


unsigned int duty_vote[3] = {0,0,0};
char direction_vote[5] = {STOP,STOP,STOP,STOP,STOP};

unsigned int Lowpass(int DataIn)
{
	static char i = 0;
	char count;
	int sum = 0;
	duty_vote[i++] = DataIn;
	if(i>=3)
		i=0;
	for(count =0;count < 3;count++)
		sum+=duty_vote[count];
	return (int)(sum/3);
}
	

void SetDirection(char channel, char pre_mode)
{
	static char i = 0;
	char result;
	direction_vote[i++]=pre_mode;
	if(i == 5) 
		i = 0;
	
	for(int j = 0;j<=4;j++)
		result+=direction_vote[j];
	
	if (result>0)
    {
      digitalWrite(DIR_CH0_L, HIGH);
      digitalWrite(DIR_CH0_H, LOW);
    }
    else if (result==0)
    {
      digitalWrite(DIR_CH0_L, LOW);
      digitalWrite(DIR_CH0_H, LOW);
    }
    else if (result<0)
    {
     digitalWrite(DIR_CH0_L, LOW);
     digitalWrite(DIR_CH0_H, HIGH);
    }
}

void SetDriverPWM(char channel, unsigned char duty)
{
  switch (channel)
  {
    case 0:
      {
        analogWrite(DRIVER_OUT_CH0, duty);
        break;
      }
  }
}

void MotorOutput()
{
  if (In_Duty[0] > CH0_In_Duty_MAX || In_Duty[0] < CH0_In_Duty_MIN)
	 return;
  
  if (In_Duty[0] >= CH0_In_Duty_MID - CH0_THRESHOLD && In_Duty <= CH0_In_Duty_MID + CH0_THRESHOLD)
  {
    Out_Duty[0] = 0;
    SetDirection(0, STOP);
    SetDriverPWM(0, Lowpass(Out_Duty[0]));
  }
  if (In_Duty[0] > CH0_In_Duty_MID + CH0_THRESHOLD)
  {
    Out_Duty[0] = map(In_Duty[0], CH0_In_Duty_MID + CH0_THRESHOLD, CH0_In_Duty_MAX, 0, 255);
    SetDirection(0, CW);
    SetDriverPWM(0, Lowpass(Out_Duty[0]));
  }
  if (In_Duty[0] < CH0_In_Duty_MID - CH0_THRESHOLD)
  {
    Out_Duty[0] = 255 - map(In_Duty[0], CH0_In_Duty_MIN, CH0_In_Duty_MID - CH0_THRESHOLD, 0, 255);
    SetDirection(0, CCW);
    SetDriverPWM(0, Lowpass(Out_Duty[0]));
  }
}

void setup()
{
  GPIO_Init();
  Serial.begin(9600);
  attachInterrupt(0, GetDutyOnCh0, CHANGE);
}

void loop()
{
  MotorOutput();

    Serial.print(In_Duty[0]);
    Serial.print(" -> ");
    Serial.print(Out_Duty[0]);
    Serial.println();

  delay(50);
}

void GetDutyOnCh0()
{
  if (digitalRead(RECEIVER_IN_CH0))
    CurrentMicros[0] = micros();
  else
    In_Duty[0] = micros() - CurrentMicros[0];
}
