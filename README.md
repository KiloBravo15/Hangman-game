# Hangman-game

# Serwer Wisielca

## Uruchomienie
Aby skompilować i uruchomić serwer, należy wykonać komendę `make run` w głównym katalogu projektu.
Spowoduje to uruchomienie serwera na porcie 8080. Inny port można wskazać, uruchamiając serwer gry bezpośrednio.
Na przykład: `./wisielec-srv 12345`.

## Wiadomości
Każda wiadomość składa się z czterech bajtów, określających jej rozmiar (w konwencji little-endian),
a następnie tylu bajtów zawartości. Pierwszy bajt w zawartości określa typ wiadomości.

* `01` - prośba o dołączenie do gry
* `11` - próba odgadnięcia litery
* `12` - prośba o tabelę wyników

* `41` - odpowiedź na prośbę o dołączenie do gry
* `52` - odpowiedź z tabelą wyników

* `81` - powiadomienie o graczu dołączającym do gry
* `82` - powiadomienie o graczu opuszczającym grę
* `91` - powiadomienie o zmianie wyświetlanego słowa / odsłonięciu liter
* `92` - powiadomienie o przyznaniu punktów graczowi
* `93` - powiadomienie o powieszeniu gracza
