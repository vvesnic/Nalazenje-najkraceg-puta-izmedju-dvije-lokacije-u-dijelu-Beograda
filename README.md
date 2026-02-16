### Pronalaženje najkraćeg puta (Beograd OSM)

Ovaj projekat implementira algoritam za pronalaženje najkraćeg puta između dvije tačke u dijelu Beograda (DATA JE SLIKA U FOLDERU) koristeći podatke sa OpenStreetMap-a (OSM). Program je napisan u C programskom jeziku i koristi Dijkstrin algoritam.

Šta program radi?

# Učitava mapu: Parsira veliki XML fajl (`map.osm`) koji sadrži podatke o ulicama i objektima u Beogradu.
# Pretraga po imenu: Omogućava korisniku da unese ime ulice ili objekta (npr. "Vukov spomenik") i pronalazi odgovarajući čvor u grafu.
# Pametno povezivanje (Snapping): Ako korisnik izabere tačku koja nije direktno na putu (npr. zgrada fakulteta), program automatski pronalazi najbližu tačku na putu kako bi mogao da izračuna putanju.
# Računanje putanje: Koristi Dijkstrin algoritam da pronađe najkraći put (u metrima) između početne i krajnje tačke.
# Ispis: Prikazuje ukupnu udaljenost i listu ulica/čvorova kroz koje se prolazi.

## Razvojni Put:

Razvoj je tekao kroz nekoliko faza, od jednostavnog prototipa do kompletne aplikacije:

# Početak sa malim podacima (`test_map.xml`):
    Prvo sam kreirao mali, ručno napisani XML fajl (`test_map.xml`) sa samo 4-5 čvorova.
    Na njemu sam razvio i testirao osnovne strukture podataka i sam Dijkstrin algoritam.
    Ovo mi je omogućilo da potvrdim da logika radi prije nego što sam prešao na veće podatke.

# Prelazak na pravu mapu (`map.osm`):
    Kada je algoritam potvrđen, prešao sam na `map.osm` (veliki fajl Beograda).
    Ovdje sam naišao na problem sa bibliotekama (`libxml2`), pa sam odlučio da napišem "sopstveni (custom) parser".

# Implementacija pretrage i UI:
    Dodao sam logiku u `main.c` koja omogućava korisniku da unosi tekst (imena) umjesto samo ID brojeva.
    Implementirao sam pretragu koja prepoznaje i latinične i ćirilične nazive.

# Rješavanje problema i finalizacija:
    Fokusirao sam se na "edge cases" (izolovane tačke, veliki brojevi) i poliranje korisničkog iskustva.

## Tehnički Detalji (Gdje se šta nalazi):

Projekat je modularan i podijeljen u nekoliko fajlova:

# `model/graph.c` & `graph.h`:
    Definiše strukture (čvor) i (ivica/ulica).
    Sadrži hash mapu (`nodeMap`) za brzo pronalaženje čvorova po ID-u.
    Funkcije: `createGraph`, `addNode`, `addEdge`, `findNode`, `findNodesByName`.

# `service/parser.c` & `parser.h`:
    Sadrži "custom XML parser".
    Čita `map.osm` liniju po liniju, prepoznaje `<node>` i `<way>` tagove i popunjava graf.
    Funkcija: `parseMap`.

# `service/pathfinder.c` & `pathfinder.h`:
    Implementacija Dijkstrinog algoritma.
    Koristi Min-Heap (binarni heap) za efikasno pronalaženje sljedećeg najbližeg čvora (ključno za brzinu na velikim mapama).
    Funkcija: `findShortestPath`.

# `utils/geometry.c` & `geometry.h`:
    Sadrži HAVERSINU formulu za izračunavanje stvarne udaljenosti u metrima između dvije GPS koordinate (latituda/longituda).
    Funkcija: `calculateDistance`.

# `utils/levenstajn.c` & `levenstajn.h`:
    Sadrži Levenstajnov algoritam za racunanje edit rastojanja između dva stringa. 
    ! PRVO SE RADI PRETRAGA DIREKTNIH PODUDARANJA (SEKVENCIJALNIM ALGORITMOM),
    ! AKO SE NE PRONADJE NI JEDNO DIREKTNO PODUDARANJE, ONDA USKACE LEVENSTAJNOV ALGORITAM.
    Funkcija: `levenshtein_distance`.

# `main.c`:
    Glavni program. Učitava mapu, komunicira sa korisnikom, poziva pretragu i ispisuje rezultate.
    Sadrži logiku za "snapping" (povezivanje izolovanih tačaka).

## Izazovi i Rješenja (Poteškoće tokom rada):

Tokom razvoja naišao sam na nekoliko ozbiljnih problema koje sam uspješno riješio:

1. Problem: "No path found" (Put nije pronađen)
    Uzrok: Kada korisnik izabere zgradu (npr. "Mašinski fakultet"), taj čvor često nije povezan sa ulicom u OSM podacima (stoji sam za sebe). Algoritam nije mogao da nađe put jer nije bilo ivica.
    Rješenje: Implementirao sam Nearest Node Snapping. Ako je izabrani čvor izolovan, program automatski traži najbliži čvor koji jeste na putu i računa putanju odatle.

2. Problem: "Silent Crash" (Pucanje programa bez greške)
    Uzrok su bili OSM ID-evi čvorova koji su ogromni brojevi (npr. 8275982698), koji ne staju u standardni `long` (32-bita na Windows-u). To je dovodilo do overflow-a i pristupa pogrešnoj memoriji.
    Rješenje: Prebacio sam sve ID-eve na `long long` (64-bita) i koristio `atoll` funkciju za parsiranje.

3. Problem: Čudni simboli u konzoli
    Uzrok je bio to što Windows konzola podrazumijevano ne prikazuje UTF-8 karaktere (ćirilicu i naša slova), pa su se vidjeli "hijeroglifi".
    Rješenje:
        Za Windows (`.exe`): Dodao sam `SetConsoleOutputCP(65001)`.
        Za Linux/WSL: Kompajlirao sam "native" Linux binarni fajl koji koristi sistemski UTF-8.

4. Problem: Zavisnosti
    Uzrok je bio to što je pokušaj korišćenja `libxml2` biblioteke bio komplikovan za podešavanje na Windows/WSL okruženju.
    Rješenje: Napisao sam jednostavan parser koji koristi samo standardne C biblioteke (`stdio.h`, `string.h`).

## Kako se pokrece:

# WSL / Linux:

Kompajliranje:
gcc -o shortest_path main.c model/graph.c service/parser.c service/pathfinder.c utils/geometry.c utils/levenstajn.c -lm

Pokretanje:
./shortest_path map.osm

# Windows MinGW:

Kompajliranje:
x86_64-w64-mingw32-gcc.exe -static -o shortest_path.exe main.c model/graph.c service/parser.c service/pathfinder.c utils/geometry.c utils/levenstajn.c -lm

Pokretanje:
.\shortest_path.exe map.osm
