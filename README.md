# RFID-Authenticated-Door

This project aims to create a finite state machine (FSM) computer-controlled system in which a locked door can only be opened via verification in the form of an RFID card and an RFID sensor. Utilizing a photoresistor, the system can detect users and prompt them on an LCD to scan their RFID card. Once the card is detected by the RFID scanner, an active buzzer will beep to indicate the card was detected. If the card has a valid UID a passive buzzer will play a specific tone and a motor will open the door, if the card's UID is invalid the passive buzzer will play a different tone and the door will not open and the user will be informed via the LCD that the card wasn’t valid, then they will be prompted once again to scan a valid RFID card. 
