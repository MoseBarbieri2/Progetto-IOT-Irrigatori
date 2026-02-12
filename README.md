# Sistema di irrigazione indipendente e automatico

## Specifiche

- Alimentazione: a batteria per i sensori, corrente elettrica per il server
- Metodi di comunicazione: MQTT

## Componenti

- Server centrale che comunica con un broker MQTT in cloud che fornisce una semplice dove posso controllare lo stato attuale di tutti i sensori, connettermi a sensori nuovi, e verificare se gli irrigatori a loro collegati sono attualmente attivi o meno.
- Sensori: uno per ogni sottosezione della serra, collegabile a N irrigatori in modo fisico che emetteranno acqua quando l'umidità scende eccessivamente. Si salvano nella EEPROM il loro nome assegnato dall'utente dal server centrale, le due treshold, quante valvole hanno, e i loro pin tramite JSON inviato per MQTT. Deve poter dire al broker lo stato di umidità ogni tot minuti e devo poter modificare i treshold che voglio settare per l'attivazione/disattivazione degli irrigatori (ad esempio attiva se inferiore a 70% e disattiva quando raggiunge 90%)
