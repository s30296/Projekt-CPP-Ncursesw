#include <iostream>
#include <ncursesw/curses.h>
#include <time.h>
#include <queue>

// Tworzenie obiektow na planszy gry
class Create {
	public:
		int y = 0;
		int x = 0;
		char character;
		
		// Podstawowy/pusty konstruktor
		Create() {}
		
		// Konstruktor rysowania znakow
		Create(int y, int x, char character) : y(y), x(x), character(character) {}
		
		// Zwrocenie danych szerokosci
		int getX() {
			return x;
		}
		
		// Zwrocenie danych wyskosci
		int getY() {
			return y;
		}
		
		// Zwrocenie znaku
		char getCharacter() {
			return character;
		}
};

// Klasa z jedzeniem
class Food : public Create {
	public:
		// Konstruktor jedzenia dziedziczacy z Create
		Food(int y, int x) : Create(y, x, '*') {}
};

// Klasa ze spacja
class Space : public Create {
	public:
		// Konstruktor pustego znaku dziedziczacy z Create
		Space(int y, int x) : Create(y, x, ' ') {}
};

// Okno/obszar gry
class GameWindow {
	private:
		WINDOW *gameWindow; // Wczytanie okna pod wskaznik
		int height = 0;
		int width = 0;
		int y = 0;
    	int x = 0;
		int gameWindowBuilder(int height, int width) {
    		getmaxyx(stdscr, y, x); // Maksymalna wysokosc i szerokosc terminala
    		gameWindow = newwin(height, width, y / 2 - height / 2, x / 2 - width / 2); // Obszar gdzie ma byc narysowana granica okna, calosc jest wycentrowana
    		wtimeout(gameWindow, 150); // Ustawienia podstawowego/pierwszego czasu gry
		}
	public:	
		// Podstawowy/pusty konstruktor
		GameWindow() {}
		
		// Konstruktor okna gry
		GameWindow(int height, int width) : height(height), width(width) {
			gameWindowBuilder(height, width);
		}

		// Inicjalizator okna/granicy gry
		int initBorder() {
			wclear(gameWindow); // Wyczyszcznie okna gry, potrzebne do restartu
    		wattron(gameWindow, COLOR_PAIR(1)); // Wlaczenie palety kolorow dla okna gry
    		wattron(gameWindow, A_BOLD); // WLaczenie pogrubienia lini
    		box(gameWindow, '|', '-'); // Rysowanie okna gry
    		wattroff(gameWindow, A_BOLD); // Wylaczenie pogrubienia linii
    		wattroff(gameWindow, COLOR_PAIR(1)); // Wylaczenie palety kolorow, by nie nachodzila na reszte kodu
    		refreshGameWindow(); // Odswiezenie zawartosci okna gry
		}
		
		// Funkcja do odswiezenia zawartosci okna gry z zewnatrz
		int refreshGameWindow() {
			wrefresh(gameWindow);
		}
		
		// Ustawienie czasu, potrzebne do przyspieszania gry
		int setTimeout(int timeout) {
			wtimeout(gameWindow, timeout);
		}
		
		// Uzyskanie wejscia z klawiatury gracza
		char getInput() {
			char input = wgetch(gameWindow);
			return input;
		}

		// Zlokalizowanie pozycji
		char getPos(int y, int x) {
			return mvwinch(gameWindow, y, x);
		}
		
		// Zwrocenie wartosci wiersza okna gry
		int getRow() {
			return y / 2 - height / 2;
		}
		
		// Zwrocenie wartosci kolumny
		int getCol() {
			return x / 2 - width / 2;
		}
		
		// Umieszczenie obiektu w granicach obszaru okna gry
		int placeObject(int height, int width, char character) {
			return mvwaddch(gameWindow, height, width, character);
		}
		
		// Dodanie obiektu, ktory moze byc narysowany. Jest to jedna funkcja bez wzgledu na rodzaj znaku, ktory ma byc przestawiony
		int addObject(Create create) {
			return placeObject(create.getY(), create.getX(), create.getCharacter());
		}
		
		// Tworzenie losowych koordynatow na ktorych ma pojawic sie obiekt
		int random(int &y, int &x) {
			while((mvwinch(gameWindow, y = rand() % height, x = rand() % width)) != ' ');
			return y, x;
		}
};

class Info {
	WINDOW *infoWindow; // Wczytanie okna pod wskaznik
	public:
		// Podstawowy/pusty konstruktor
		Info() {}
		
		// Konstruktor info
		Info(int size, int col, int row) {
			infoWindow = newwin(7, size, col, row);
		}
		
