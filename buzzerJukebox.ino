#include "pitches.h"

// ── Pin ──────────────────────────────────────────────
const int BUZZER_PIN = 8;

// ── Song data ────────────────────────────────────────
// Song 1: Super Mario Theme (intro)
int mario_notes[] = {
  NOTE_E5,NOTE_E5,0,NOTE_E5,0,NOTE_C5,NOTE_E5,0,
  NOTE_G5,0,0,0,NOTE_G4,0,0,0
};
int mario_dur[] = {
  8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
};

// Song 2: Tetris Theme
int tetris_notes[] = {
  NOTE_E5,NOTE_B4,NOTE_C5,NOTE_D5,NOTE_C5,NOTE_B4,
  NOTE_A4,NOTE_A4,NOTE_C5,NOTE_E5,NOTE_D5,NOTE_C5,
  NOTE_B4,NOTE_C5,NOTE_D5,NOTE_E5,NOTE_C5,NOTE_A4,NOTE_A4
};
int tetris_dur[] = {
  4,8,8,4,8,8,4,8,8,4,8,8,4,8,4,4,4,4,4
};

// Song 3: Happy Birthday
int birthday_notes[] = {
  NOTE_C4,NOTE_C4,NOTE_D4,NOTE_C4,NOTE_F4,NOTE_E4,
  NOTE_C4,NOTE_C4,NOTE_D4,NOTE_C4,NOTE_G4,NOTE_F4
};
int birthday_dur[] = {
  8,8,4,4,4,2,8,8,4,4,4,2
};

// ── Song registry ────────────────────────────────────
struct Song {
  int*  notes;
  int*  durations;
  int   length;
  const char* name;
};

Song songs[] = {
  { mario_notes,    mario_dur,    16, "Super Mario"    },
  { tetris_notes,   tetris_dur,   19, "Tetris Theme"   },
  { birthday_notes, birthday_dur, 12, "Happy Birthday" }
};
const int NUM_SONGS = 3;

// ── Playback state ───────────────────────────────────
int  currentSong  = 0;
int  currentNote  = 0;
bool playing      = false;

unsigned long noteStart    = 0;
unsigned long noteDuration = 0;
bool          inPause      = false;   // true = we're in the silent gap between notes

// ── Helpers ──────────────────────────────────────────
void printStatus() {
  Serial.print("[");
  Serial.print(playing ? "PLAYING" : "PAUSED ");
  Serial.print("] Song ");
  Serial.print(currentSong + 1);
  Serial.print("/");
  Serial.print(NUM_SONGS);
  Serial.print(" - ");
  Serial.print(songs[currentSong].name);
  Serial.print("  (note ");
  Serial.print(currentNote + 1);
  Serial.print("/");
  Serial.print(songs[currentSong].length);
  Serial.println(")");
}

void startNote() {
  Song& s = songs[currentSong];
  int bpm_divisor = 1000;             // tune this to change overall tempo
  int dur = bpm_divisor / s.durations[currentNote];

  if (s.notes[currentNote] == 0) {   // rest
    noTone(BUZZER_PIN);
  } else {
    tone(BUZZER_PIN, s.notes[currentNote], dur * 0.9);
  }

  noteStart    = millis();
  noteDuration = dur;
  inPause      = false;
}

void stopPlayback() {
  noTone(BUZZER_PIN);
  playing = false;
}

void startPlayback() {
  playing = true;
  startNote();
}

void skipSong() {
  noTone(BUZZER_PIN);
  currentSong = (currentSong + 1) % NUM_SONGS;
  currentNote = 0;
  inPause     = false;
  Serial.print("Skipped to: ");
  Serial.println(songs[currentSong].name);
  if (playing) startNote();
}

void printHelp() {
  Serial.println("─────────────────────────────");
  Serial.println(" BUZZER JUKEBOX - Commands");
  Serial.println("─────────────────────────────");
  Serial.println("  p  → Play / Pause");
  Serial.println("  n  → Next song");
  Serial.println("  s  → Status");
  Serial.println("  h  → Help");
  Serial.println("─────────────────────────────");
}

// ── Setup ────────────────────────────────────────────
void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  printHelp();
  printStatus();
}

// ── Loop ─────────────────────────────────────────────
void loop() {

  // ── Serial input ──
  if (Serial.available()) {
    char cmd = tolower(Serial.read());

    if (cmd == 'p') {
      if (playing) {
        stopPlayback();
        Serial.println("⏸  Paused.");
      } else {
        startPlayback();
        Serial.println("▶  Playing.");
      }
      printStatus();
    }
    else if (cmd == 'n') {
      skipSong();
      printStatus();
    }
    else if (cmd == 's') {
      printStatus();
    }
    else if (cmd == 'h') {
      printHelp();
    }
  }

  // ── Note sequencer ──
  if (!playing) return;

  unsigned long now     = millis();
  unsigned long elapsed = now - noteStart;

  if (!inPause && elapsed >= noteDuration * 9 / 10) {
    // Short silent gap between notes (10 % of note duration)
    noTone(BUZZER_PIN);
    inPause = true;
  }

  if (elapsed >= noteDuration) {
    // Advance to next note
    currentNote++;
    Song& s = songs[currentSong];

    if (currentNote >= s.length) {
      // Song finished – loop it
      currentNote = 0;
      Serial.print("Looping: ");
      Serial.println(s.name);
    }
    startNote();
  }
}