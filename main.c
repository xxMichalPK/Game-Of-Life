#include <stdio.h>
#include <memory.h>
#include <stdlib.h>

// Importy dla funkcji sleep na różnych platformach
#ifdef _WIN32
    #include <windows.h>
#else
    #include <time.h>
    #include <unistd.h>
#endif

// Kod ANSI do czyszczenia ekranu
const char CLEAR_SCREEN[] = "\033[2J\033[1;1H";
// Kod ANSI przesuwający kursor do pozycji (0,0)
const char SET_CURSOR_HOME[] = "\033[H";

// Wymiary mapy
#define MAP_WIDTH 80
#define MAP_HEIGHT 25
#define MAP_CHARS_PER_LINE (MAP_WIDTH + 1)  // Liczba znaków na każdą linię (szerokość mapy + 1 dla '\n')
#define MAP_SIZE MAP_CHARS_PER_LINE * MAP_HEIGHT

// Backbuffer na który wypisujemy znaki
unsigned char mapBackBuffer[MAP_SIZE + 1];

// Funkcja zatrzymujaca program na określony czas w millisekundach
void sleep_ms(int milliseconds) {
#ifdef WIN32
    Sleep(milliseconds);    // Na Windowsie funkcja Sleep przyjmuje czas w millisekundach więc wywołujemy ją
#else
    // Na Linuxie musimy użyć kombinacji sleep[s] oraz usleep[ms]
    if (milliseconds >= 1000)
        sleep(milliseconds / 1000);
    usleep((milliseconds % 1000) * 1000);
#endif
}

// Funkcja licząca żywych sąsiadów komórki o danych koordynatach
int count_live_neighbours(char* frame, int x, int y) {
    int count = 0;

    // Sprawdzamy cały "kwadrat" do okoła podanej komórki
    for (int i = y - 1; i <= y + 1; i++) {
        for (int j = x - 1; j <= x + 1; j++) {
            if (j == x && i == y) continue; // Pomijamy komórkę, której sąsiadów szukamy
            if (i < 0) continue;            // Wychodzimy poza planszę więc tam nie ma żadnych sąsiadów
            if (i >= MAP_HEIGHT) continue;  // tak samo tutaj...
            
            // Implementacja mapy w kształcie walca
            int j2 = j;
            if (j < 0) j2 = MAP_WIDTH - 1;  // Jeżeli indeks komórki wychodzi "przed mapę" musimy go przesunąć na koniec
            if (j >= MAP_WIDTH) j2 = 0;     // I na odwrót z końca na początek

            // Sprawdzamy czy dana komórka jest "żywa"
            if (frame[i * MAP_WIDTH + j2] == 1) count++;
        }
    }

    return count;   // Zwracamy ilość "żywych" sąsiadów
}

// Główna funkcja programu (funkcja main)
int main() {
    // Alokujemy pamięć dla poszczególnych klatek
    // klatki przechowują wartości 0/1, czyli czy komórka na danej pozycji jest martwa (0) czy żywa (1)
    char* nextFrame = (char *)calloc(MAP_WIDTH * MAP_HEIGHT + 1, sizeof(char));
    char* displayedFrame = (char *)calloc(MAP_WIDTH * MAP_HEIGHT + 1, sizeof(char));

    printf("%s", CLEAR_SCREEN);

    // Tworzymy pierwszą iterację mapy poprzez użycie funkcji rand
    srand(time(NULL));
    for (int i = 0; i < MAP_SIZE; i++) {
        displayedFrame[i] = (rand() % 10) >= 8;
    }

    while (1) {
        // Czyścimy klatkę, która będzie użyta jako nowa klatka
        memset(nextFrame, 0, MAP_WIDTH * MAP_HEIGHT);

        // Generujemy nową mapę na podstawie dotychczasowej "klatki"
        for (int i = 0; i < MAP_HEIGHT; i++) {
            for (int j = 0; j < MAP_WIDTH; j++) {
                // Liczymy sąsiadów komórki
                int neighbours = count_live_neighbours(displayedFrame, j, i);

                // Zgodnie z zasadami jeśli komórka jest żywa i ma 2 lub 3 sąsiadów pozostaje żywa, inaczej "umiera"
                // A jeżeli jest "martwa" i ma dokładnie 3 sąsiadów, staje się znów żywa
                if (displayedFrame[i * MAP_WIDTH + j] == 1) {
                    if (neighbours < 2 || neighbours > 3) nextFrame[i * MAP_WIDTH + j] = 0;
                    else nextFrame[i * MAP_WIDTH + j] = 1;
                } else {
                    if (neighbours == 3) nextFrame[i * MAP_WIDTH + j] = 1;
                    else nextFrame[i * MAP_WIDTH + j] = 0;
                }
            }
        }

        // Zmiana wskaźników dotychczasowej i kolejnej "klatki"
        // tzw. buffer swap
        unsigned char *tmp = displayedFrame;
        displayedFrame = nextFrame;
        nextFrame = tmp;

        // Tworzenie nowej mapy (graficznie) na podstawie wygenerowanych komórek
        for (int i = 0; i < MAP_HEIGHT; i++) {
            for (int j = 0; j < MAP_WIDTH; j++) {
                // Jeżeli komórka jest martwa wyświetlamy kropkę
                if (displayedFrame[i * MAP_WIDTH + j] == 0) {
                    mapBackBuffer[i * MAP_CHARS_PER_LINE + j] = '.';
                    continue;
                }
                // Inaczej gwiazdkę
                mapBackBuffer[i * MAP_CHARS_PER_LINE + j] = '*';
            }

            // Na koniec każdego "rzędu" dodajemy znak nowej linii "\n"
            mapBackBuffer[i * MAP_CHARS_PER_LINE + MAP_WIDTH] = '\n';
        }

        // Drukujemy całą mapę na raz aby zmniejszyć migotanie
        printf("%s%s", SET_CURSOR_HOME, mapBackBuffer);
        sleep_ms(100);  // Czekamy 100ms
    }

    // Zwalniamy pamięć (choć i tak nigdy nie wychodzimy z pętli)
    free(nextFrame);
    free(displayedFrame);
    return 0;
}