		// Inicjator tabelki informacyjnej
		int initInfo(int points, int timeout) {
			clear();
			wattron(infoWindow, COLOR_PAIR(2));
			mvwprintw(infoWindow, 0, 0, "w,a,s,d ");
			mvwprintw(infoWindow, 1, 0, "r");
			mvwprintw(infoWindow, 2, 0, "ENTER ");
			mvwprintw(infoWindow, 3, 0, "ESC");
			mvwprintw(infoWindow, 4, 0, "");
			mvwprintw(infoWindow, 5, 0, "PREDKOSC: ");
			mvwprintw(infoWindow, 6, 0, "WYNIK: ");
			wattroff(infoWindow, COLOR_PAIR(2));
			updateInfo(points, timeout);
			refresh();
		}
		
		// Aktualizacja dla tabelki informacyjnej
		int updateInfo(int points, int timeout) {
			wattron(infoWindow, COLOR_PAIR(3));
			mvwprintw(infoWindow, 0, 8, "- STEROWANIE");
			mvwprintw(infoWindow, 1, 8, "- RESTART");
			mvwprintw(infoWindow, 2, 8, "- PAUZA");
			mvwprintw(infoWindow, 3, 8, "- WYJSCIE Z GRY");
			mvwprintw(infoWindow, 5, 10, "%3d", timeout);
			mvwprintw(infoWindow, 5, 13, "ms MNIEJ == SZYBCIEJ");
    		mvwprintw(infoWindow, 6, 10, "%d", points);
    		wattron(infoWindow, COLOR_PAIR(3));
		}
		
		// Odswiezenie tabelki informacyjnej
		int refreshInfo() {
			wrefresh(infoWindow);
		}
};

// Definicja oraz odstepy kierunkow
enum Direction {
	up = -1, down = 1, left= -2, right = 2
};

// Segmenty gracza
class PlayerSeg : public Create {
	public:
		// Konstruktor segmentow z ktorych sklada sie gracz
		PlayerSeg(int y, int x) : Create(y, x, 'o') {}
};

// Kontroler gracza
class Player {
	private:
		std::queue<PlayerSeg> segmentQueue; // Zainicjowanie kolejki segmentow z ktorych bedzie skladal sie waz, elementy beda zabierane z tylu kolejki i przekazywane z przodu nowej pozycji
		Direction direction;
	
	public:
		// Podstawowy/poczatkowy kierunek weza
		Player() {
			direction = right;
		}
	
		// Dodanie segmentu weza do kolejki
		int addSegment(PlayerSeg val) {
			segmentQueue.push(val);
		}
	
		// Zabranie oczekujacego segmentu weza z kolejki
		int removeSegment() {
			segmentQueue.pop();
		}
	
		// Tyl gracza
		PlayerSeg bottom() {
			return segmentQueue.front();
		}
	
		// Przod gracza
		PlayerSeg top() {
			return segmentQueue.back();
		}
	
		// Zwrocenie kierunku
		Direction getDirection() {
			return direction;
		}
	
		// Ustawienie kierunku
		int setDirection(Direction val) {
			int var = direction + val;
			if (var != 0) {
				direction = val;
			}
		}
	
		// Czesc gorna weza, odpowiada za gorna czesc kolejki segmentow
		PlayerSeg newTop() {
			int row = top().getY(); // Zczytanie gornej pozycji weza Y
			int col = top().getX(); // Zczytanie gornej pozycji weza X
			// Wartosci poruszania sie w zaleznosci od kierunku
			switch (direction) {
				case up:
					row = row - 1;
					break;
				case down:
					row = row + 1;
					break;
				case left:
					col = col - 1;
					break;
				case right:
					col = col + 1;
					break;
			}
			return PlayerSeg(row, col); // Zwrocenie pozycji do PlayerSeg
		}
};

// Kontroler gry
class Game {
	private:
		bool isGameOver = false;
		int points = 0;
		int timeout = 150;
		Food *food = 0;
		GameWindow gameWindow;
		Info infoWindow;
		Player player;
	public:
		// Konstruktor gry z inicjatorami
		Game(int height, int width) {
			gameWindow = GameWindow(height, width);
			int y = gameWindow.getRow() + height / 2.75; // Dostoswanie wysokosci do okna dla okna informacyjnego
			int x = gameWindow.getCol() + width; // Dostoswanie szerokosci do okna dla okna informacyjnego
			infoWindow = Info(width, y, x);
			initGame(); // Inicjlaizacja elementow gry
		}
		
		// Funkcja tworzenia jedzenia
		int makeFood() {
			if (food == 0) {
				int x = 0;
				int y = 0;
				gameWindow.random(y, x); // Wydobycie losowych koordynatow
				food = new Food(y, x); // Stworzenie nowego obiektu na podstawie koordynatow
				gameWindow.addObject(*food); // Dodanie nowego obiektu na ekran gry
			}
		}
		
