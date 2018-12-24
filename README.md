# pru-edma-pwm
### Controllo PWM tramite EDMA. 

Una semplice sperimentazione della componente EDMA su Arm335x di BeagleBone Black.
Genero segnale DSHOT (segnale di controllo per ESC di nuova generazione) in modo continuo, senza impegno della CPU.

Il buffer sorgente contiene valori compresi tra 0 e 166. In main.c: 

```java
uint16_t FRAME_TO_TRANSFER[NUM_FRAME_BLOCK] = { 125, 63, 63, 125, 125, 63, 125,
                                           125, 125, 63, 63, 63, 125, 63,
                                           125, 63, 1 };
```
                                          
Il periodo PWM è impostato in modo da generare una frequenza di 600KHz (DShot600).
EDMA è impostata in modo A-Sync e trasferisce due byte del buffer sorgente nel registro CMPA della componente PWM0 
ogni volta che termina il periodo.
Quando ha terminato di trasferire l'intero buffer, ricomincia in modo da generare un segnale continuo.

Nota: il canale DMA (l'interrupt PWM0 è sul canale 14) deve essere abilitato da Linux (script abilita.sh). 
Credo che dipenda dal fatto che la PRU non ha i privilegi per poterlo fare.
Dopo aver creato il canale, è possibile 'installare' il firmware sulla PRU (PRU0) con lo script install-edma.sh

Il reset dell'interrupt PWM0 al termine del periodo, è necessario e a carico della CPU PRU0 
affinchè possa essere generato l'evento DMA per il ciclo di trasferimento dei due bytes successivi.

Questo il motivo per cui in main.c si ha:
```java
    while (1)
    {
        // reset interrupt fine periodo PWM
        if(PWMSS0.EPWM_ETFLG & 0x0001) {
            PWMSS0.EPWM_ETCLR = 1;
        }
        pru_rpmsg_lib_Pulse(); // verifica se ci sono nuovi messaggi provenienti da ARM ed eventualmente li esegue
    }
```
Si tratta solo di una sperimentazione per verificare le interazioni tra EDMA ed altre componenti hardware nel SoC.
Per il controllo motori, lo scenario che si ipotizza è il seguente:

### Probabili step per il controllo motore

* Il controllore di volo definisce il canale e due buffer sorgenti da usare in modalità ping/pong (cache a due livelli) con la DMA.
* La PRU configura EDMA (ParameterSet)
* Il controllore di volo scrive sul buffer corrente (es. ping) i valori richiesti per il motore
* EDMA trasferisce su PWM il secondo buffer (es. pong)
* EDMA invia interrupt al controllore di volo al termine del trasferimento ed inizia il trasferimento del primo buffer (es. ping)
* Il controllore di volo esegue il reset dell'interrupt e trasferisce nel secondo buffer (es. pong) i nuovi dati richiesti.


