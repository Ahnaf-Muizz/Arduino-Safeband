# Arduino-Safeband
This Arduino code is to created to work with the Safe School web app 
Safe-School: Multi-Tap Alarm & RFID Attendance System

This Arduino project integrates two critical safety and tracking features into one system:

Multi-Tap Alarm System

2 taps (Concern): Activates buzzer + red LED for 2 seconds.

3 taps (SOS): Triggers a Morse code SOS signal with buzzer + red LED.

Safety Check: Every 10 seconds of inactivity, a red light prompts a check-in tap for user safety.

RFID Attendance Tracker

Uses an MFRC522 RFID reader to scan student/staff cards.

Displays "Attendance Recorded!" on Serial Monitor when a card is scanned.

Plays a confirmation beep + flashes green LED on successful scan.

Resets safety timer whenever a card is scanned.

QR code to Web Application:
<img width="3000" height="3000" alt="qr code" src="https://github.com/user-attachments/assets/8cef515b-461c-4652-9774-f24db3b7e735" />

🔧 Hardware Requirements

Arduino Uno/Nano

RFID Module (MFRC522)

Buzzer (connected to Pin 6)

Pushbutton (Pin 2 → GND)

LEDs:

Red LED → Pin X (for alarms & inactivity warning)

Green LED → Pin Y (for RFID success)

Jumper wires, breadboard, RFID tags/cards

📚 Required Libraries

Install via Arduino Library Manager:

MFRC522

⚡ Pin Configuration (Arduino Uno/Nano)
Component	Pin	Notes
Pushbutton	D2	Pulled LOW when pressed
Buzzer	D6	(Moved from D9 to D6 to avoid conflicts)
RFID RST	D9	
RFID SDA (SS)	D10	
RFID MOSI	D11	
RFID MISO	D12	
RFID SCK	D13	
Red LED	D3	Alarm + inactivity prompt
Green LED	D4	RFID success
🚀 How It Works

Start the system → LEDs + RFID ready.

User taps button:

2 taps → Concern alert.

3 taps → SOS Morse alarm.

RFID card scan:

Records attendance.

Gives green LED + buzzer confirmation.

Resets inactivity safety timer.

No activity for 10s → Red LED prompts a check-in tap.

🛠️ Setup & Upload

Clone this repository:

git clone https://github.com/your-username/Safe-School.git
cd Safe-School


Open Safe-School.ino in Arduino IDE.

Install the MFRC522 library.

Connect your hardware as per pin configuration.

Select your board & port in Arduino IDE.

Upload the sketch to your Arduino.

✅ Future Improvements

Add GSM/WiFi module for remote alerts.

Store attendance logs on SD card or cloud.

Use OLED display for on-device feedback.