		// Inicjalizacja elementow gry
		int initGame() {
			infoWindow.initInfo(points, timeout); // Inicjalizacja okna informacyjnego
			gameWindow.initBorder(); // Inicjalizacja okna/granicy gry
			// Poczatkowy szkielet gracza
			PlayerSeg segment = PlayerSeg(10, 10); // Poczatkowa pozycja gracza
			gameWindow.addObject(segment);
			player.addSegment(segment);
			// Pierwsze poczatkowe ustawienie jedzenia w oknie gry
			makeFood();
		}
		
		// Restart gry
		int restartGame() {
			isGameOver = false;
        	delete food; // Usuniecie starego przedmiotu
        	food = 0;
        	points = 0; // Wyzerowanie punktow
        	refreshGame(); // Odswiezenie gry
        	player = Player(); // Zresetowanie gracza
        	initGame(); // Ponowna inicjalizacja gry
        	timeout = 150; // Przywrocenie pierwotnego czasu gry
        	gameWindow.setTimeout(timeout);
        	infoWindow.updateInfo(points, timeout);
    	}
    	
		// Uzyskanie wejscia gracza
		int input() {
			char input = gameWindow.getInput();
			switch (input) {
				case 'w':
					player.setDirection(up);
					break;
				case 's':
					player.setDirection(down);
					break;
				case 'a':
					player.setDirection(left);
					break;
				case 'd':
					player.setDirection(right);
					break;
				// Przycisk do restartu gry
				case 'r':
					gameWindow.setTimeout(150);
					restartGame();
					break;
				// Przycisk do pauzy gry (ENTER)
				case 10:
					gameWindow.setTimeout(-1);
					while (gameWindow.getInput() != 10);
					gameWindow.setTimeout(timeout);
					break;
				// Przycisk do wyjscia z gry (ESC)
				case 27:
					isGameOver = true;
					break;
			}
		}

		// Aktualizacja stanu gry
		int update() {	
			PlayerSeg segment = player.newTop(); // Segment gracza
			int spaceX = 0;
			int spaceY = 0;
			// Zachowanie gracza wzgledem elementow na polu gry
			if (food != 0) {
				switch (gameWindow.getPos(segment.getY(), segment.getX())) {
					// Zachowanie po wejsciu na jedznie
					case '*':
						delete food; // Usuniecie starego przedmiotu
						food = 0;
						points = points + 10;
						// Przyspieszanie gry (minimum 3ms)
						if (timeout - 3 >= 3) {
							timeout = timeout - 3;
						}
						gameWindow.setTimeout(timeout);
						infoWindow.updateInfo(points, timeout);
						break;
					// Zmiana kolejki segmentow, usuniecie z tylu weza segmentu i dodanie pustego znaku
					case ' ':
						spaceX = player.bottom().getY(); // Zczytanie pozycji tylu gracza Y
						spaceY = player.bottom().getX(); // Zczytanie pozycji tylu gracza X
						gameWindow.addObject(Space(spaceX, spaceY)); // Dodanie pustego pola
						player.removeSegment(); // Usuniecie segmentu weza
						break;
					// Koniec gry oraz ewentualny restart
					default:
						gameWindow.setTimeout(-1);
						if (gameWindow.getInput() == 'r') {
							gameWindow.setTimeout(150);
                    		restartGame();
                		} else {
                    		isGameOver = true;
                		}
                		break;
				}
			}
			gameWindow.addObject(segment); // Dodanie nowego segmentu weza do obrazu gry
			player.addSegment(segment); // Dodanie nowego segmentu weza do kontrolera gracza, w tym dodanie do kolejki segmentow
			makeFood(); // Stworzenie nowego jedzenia
		}

		// Odswiezenie stanu gry
		int refreshGame() {
			gameWindow.refreshGameWindow();
			infoWindow.refreshInfo();
		}

		// Zakonczenie stanu gry
		bool endGame() {
			return isGameOver;
		}
};

// Uruchomienie gry
int main(int argc, char** argv) {
    initscr(); // Zainicjowanie obrazu
    curs_set(0); // Wylaczenie podkreslnika w terminalu
    noecho(); // Nie printuja sie znaki wcisnkane na klawiaturze
    start_color(); // Wlaczenie kolorow

	// Kolory
    init_pair(1, COLOR_GREEN, COLOR_BLACK); // Kolory dla okna/granicy gry
    init_pair(2, COLOR_BLUE, COLOR_BLACK); // Kolory dla okna informacyjnego 1
    init_pair(3, COLOR_CYAN, COLOR_BLACK); // Kolory dla okna informacyjnego 2

    Game game(25, 50); // Inicjlaizacja gry (wartosci oznaczaja wielkosc okna gry)
	// Petla gry
	while (game.endGame() != true) {
		game.input();
		game.update(); // Odswiezanie gry
		game.refreshGame(); // Zaladowanie/aktualizacja elementow gry
	}
}
