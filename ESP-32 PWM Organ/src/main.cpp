#include <Arduino.h>

#include "MidiNotes.h"
#include "AxelFixed.h"
#include "mozart_eine_kleine_short.h"
#include "LibertyBellHighPart.h"
#include "LibertyBellTwoParts.h"
#include "FurEliseShort.h"

int pwmPins[3] = {4, 16, 17};
int pwmChannels[3] = {0, 2, 4};
const int LedPin = 2;

int channelNotes[3];
int channelPwm[3];

char buffer[128];

void SetPwm(int channel, int value)
{
  ledcWrite(pwmChannels[channel], value);
  channelPwm[channel] = value;
}

void setup()
{
  Serial.begin(115200);
  Serial.println(">Setup");

  pinMode(LedPin, OUTPUT);

  for (int channel = 0; channel < 3; channel++)
  {
    pinMode(pwmPins[channel], OUTPUT);
    ledcSetup(pwmChannels[channel], 500, 8);
    ledcAttachPin(pwmPins[channel], pwmChannels[channel]);
    channelNotes[channel] = 0;
    SetPwm(channel, 255);
  }
  Serial.println("<Setup");
}

float GetFrequencyForMidiNote(int midiNoteNumber)
{
  float frequency = MidiNotes[midiNoteNumber];

  //Serial.print(midiNoteNumber);
  //Serial.print(" ");
  //Serial.println(frequency);

  return frequency;
}

float ComputeTickLengthInMs(int ppqFromHeader, int tempoFromTrack)
{
  // ppqFromHeader = parts per quarter note aka ticks per quarter note
  // tempoFromTrack = uS (microseconds) per quarter note

  // Therefore:
  // tick time in mS = tempoFromTrack / 1000 / ppqFromHeader

  return tempoFromTrack / 1000 / ppqFromHeader;
}

void SetChannels()
{
  for (int channel = 0; channel < 3; channel++)
  {
    if (channelNotes[channel] == 0)
    {
      SetPwm(channel, 256);
    }
    else
    {
      float frequency = GetFrequencyForMidiNote(channelNotes[channel]);
      ledcSetup(pwmChannels[channel], frequency, 8);
      sprintf(buffer, "Set Frequency: %d %d", channel, frequency);
      Serial.println(buffer);
      SetPwm(channel, 64);
    }
  }
  for (int channel = 0; channel < 3; channel++)
  {
    Serial.print("N, F, P: ");
    Serial.print(channelNotes[channel]);
    Serial.print(" ");
    Serial.print(ledcReadFreq(pwmChannels[channel]));
    Serial.print(" ");
    Serial.println(ledcRead(pwmChannels[channel]));
  }
}

void NoteOn(int noteNumber)
{
  for (int channel = 0; channel < 3; channel++)
  {
    if (channelNotes[channel] == 0)
    {
      Serial.print("On: ");
      Serial.print(channel);
      Serial.print(" => ");
      Serial.println(noteNumber);
      channelNotes[channel] = noteNumber;
      SetChannels();
      return;
    }
  }
}

void NoteOff(int noteNumber)
{
  for (int channel = 0; channel < 3; channel++)
  {
    if (channelNotes[channel] == noteNumber)
    {
      Serial.print("Off: ");
      Serial.print(channel);
      Serial.print(" => ");
      Serial.println(noteNumber);
      channelNotes[channel] = 0;
      SetChannels();
      return;
    }
  }
}

void PlaySong(int *pNotes, int noteCount, int ppqFromHeader, int tempoFromTrack)
{
  float tickLengthInMs = ComputeTickLengthInMs(ppqFromHeader, tempoFromTrack);
  Serial.println("Tick length in ms");
  Serial.println(tickLengthInMs);

  for (int channel = 0; channel < 3; channel++)
  {
    channelNotes[channel] = 0;
    ledcWrite(pwmChannels[channel], 255);
  }

  int currentTimeInMs = 0;

  for (int note = 0; note < noteCount; note++)
  {
    Serial.println();

    int noteTime = *pNotes;
    pNotes++;
    int noteNumber = *pNotes;
    pNotes++;
    int velocity = *pNotes;
    pNotes++;

    int noteTimeInMs = tickLengthInMs * noteTime;
    //Serial.print("noteTimeInMs: ");
    //Serial.println(noteTimeInMs);

    if (noteTimeInMs > currentTimeInMs)
    {
      Serial.println("delay");
      Serial.println(noteTimeInMs - currentTimeInMs);
      delay(noteTimeInMs - currentTimeInMs);
      currentTimeInMs = noteTimeInMs;
    }

    Serial.print("Time, note, vel: ");
    Serial.print(noteTime);
    Serial.print(" ");
    Serial.print(noteNumber);
    Serial.print(" ");
    Serial.println(velocity);

    if (velocity == 0)
    {
      NoteOff(noteNumber);
    }
    else
    {
      NoteOn(noteNumber);
    }

    Serial.print("Channels: ");
    Serial.print(channelNotes[0]);
    Serial.print(" ");
    Serial.print(channelNotes[1]);
    Serial.print(" ");
    Serial.println(channelNotes[2]);

    Serial.print("Pwm: ");
    Serial.print(ledcRead(pwmChannels[0]));
    Serial.print(" ");
    Serial.print(ledcRead(pwmChannels[1]));
    Serial.print(" ");
    Serial.println(ledcRead(pwmChannels[2]));
  }
}

int closeEncounters[] = {0, 79, 81,
                         960, 79, 0,
                         960, 81, 81,
                         1920, 81, 0,
                         1920, 77, 81,
                         2880, 77, 0,
                         2880, 65, 81,
                         3840, 65, 0,
                         3840, 72, 81,
                         4800, 73, 0,
                         5800, 0, 0};

void noteScale()
{
  for (int note = 20; note < 123; note++)
  {
    NoteOn(note);
    NoteOn(note);
    NoteOn(note);
    delay(250);
    NoteOff(note);
    NoteOff(note);
    NoteOff(note);
    delay(10);
  }
}

void TwoNoteTest()
{
  for (int i = 0; i < 100; i++)
  {
    NoteOn(81);
    NoteOn(50);
    delay(3000);
    NoteOff(81);
    NoteOff(50);

    NoteOn(78);
    NoteOn(53);
    delay(3000);
    NoteOff(78);
    NoteOff(53);
  }
}

void loop()
{
  PlaySong(FurEliseTwoParts, 646, 256, 800000);
  delay(1000);

  //PlaySong(LibertyBell, 634, 225, 300000);
  //delay(1000);

  PlaySong(LibertyBellTwoParts, 1403, 225, 300000);
  delay(1000);

  PlaySong(mozart, 210, 256, 600000);
  delay(1000);

  //PlaySong(closeEncounters, 11, 480, 500000);

  PlaySong(axelFixed, 92, 300, 508474);
  delay(1000);

  //DoLoop(192);
  //DoLoop(128);
  //DoLoop(64);
}