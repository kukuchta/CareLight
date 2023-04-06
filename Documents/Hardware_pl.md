# Sprzęt

## Płytka główna

Program jest przeznaczony do uruchamiania na chipie WiFi/BT serii ESP32. Na rynku są dostępne podobne serie, takie jak ESP32-S2, -S3, -C3 lub -C6, ale różnią się one ilością pamięci RAM, peryferiami itp. Obecnie tylko zwykły ESP32 został przetestowany jako rozwiązanie.

Dostępne są różne płytki deweloperskie zawierające ten chip, takie jak ESP-WROOM-32, Lolin32 Lite, Wemos D1 R32, Wemos D1 Mini ESP-32, ESP32-CAM itp. Jedynym realnym wymaganiem dla płytki jest dostępność 3 pinów do podłączenia matrycy LED:
* Masa, pin GND
* 1 pin wyjściowy, GPIO12 zostałokreślony na sztywno w kodzie, ale po zmianie może to być dowolny pin GPIO
* Zasilanie, pin +5V - Nie każda płyta ma dedykowany pin, którego można użyć jako źródła napięcia USB +5V.

Wiele płyt ma tylko piny wyjściowe o napięciu 3,3V, ponieważ to jest napięcie, z którym pracuje ESP32. Takie napięcie nie będzie wystarczające do zasilenia matrycy diod LED.
* ESP-WROOM-32 (do kupienia [tutaj](https://allegro.pl/listing?string=esp-wroom-32)) ma pin VIN, który spełnia to zadanie, 
* Lolin32 Lite (do kupienia [tutaj](https://allegro.pl/listing?string=lolin32%20lite)) nie ma dedykowanego pinu, ale po niewielkiej modyfikacji przy użyciu noża i lutownicy (patrz poniżej) można to poprawić. 

### Modyfikacja Lolin32 Lite

Płytka jest warta rozważenia, ponieważ jest mniejsza niż ESP-WROOM-32, dużo tańsza i zapewnia obsługę ładowania baterii Li-Poly bez dodatkowego sprzętu (CareLight może być przez pewien czas całkiem bezprzewodowy :).

Aby przekształcić pin GPIO 14 w źródło napięcia +5V:
1. Przeciąć cienką ścieżkę prowadzącą do pinu 14 ostrym nożem (odłączamy go od chipa ESP32)
2. Lutujemy mostek od pinu 14 do wejściowego pinu regulatora napięcia 3,3V na płytce (łączymy pin 14 z punktem, w którym jest dostępne napięcie zarówno z USB, jak i Li-Poly)

W ten sposób mamy dostępne trzy sąsiadujące piny - G, 12 i 14 (przekształcony w +5V) - gotowe do połączenia z matrycą diod LED.

## Płytka z diodami LED

Program jest przeznaczony do wyświetlania danych na macierzy 64 diod RGB WS2812B ułożonych w kwadrat 8x8. Diody powinny być połączone w taki sposób, że indeks diody w każdym rzędzie zwiększa się od lewej do prawej, a ostatnia dioda w rzędzie jest połączona z pierwszą diodą w następnym rzędzie (bez zmiany kierunku w kolejnych rzędach). To najczęściej spotykany układ diod LED w tanich modułach dostępnych na allegro ([tutaj](https://allegro.pl/listing?string=8x8%20ws2812b)) lub w Chinach ([tutaj](https://aliexpress.com/w/wholesale-8x8-ws2812b.html)).

Zamienione kierunki lub pocięte strzałki są skutkami niekompatybilnego układu diod LED na płytce, ale można to naprawić w kodzie (do zrobienia w przyszłości).

Zwykle na płytce z diodami LED są dwa zestawy po 3 piny. Zestaw, który jest nam potrzebny, składa się z napięcia zasilającego (VCC/+5V/V+), masy (G/GND/V-) i wejścia danych (DIN/IN/INPUT). Inny zestaw z wyjściem danych (DOUT/OUT/OUTPUT) służy do połączenia kolejnych diod LED w łańcuchu (nie jest tutaj używany).

## Kable połączeniowe

Potrzebne są tylko 3 przewody żeńsko-żeńskie typu DuPont/goldpin. Oczywiście można zlutować wszystkie połączenia dla większej trwałości.
