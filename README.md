# NEG – Verhandlungsspiel

Eigenständige erste Grundlage für ein späteres Minispiel im Sachsenkrieg von
„DieFranken“. Der Spielkern ist eine C++17-Bibliothek; `NegInit` erlaubt das
Ausprobieren auf der Kommandozeile. Keine externen Bibliotheken und keine
Netzwerkzugriffe zur Laufzeit.
Programmausgaben, Fehlermeldungen und Codekommentare sind ausschließlich englisch;
die Dokumentation ist deutsch.

## Regeln

Zu Beginn werden unterschiedliche verfügbare Waren zufällig ausgewählt. Jede
Position der geheimen Folge wird unabhängig und gleichverteilt mit einer dieser
Waren besetzt. Wiederholungen sind erlaubt; verfügbare Waren müssen nicht alle
vorkommen. Die Geheimfolge bleibt während der Partie verborgen.

| Kennung | Ware | Englischer Name |
|---|---|---|
| W | Feuerholz | Wood |
| F | Nahrung | Food |
| P | Stämme | Pole |
| T | Balken | Timber |
| B | Bretter | Board |
| S | Stroh | Straw |

| Schwierigkeit | Verfügbare Waren | Positionen | Maximale Versuche |
|---|---:|---:|---:|
| 1 | 3 | 3 | 3 |
| 2 | 3 | 4 | 3 |
| 3 | 4 | 4 | 4 |
| 4 | 4 | 5 | 4 |
| 5 | 5 | 5 | 5 |
| 6 | 5 | 6 | 5 |
| 7 | 6 | 6 | 6 |

Pro Versuch genau einen verfügbaren Warenbuchstaben je Position eingeben.
Groß- und Kleinschreibung werden akzeptiert; Leerraum an den Rändern wird
entfernt. Interner Leerraum, falsche Länge und nicht verfügbare Waren sind Fehler.
Die Konsole beendet sich dann sofort; der Kern verbraucht keinen Versuch.

`X` bedeutet richtige Ware an genau dieser Position, `-` eine falsche Ware.
Es gibt keine Hinweise auf andere Positionen und keine gesperrten Positionen.
Nach jedem gültigen Versuch erscheinen Feedback und verbleibende Versuche.
Eine vollständig richtige Eingabe gewinnt, auch beim letzten Versuch. Ansonsten
endet die Partie nach Ausschöpfen der Versuche mit Niederlage und Aufdeckung.
Die Versuchszahl entspricht der Warenanzahl und erlaubt systematischen Ausschluss;
ein Solver oder eine automatische Hilfe ist nicht enthalten.

## Bauen und testen

Voraussetzungen: CMake ab 3.16 und ein C++17-Compiler.

Windows mit Visual Studio 2022 und installierter C++-Desktopentwicklung,
im Repository-Verzeichnis:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\Release\NegInit.exe 1
```

Linux mit GCC:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/NegInit 1
```

Für CMake 3.16 kann CTest alternativ direkt im Build-Verzeichnis gestartet werden:
`cd build` und `ctest -C Release --output-on-failure`.

## Beispiel

Illustrativer Ablauf für verfügbare Waren `WFS` und Geheimfolge `WFF`:

```text
Available goods: WFS
W=Wood F=Food P=Pole T=Timber B=Board S=Straw
Positions: 3
Maximum attempts: 3
Input: SFW
Feedback: -X- | Remaining attempts: 2
Input: WFS
Feedback: XX- | Remaining attempts: 1
Input: WFF
Feedback: XXX | Remaining attempts: 0
You won!
```

Waren und Geheimfolge werden beim normalen Start zufällig erzeugt; dieser Ablauf
ist als fester Aufbau im Kerntest abgedeckt. Ein reproduzierbarer CLI-Fehlerablauf
unter PowerShell ist `"???" | .\build\Release\NegInit.exe 1`: Nach den Startdaten
erscheint eine Fehlermeldung, `$LASTEXITCODE` ist 2. Unter Linux entsprechend
`printf '???\n' | ./build/NegInit 1`.

| Exitcode | Bedeutung |
|---|---|
| 0 | Gewonnen |
| 1 | Verloren |
| 2 | Aufruf-/Eingabefehler, vorzeitiges Eingabeende oder Initialisierungsfehler |

Genau ein Startparameter ist erlaubt: eine Ziffer von `1` bis `7`. Andere
Schreibweisen wie `01` oder `+1` werden abgelehnt. Bei Aufruffehlern erscheint
zusätzlich ein Nutzungshinweis. EOF während einer laufenden Partie ist ein Fehler.

## Aufbau und Schnittstelle

- `include/neg/game.hpp`, `src/game.cpp`: Konfiguration, Zufallserzeugung,
  Validierung, Bewertung, Versuchszähler und Status ohne Konsolen-Ein-/Ausgabe.
- `src/main.cpp`: Parameterprüfung, Einlesen, Anzeigen und Prozess-Exitcodes.
- `tests/`: Tests ohne externe Testbibliothek, über CTest ausführbar.

`neg::Game(difficulty)` verwendet `std::mt19937` mit einem Seed aus
`std::random_device`. `neg::Game(difficulty, seed)` ermöglicht reproduzierbare
Initialisierung ohne zusätzlichen CLI-Parameter. Die genaue erzeugte Folge muss
nicht zwischen verschiedenen Standardbibliotheken übereinstimmen, da Shuffle und
Verteilungen implementationsabhängig sein können.

`neg::Game(difficulty, availableGoods, secret)` erlaubt feste Bewertungsfälle und
prüft Anzahl, eindeutige feste Warenkennungen sowie Länge und Inhalt der Folge.
Dieser Konstruktor erwartet Großbuchstaben. Fehlerhafte Konstruktorargumente
werfen `std::invalid_argument`. `submit()` liefert Annahme, Feedback und eine
Fehlermeldung; abgelehnte Eingaben ändern keinen Zustand. Nach Spielende werden
alle weiteren Versuche abgelehnt. Getter geben Status und Versuchszähler zurück.
Der lesende Zugriff auf `secret()` dient Tests und der Aufdeckung nach Niederlage;
eine spätere Oberfläche ist für das Verbergen während der Partie verantwortlich.
Eine Unreal-Oberfläche kann die Bibliothek ohne Änderung der Spiellogik verwenden.

## Prüfstand

Die Grundlage wurde unter Windows mit Visual Studio 2022/MSVC in Release gebaut
und mit CTest geprüft. Kerntests prüfen alle Konfigurationen, Erzeugungsinvarianten
für feste Seeds, reproduzierbare Initialisierung, konkrete Feedbackfälle,
Validierung und Endzustände. CLI-Tests prüfen fehlerhafte Parameter, ungültige
Eingaben und EOF. Linux/GCC wurde in dieser Umgebung nicht ausgeführt.

Nicht enthalten sind Unreal-Anbindung, grafische Oberfläche, Warenverbrauch,
Spielstände, Datenbank und Netzwerk.
