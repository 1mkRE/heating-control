// ----------- Einfache Heizungs Steuerung ------------- 

#include <Sio51.h>
#include <sfr51.h>
int Wert; // Messwert AD-Wandler
char Einheit; // Einheit Temperatur °C
int Betriebsart; // 0 Manual, 1 Automatik
int amess; // Anzahl Messungen (Mittelwertbildung)
int mwert; // Mittelwert der Messungen
int messen(); // Funktion, die den AD-Wandler steuert und den Messwert skaliert zurueck gibt
void Anzeige(int mwert, char Einheit); // Funktion, zur Messwert Anzeigt. Uebergeben werden der Messwert und die Einheit


void interrupt (0x000B) timer0(void){ 
TR0=0;      // timer 0 stoppen
TH0=0x3C;   // timer 0 laden HB
TL0=0xAF;   // timer 0 laden LB,  
TR0=1;      // timer 0 starten
P3_5!=P3_5; // Toggle Pin 3.5 
P3_5!=P3_5; // Zähler 1 treiben
}


void interrupt (0x001B) timer1(void){ 
TR1=0;                  // Timer 1 stoppen
TH1=0xFF;               // timer 1 laden HB
TL1=0xFB;               // timer 1 laden LB
TR1=1;                  // timer 1 starten
Wert=messen();          // messwert lesen
amess++;                // Anzahl Messungen um eins erhoehen
mwert=mwert+Wert;       // messwerte aufaddieren

if (amess==5){          // Nach fuenf Messungen
    amess=0;            // Anzahl Messungen zuruecksetzen
    mwert=mwert/5;      // Mittelwert bilden
    Anzeige(mwert,'C'); // Funktion Anzeige aufrufen, mit dem Mittelwert der Messungen und der Einheit °C
    mwert=0;            // Mittelwert zuruecksetzen
    }
}


int messen()		// AD-Wandler aufrufen

{
int Wertigkeit;     // Wertigkeit des empfangenen Bit
int Ain;	    // Messwert (0-255)
P1_Clock=1;         // Clock auf 0
P1_CS=1;            // Chip select auf 1
Ain=0;              // Messwert zuruecksetzen
Wertigkeit = 128;   //Wertigkeit auf 128.  Bei 8 Bit-Wandler
P1_CS=0;            // Chip select auf 0;
while( Wertigkeit >=1)
{
    P1_Clock=0;                     // Clock auf 0 
    Ain=P1_DAta*Wertigkeit+Ain;     // Messwert berechnen
    Wertigkeit=Wertigkeit/2;        // Wertigkeit neu festlegen
    P1_Clock=1;                     // Clock auf 1
}

Ain=Ain*80;     //Skalierung 80°C
Ain=Ain/256;    //auf 255 Zustände
return Ain; 	//Messwert an Hauptprogram übergeben
}



main()
{
Betriebsart = 1;               // Im Automatikbetrieb starten 
TR0 = 0;                       // Timer 0 stoppen
TR1 = 0;                       // Timer 1 stoppen
TMOD=TMOD & 0xF0;              // Timerregister für timer 0 löschen
TMOD=TMOD | 0x01;              // timer 0 16 bit Modus
TMOD=TMOD & 0x0F;              // Timerregister für timer 1 löschen
TMOD=TMOD | 0x50;              // timer 1 16 bit Zählermodus ueber Pin 3_5
TH0=0x3C;                      // timer 0 laden HB
TL0=0xAF;                      // timer 0 laden LB
TH1=0xFF;                      // timer 1 laden HB
TL1=0xFB;                      // timer 1 laden LB
TR0 = 1;                       // timer 0 starten
TR1 = 1;                       // timer 1 starten
ET0=1;                         // timer 0 interrupt freischalten
ET1=1;                         // timer 1 interrupt freischalten
Wert=messen();                    // Starttemperatur holen
while (1){                      // Endlosschleife
    if ( P1_S2 ==1 ) {          // S2 gedrueckt
        Betriebsart = 1;        // Btriebsart automatik
        }
    if (P1_S3 == 1) {           // S3 gedrueckt
        Betriebsart = 1;        // Btriebsart manuell
        }
    if ( Betriebsart == 1) {    // Automatikbetrieb
        P1_auto = 0;            // LED fuer Automatikbetrieb ein (Inverter!!)
        P1_manuell = 1;         // LED fuer manuellen Betrieb aus (Inverter!!)
        
        if ( Wert < 40) {       // Temperatur unter 40 °C
            P1_brenner=1;       // Brenner an
            }
         if ( Wert > 45) {       // Temperatur unter 40 °C
            P1_brenner=0;       // Brenner aus
            }
        }
    else {                      // Handbetrieb
        P1_auto = 1;            // LED fuer Automatikbetrieb aus (Inverter!!)
        P1_manuell = 0;         // LED fuer manuellen Betrieb ein (Inverter!!)
        if (P1_brenner_ein==1) {// Taster "Brenner ein" betaetigt
            P1_brenner=1;          // Brenner einschalten
            }
        if (P1_brenner_aus==1) {// Taster "Brenner aus" betaetigt
            P1_brenner=1;           // Brenner ausschalten
            }
             
        }   // else
    }       // while
}           // main

