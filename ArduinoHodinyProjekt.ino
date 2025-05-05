#include <Wire.h>
#include <RTClib.h>
#include <TM1637Display.h>
#include <Timer.h>
#include <stdlib.h>

#define CLK 2
#define DIO 3

RTC_DS3231 rtc;
TM1637Display tm1637(CLK, DIO);
Timer t;

typedef struct {
  int timeInMinutes; // Čas události v minutách od půlnoci
} event_t;

typedef struct {
  event_t *data;     // Dynamické pole událostí
  size_t count;      // Aktuální počet uložených událostí
  size_t capacity;   // Kapacita pole
} event_arr_t;

event_arr_t events;

size_t currentEventIndex = 0;
int mainStep = 0;
int mainTimerId = -1;
int eventTimerId = -1;

// Segmentový kód pro zobrazení "0"
const uint8_t segmentZero = SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
const uint8_t off = 0x00;

/* Inicializace dynamického pole událostí */
void arrInit(event_arr_t *a) {
  a->data = NULL;
  a->count = 0;
  a->capacity = 0;
}

/* Porovnávací funkce pro qsort - řazení událostí podle času */
int sortFn(const void *a, const void *b) {
  const event_t *aPtr = (const event_t *)a;
  const event_t *bPtr = (const event_t *)b;
  if (aPtr->timeInMinutes < bPtr->timeInMinutes) return -1;
  if (aPtr->timeInMinutes > bPtr->timeInMinutes) return 1;
  return 0;
}

/* Seřazení pole událostí podle času */
void sort(event_arr_t *a) {
  qsort(a->data, a->count, sizeof(event_t), sortFn);
}

/* Přidání nové události do pole a automatické rozšíření a seřazení */
void addData(event_arr_t *a, event_t *e) {
  if (a->count == a->capacity) {
    a->capacity = a->capacity * 2 + 10;
    a->data = (event_t *)realloc(a->data, a->capacity * sizeof(event_t));
  }
  a->data[a->count++] = *e;
  sort(a);
}

/* Nastavení systému: RTC, displej, inicializace pole, start hlavního cyklu */
void setup() {
  Serial.begin(9600);

  if (!rtc.begin()) {
    Serial.println("RTC modul nebyl nalezen!");
    while (1);
  }

  if (rtc.lostPower()) {
    // Pokud ztratil napájení, nastav výchozí čas
    rtc.adjust(DateTime(2025, 4, 27, 19, 40, 0));
  }

  tm1637.setBrightness(5);
  tm1637.clear();

  arrInit(&events);
  mainTimerId = t.every(4000, mainDisplayCycle); // Spuštění hlavního cyklu zobrazení
}

/* Hlavní smyčka programu – aktualizuje časovače a čte vstup */
void loop() {
  t.update();
  getIncomingTime();
}

/* Zpracování příchozího času ze sériového portu a uložení do seznamu událostí */
void getIncomingTime() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    int eventTimeInMinutes = input.substring(0, input.indexOf(',')).toInt();
    event_t newEvent = {eventTimeInMinutes};
    addData(&events, &newEvent);
  }
}

/* Zobrazení aktuálního času na displeji ve formátu HHMM */
void showTime() {
  DateTime now = rtc.now();
  int displayTime = now.hour() * 100 + now.minute();
  tm1637.showNumberDecEx(displayTime, 0b01000000, true); // Zobrazení s dvojtečkou
}

/* Zobrazení aktuálního data na displeji ve formátu DDMM */
void showDate() {
  DateTime now = rtc.now();
  int displayDate = now.day() * 100 + now.month();
  tm1637.showNumberDecEx(displayDate, 0b01000000, true); // Zobrazení s dvojtečkou
}

/* Animace zleva doprava nebo zpět po jednotlivých segmentech */
void runRightAnimationTimed(unsigned long durationMillis, bool back = false) {
  unsigned long start = millis();
  int i = back ? 3 : 0;
  bool forward = !back;

  while (millis() - start < durationMillis) {
    uint8_t segs[4] = {off, off, off, off};
    segs[i] = segmentZero;
    tm1637.setSegments(segs);
    delay(150);

    if (forward) {
      i++;
      if (i > 3) i = 0;
    } else {
      i--;
      if (i < 0) i = 3;
    }
  }

  // Po dokončení animace nastav výchozí stav segmentů
  uint8_t segs[4] = {off, off, off, off};
  segs[0] = segmentZero;
  tm1637.setSegments(segs);
}

/* Zobrazení jedné události z pole; po zobrazení všech se vrací na hlavní cyklus */
void showSingleEvent() {
  if (currentEventIndex < events.count) {
    int eventTime = events.data[currentEventIndex].timeInMinutes;
    int eventHour = eventTime / 60;
    int eventMinute = eventTime % 60;
    int displayValue = eventHour * 100 + eventMinute;
    tm1637.showNumberDecEx(displayValue, 0b01000000, true);
    currentEventIndex++;
  } else {
    t.stop(eventTimerId);
    runRightAnimationTimed(2000, true); // zpětná animace
    currentEventIndex = 0;
    mainStep = 0;
    mainTimerId = t.every(4000, mainDisplayCycle); // návrat do hlavního cyklu
  }
}

/* Hlavní zobrazovací cyklus – střídá čas, datum a události */
void mainDisplayCycle() {
  switch (mainStep) {
    case 0:
      showTime();
      mainStep++;
      break;
    case 1:
      showDate();
      mainStep++;
      break;
    case 2:
      if (events.count > 0) {
        t.stop(mainTimerId); // zastavení hlavního cyklu
        runRightAnimationTimed(2000); // animace vpřed
        currentEventIndex = 0;
        eventTimerId = t.every(4000, showSingleEvent); // začít zobrazovat události
      } else {
        mainStep = 0; // pokud nejsou události, pokračuj časem
        mainDisplayCycle();
      }
      break;
  }
}
