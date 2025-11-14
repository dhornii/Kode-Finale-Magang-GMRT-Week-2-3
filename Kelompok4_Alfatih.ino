/*
===============================================================
Referensi kode serta bacaan untuk memperjelas:
https://randomnerdtutorials.com/esp32-dc-motor-l298n-motor-driver-control-speed-direction/
https://lastminuteengineers.com/esp32-l298n-dc-motor-control-tutorial/


catatan : beberapa syntax dari referensi sudah outdated, jadi bakal ada yang berbeda dengan yang terdapat di kode
===============================================================
*/

// === deklarasi pin driver motor L298N ===
// motor A
#define ENA 14 // enable motor A buat bergerak (roda kiri)
#define IN1 18 // input 1 buat motor A
#define IN2 19 // input 2 buat motor A

// motor B
#define ENB 4  // enable motor B buat bergerak (roda kanan)
#define IN3 27 // input 1 buat motor B
#define IN4 26 // input 2 buat motor B
// orientasi kanan kiri berdasarkan sensor depan, sensor depan jadi arah depan robot

// === deklarasi pin ketiga sensor ultrasonic ===
#define TRIG1 13 // sensor kiri
#define ECHO1 12

#define TRIG2 25 // sensor tengah
#define ECHO2 33

#define TRIG3 32 // sensor kanan
#define ECHO3 35

// === deklarasi karakteristik dari PWM (Pulse Width Modulation) ===
const int freq = 1000;
const int resolution = 8;

// === fungsi untuk mengambil jarak lalu outputnya dalam satuan cm ===
long ambil_jarakCM(int TRIG_PIN, int ECHO_PIN){
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  /*setting timeout pembacaan selama 3 ms (waktu maksimal sebelum lanjut eksekusi baris berikutnya). 
  Artinya jika sinyal tidak kunjung diterima, asumsi jarak adalah 3000 * 0.0343 / 2 = 51.5 cm*/
  long durasi = pulseIn(ECHO_PIN, HIGH, 3000);
  long jarak_cm = (durasi * 0.0343) / 2; 
  return jarak_cm;
}

// === fungsi untuk mengatur gerakan robot sesuai arahnya ===
void gerak_maju(long jarak){
  float limit = jarak - batas_aman;                         // jarak yang perlu ditempuh robot untuk gerakan saat ini agar tetap hasil akhirnya berada di jarak aman
  long waktu_gerak = floor((limit/kecepatan_linear)*1000);  // ambil perkiraan waktu yang dibutuhkan dalam satuan ms untuk melewati jarak limit
                                                            // konsekuensinya robot harus mulai nyala pas udah di dalam arena saja
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH); // putar roda kiri maju

  digitalWrite(IN3, LOW); // putar roda kanan maju
  digitalWrite(IN4, HIGH);

  delay(waktu_gerak); // biarkan selesai menempuh jarak 'limit'

  // hentikan putaran roda robot

  digitalWrite(IN2, LOW);
  digitalWrite(IN4, LOW);

  ledcWrite(ENA, 70); 
  ledcWrite(ENB, 70);
}

void belok_kanan(){         // logikanya : roda kanan bergerak mundur, roda kiri bergerak maju, biar robot mungkin bisa berputar di tempat sehingga menghadap kanan
  digitalWrite(IN1, LOW);   // kiri maju
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);  // kanan mundur
  digitalWrite(IN4, LOW);
  delay(1500);              // perkiraan waktu yang dibutuhkan buat berputar 90 derajat ke arah target

  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
}

void belok_kiri(){          // logikanya : roda kanan bergerak maju, roda kiri bergerak mundur
  digitalWrite(IN1, HIGH);   
  digitalWrite(IN2, LOW);  // kiri mundur

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);  // kanan maju
  delay(1500);

  digitalWrite(IN1, LOW);
  digitalWrite(IN4, LOW);
}

// === variabel tambahan untuk batas robot ===
int batas_aman = 25; // satuan cm
float kecepatan_linear = 6; // butuh dari eksperimen langsung ke robotnya dengan hitung jarak tempuh habis digerakkan maju dengan delay(1000), satuan cm/s

void setup() {
  Serial.begin(115200);
  // persiapan semua pin dan variabel yang perlu :

  // BAGIAN DRIVER MOTOR L298N
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // BAGIAN 3 SENSOR ULTRASONIC
  pinMode(TRIG1, OUTPUT); // output soalnya bakal bertugas memancarkan gelombang ultrasonik
  pinMode(ECHO1, INPUT);  // input soalnya bakal menerima pantulan gelombang yang datanya dibaca oleh mikon

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(TRIG3, OUTPUT);
  pinMode(ECHO3, INPUT);

  // BAGIAN DRIVER DRIVER MOTOR L298N
  ledcAttach(ENA, freq, resolution); // set karakteristik PWM setiap motor
  ledcAttach(ENB, freq, resolution);
  ledcWrite(ENA, 70); // atur kecepatan motor ke 70 dalam rentang [0, 255] karena resolusi 8 --> rentang 0 hingga 2^8 - 1 
  ledcWrite(ENB, 70);
}

void loop() {
  // perulangan aktivitas yang dilakukan robot
  long kiri  = ambil_jarakCM(TRIG1, ECHO1);
  delay(50);
  long depan = ambil_jarakCM(TRIG2, ECHO2);
  delay(50);
  long kanan = ambil_jarakCM(TRIG3, ECHO3);

  Serial.print("Depan: "); 
  Serial.print(depan); 
  Serial.print(" cm");

  Serial.print(" | Kiri: "); 
  Serial.print(kiri); 
  Serial.print(" cm");

  Serial.print(" | Kanan: "); 
  Serial.print(kanan); 
  Serial.println(" cm");

  if(depan > batas_aman){
    gerak_maju(depan);
  }
  else if( (kiri > batas_aman) || (kanan > batas_aman) ){
    if(kanan > batas_aman){
      belok_kanan();
    }
    else{
      belok_kiri();
    }
  }

  /* jika robot sudah di tempat finish, maka ia cuma melaporkan rekamannya saja tanpa bergerak ke manapun. */

  delay(1000); // bagian ini nanti diatur buat kestabilan dan tingat responsivitas robot
}