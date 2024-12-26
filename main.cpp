/*
 *
 * Solution to course project 7
 * Introduction to programming course
 * Faculty of Mathematics and Informatics of Sofia University
 * Winter semester 2024/2025
 *
 * @author Yoan Dobtchev
 * @idnumber 2MI0600552
 * @compiler GCC
 *
 * main game file
 *
 */

#include <iostream>
#include <fstream>

void clearTerminal() {
#ifdef _WIN32
	system("cls");
#else
	system("clear");
#endif
}

enum Squares {
	EMPTY,
	ATTACKER,
	DEFENDER,
	KING,
};

enum Players {
	PLAYER1,
	PLAYER2,
};

const size_t TOTAL_SQUARES = 4;
char squareChars[TOTAL_SQUARES];

int** board;
int boardSize = 0;

bool loadSkin() {
	std::ifstream file("./pieceSkins/minimalistic.vch");

	if (!file.is_open()) {
		std::cerr << "Error: Couldn't open skin" << std::endl;
		return false;
	}

	char c;
	int index = 0;

	while (file.get(c) && index < TOTAL_SQUARES) {
		squareChars[index] = c;
		index++;
	}

	file.close();

	if (index != TOTAL_SQUARES) {
		std::cerr << "Error: The number of characters does not match the number of pieces" << std::endl;
		return false;
	}

	return true;
}

bool loadTable() {
	std::ifstream file("./startingTables/9x9.vch");

	if (!file.is_open()) {
		std::cerr << "Error: Couldn't open table file" << std::endl;
		return false;
	}

	const int MAX_BOARD_SIZE = 256;
	char* line = new char[MAX_BOARD_SIZE];

	if (!file.getline(line, MAX_BOARD_SIZE)) {
		std::cerr << "Error: File is empty" << std::endl;
		delete[] line;
		return false;
	}

	size_t len = strlen(line);
	for (size_t i = 0; i < len; ++i) {
		if (line[i] != ' ') {
			++boardSize;
		}
	}

	board = new int*[boardSize];
	for (int i = 0; i < boardSize; ++i) {
		board[i] = new int[boardSize];
	}

	file.clear();
	file.seekg(0);

	int row = 0;
	while (file.getline(line, MAX_BOARD_SIZE) && row < boardSize) {
		int col = 0;
		for (size_t i = 0; i < strlen(line); ++i) {
			if (line[i] == ' ') continue;

			if (line[i] == '*')
				board[row][col] = EMPTY;
			else if (line[i] == 'A')
				board[row][col] = ATTACKER;
			else if (line[i] == 'D')
				board[row][col] = DEFENDER;
			else if (line[i] == 'K')
				board[row][col] = KING;
			else {
				std::cerr << "Error: Unrecognized character '" << line[i] << "' at row " << row << ", col " << col << std::endl;
				delete[] line;

				for (int j = 0; j <= row; ++j) {
					delete[] board[j];
				}
				delete[] board;
				return false;
			}
			col++;
		}
		if (col != boardSize) {
			std::cerr << "Error: Inconsistent number of columns at row " << row << std::endl;
			delete[] line;
			for (int j = 0; j <= row; ++j) {
				delete[] board[j];
			}
			delete[] board;
			return false;
		}
		row++;
	}

	delete[] line;

	if (row != boardSize) {
		std::cerr << "Error: Inconsistent number of rows. Expected " << boardSize << ", got " << row << std::endl;
		for (int i = 0; i < row; ++i) {
			delete[] board[i];
		}
		delete[] board;
		return false;
	}

	return true;
}

void printTable() {
	clearTerminal();

	int rowHelper = 1;
	char columnHelper = 'a';

	for (int i = 0; i < boardSize; ++i) {
		std::cout << columnHelper++ << ' ';
	}
	std::cout << std::endl;

	for (int i = 0; i < boardSize; ++i) {
		std::cout << "_ ";
	}
	std::cout << std::endl;

	for (int i = 0; i < boardSize; ++i) {
		for (int j = 0; j < boardSize; ++j) {
			std::cout << squareChars[board[i][j]];
			if (j != boardSize - 1) std::cout << ' ';
		}
		std::cout << "| " << rowHelper++;
		std::cout << std::endl;
	}
}

int multipleChoice(const char* choices[], int numChoices) {
	for (int i = 0; i < numChoices; ++i) {
		std::cout << "[" << i + 1 << "]: " << choices[i] << std::endl;
	}

	int choice = 0;
	do {
		std::cout << "Choice: ";
		std::cin >> choice;
	} while (choice <= 0 || choice > numChoices);

	std::cin.ignore();
	return choice;
}

bool isValidCommand(char* command, char* error) {
	if (command == nullptr) return false;
	// std::cout << "len " << strlen(command) << std::endl;
	char inputCommand[5];

	while (*command && *command != ' ') {
		command++;
	}

	if (strlen(command) < 4) {
		strcpy(error, "NIGA");

		return false;
	}

	// return true;
}

void playerMove(bool player) {
	char command[1024];
	char error[1024] = "";
	do {
		printTable();
		if (strlen(error) > 0) {
			std::cerr << error << std::endl;
			error[0] = '\0';
		}
		std::cout << ((player == PLAYER1) ? "[ATTACKER]: " : "[DEFENDER]: ");
		std::cin.getline(command, 1024);
	} while (!isValidCommand(command, error));
}

void startGame() {
	bool gameEnded = false;
	bool currentTurn = PLAYER1;

	while (!gameEnded) {
		playerMove(currentTurn);

		currentTurn = currentTurn == PLAYER1 ? PLAYER2 : PLAYER1;
	}
}

int main() {

	if (!loadSkin()) return -1;

	if (!loadTable()) return -1;

	printTable();

	// std::cout << "Main menu:"
	//           << "[1] "
	startGame();

	const char* choices[] = {"New game", "Quit"};
	std::cout << "Main menu" << std::endl;
	int choice = multipleChoice(choices, 2);
	switch (choice) {
	case 1:
		startGame();
		break;
	case 2:
		return 0;
		break;
	}

	return 0;
}