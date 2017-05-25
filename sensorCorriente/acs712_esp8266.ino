//Measuring AC mains energy use the non-invasive current transformer method
//Arduino can receive serial value to open and close relay and set max AC to
// permit flows through relay and sensor (1-30A)
//Using acs712 (30A), its a invasive sensor (non-inductive)
//Based on OpenEnergyMonitor.org, Trystan Lea project
//Sketch calculates - Irms and Apparent power based on static voltage. SetV
// needs to be set below.
//Green IT project licenced under GNU General Public Licence
//Author: Agustin Parmisano
//Based on: Mateo Durante

#define RELAY1    7
#define C_SENSOR1 A0

//For analog read
int r1 = LOW;
int r1_received = LOW;
int incomingByte = 0;   // for incoming serial data
int c_min = 0;
int c_max = 30;

//For analog read
double value;

//Constants to convert ADC divisions into mains current values.
double ADCvoltsperdiv = 0.0048;
double VDoffset = 2.4476; //Initial value (corrected as program runs)

//Equation of the line calibration values
double factorA = 15.35; //factorA = CT reduction factor / rsens
double Ioffset = 0;

//Constants set voltage waveform amplitude.
double SetV = 217.0;

//Counter
int i=0;

int samplenumber = 4000;

//Used for calculating real, apparent power, Irms and Vrms.
double sumI=0.0;

int sum1i=0;
double sumVadc=0.0;

double Vadc,Vsens,Isens,Imains,sqI,Irms;
double apparentPower;

void setup()
{
  Serial.begin(9600);
  pinMode(D4, OUTPUT);
  digitalWrite(D4, LOW);

}

void loop()
{
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    // say what you got:
    //Serial.print("I received: ");
    //Serial.println(incomingByte, DEC);
    if (incomingByte == 48) {
      r1_received = HIGH;
      digitalWrite(RELAY1, HIGH);
      r1 = 0;
    } else if (incomingByte == 49) {
      r1_received = LOW;
      digitalWrite(RELAY1, LOW);
      r1 = 1;
    } else if (incomingByte >= 101 && incomingByte <= 130) {
      // its max current (30 max), prevent carry jump \r (char(13)) readed,
      // so added 100 to current value
      c_max = incomingByte-100;
    }
  }

  value = analogRead(C_SENSOR1);
  int val;
  val = map(value, 0, 767, 0, 512);
  value = val;

  //Summing counter
  i++;

  //Voltage at ADC
  Vadc = value * ADCvoltsperdiv;

  //Remove voltage divider offset
  Vsens = Vadc-VDoffset;

  //Current transformer scale to find Imains
  Imains = Vsens;

  //Calculates Voltage divider offset.
  sum1i++; sumVadc = sumVadc + Vadc;
  if (sum1i>=1000) {VDoffset = sumVadc/sum1i; sum1i = 0; sumVadc=0.0;}

  //Root-mean-square method current
  //1) square current values
  sqI = Imains*Imains;
  //2) sum
  sumI=sumI+sqI;

  if (i>=samplenumber)
  {
    i=0;
    //Calculation of the root of the mean of the current squared (rms)
    Irms = factorA*sqrt(sumI/samplenumber)+Ioffset;
    if (Irms<0.05) {Irms=0;}

    //Calculation of the root of the mean of the voltage squared (rms)


    if (Irms < c_min || Irms > c_max) {
      digitalWrite(RELAY1, HIGH);
      r1 = 0;
    }

    apparentPower = Irms * SetV;
    Serial.print(" A0: ");
    Serial.print(value);
    Serial.print(" Watios: ");
    Serial.print(apparentPower);
    Serial.print(" Voltaje: ");
    Serial.print(SetV);
    Serial.print(" Amperios: ");
    Serial.print(Irms);
    Serial.print(" status: ");
    Serial.print(r1);
    Serial.print(" c_max: ");
    Serial.print(c_max);
    Serial.println();

    //Reset values ready for next sample.
    sumI=0.0;

  }
}
