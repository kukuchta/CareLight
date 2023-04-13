# Budowa i uruchomienie
Aby uruchomić CareLight, należy wykonać kilka kroków. W przypadku płytki głównej ESP-WROOM-32 jest to dość proste.

## Montaż sprzętu
Aby uniknąć lutowania, należy kupić płytki od producenta, który dostarcza już z przylutowanymi złączami goldpin. Często są one po prostu wrzucone luzem do torebki razem z modułem.

Przy pomocy trzech żeńskich przewodów DuPont połącz:
* GND na płytce głównej z masą płytki LED (G/GND/V-)
* VIN na płytce głównej z napięciem dodatnim płytki LED (VCC/+5V/V+)
* D12 na płytce głównej z wejściem danych płytki LED (DIN/IN/INPUT)

## Programowanie

1. Podłącz płytkę główną do komputera za pomocą kabla mini USB, aby sprawdzić, czy jest ona rozpoznawana jako port COM.
  * Otwórz menu Start, znajdź i uruchom Menadżer Urządzeń
  * Szukaj w sekcji Porty (COM i LPT) pozycji:
  
    <img src="./Media/DevMan.png" width="30%" height="30%">
    
  * Jeśli jest obecna, zapisz numer portu COM i przejdź do kolejnego kroku. Jeśli zamiast niej pojawi się nieznane urządzenie, należy pobrać i zainstalować sterownik "CP210x Universal Windows Driver" ze strony Silabs [tutaj](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads) i ponownie podłączyć płytę przez USB. 
2. Pobierz i zainstaluj Arduino IDE w wersji przynajmniej 2.0.4 [tutaj](https://www.arduino.cc/en/software).
3. Zainstaluj wsparcie dla płytek ESP32 Arduino Core, jak opisano [tutaj](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html).
4. Zainstaluj biblioteki Arduino:
  * FastLED by Daniel Garcia v3.5
  * ArduinoJson by Benoit Blanchon v6.21.1
  * StreamUtils by Benoit Blanchon v1.7.3
5. Pobierz kod źródłowy CareLight:

    <img src="./Media/GetSource.png" width="25%" height="25%">
    
6. Otwórz plik CareLink.ino w Arduino IDE
7. Ustaw poniższe parametry w menu Narzędzia:

    <img src="./Media/Params.png" width="25%" height="25%">
    
  Port powinien być zgodny z numerem portu COM znalezionym wcześniej w Menadżerze Urządzeń.
8. Kliknij przycisk Upload żeby skompilować i załadować program na płytkę.

## Konfiguracja

Aby skonfigurować CareLight, należy wykonać kilka kroków. Poniżej opisano je dla płytki głównej ESP-WROOM-32, ponieważ wymaga to najmniejszego wysiłku: 
1. W Arduino IDE otwórz Serial Monitor (menu Tools/Serial Monitor)
2. Zmień prędkość połączenia na 115200 baud
3. Kliknij w pole Message
4. Zresetuj płytę główną, naciskając przycisk EN lub RST
5. Gdy na wyświetlaczu LED zpojawi się okrągłe logo CareLight, naciśnij klawisz Enter, żeby przesłać pustą wiadomość przez port COM. Spowoduje to przejście płytki w tryb konfiguracji. Jeśli jest uruchamiana po raz pierwszy po zaprogramowaniu, domyślnie przejdzie w tryb konfiguracji.
6. Postępuj zgodnie z wyświetlanymi komunikatami w Monitorze szeregowym i potwierdzaj każde ustawienie klawiszem Enter. Jeśli nie zostanie podana nowa wartość, zostanie użyta wartość domyślna lub bieżąca każdego ustawienia (wystarczy nacisnąć Enter).
7. Przy wyborze kolorów CareLight wyświetla je i prosi o potwierdzenie. Można wypróbować kilka kolorów, zanim zostanie potwierdzony właściwy wybór.
8. W przypadku podania niewłaściwej wartości dowolnego parametru zresetuj płytkę i zacznij od początku.

Po zaktualizowaniu ostatniego parametru, urządzenie zapisuje konfigurację w pamięci Flash i resetuje się w normalnym trybie pracy.
