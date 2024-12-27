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

int charToInt(char c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	return -1;
}

enum Squares {
	EMPTY,
	ATTACKER,
	DEFENDER,
	KING,
	KING_GOAL,
};

enum Players {
	PLAYER1,
	PLAYER2,
};

const size_t TOTAL_SQUARES = 5;
char squareChars[TOTAL_SQUARES];

int** board;
int boardSize = 0;

bool loadSkin() {
	std::ifstream file("./pieceSkins/default.vch");

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

int kingStartingRow = -1, kingStartingCol = -1;

bool loadTable() {
	std::ifstream file("./startingTables/11x11.vch");

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
			else if (line[i] == 'K') {
				board[row][col] = KING;
				kingStartingRow = row;
				kingStartingCol = col;
			} else if (line[i] == 'X')
				board[row][col] = KING_GOAL;
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
	// clearTerminal();

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

enum COMMAND_TYPES {
	MOVE,
	BACK,
	INFO,
	HELP,
};

int getCommandType(char* command) {
	char commandTypeString[1024];
	int i = 0;
	while (command[i] != ' ' && command[i] != '\0') {
		commandTypeString[i] = command[i];
		i++;
	}
	commandTypeString[i] = '\0';

	if (strcmp(commandTypeString, "move") == 0) {
		return MOVE;
	} else if (strcmp(commandTypeString, "back") == 0) {
		return BACK;
	} else if (strcmp(commandTypeString, "info") == 0) {
		return INFO;
	} else if (strcmp(commandTypeString, "help") == 0) {
		return HELP;
	} else {
		return -1;
	}
}

bool getNextMoveCommandCoordinates(int& row, int& col, char* command, char* error, int& i) {
	while (command[i] != '\0' && command[i] == ' ') {
		i++;
	}

	if (command[i] == '\0') {
		stpcpy(error, "Not enough parameters. Use 'move <from> <to>' format");
		return false;
	}

	col = command[i] - 'a';

	i++;

	row = 0;
	while (command[i] != '\0' && command[i] != ' ') {
		if (command[i] < '0' || command[i] > '9') {
			stpcpy(error, "Number missing from position. Each position must end with a number from the table");
			return false;
		}

		row = row * 10 + charToInt(command[i]);
		i++;
	}
	row--;

	return true;
}

bool validatePositionBounds(int fromRow, int fromCol, int toRow, int toColumn, char* command, char* error) {
	if (fromRow < 0 || fromRow >= boardSize || fromCol < 0 || fromCol >= boardSize) {
		strcpy(error, "Invalid 'from' position. The position must be within the board");
		return false;
	}

	if (toRow < 0 || toRow >= boardSize || toColumn < 0 || toColumn >= boardSize) {
		strcpy(error, "Invalid 'to' position. The position must be within the board");
		return false;
	}

	return true;
}

bool validateIfPieceCanMoveThrough(int fromRow, int fromCol, int toRow, int toCol, char* error) {
	enum MOVEMENT {
		VERTICAL,
		HORIZONTAL,
	};

	int movement = fromRow == toRow ? HORIZONTAL : VERTICAL;
	int from = movement == HORIZONTAL ? fromCol : fromRow;
	int to = movement == HORIZONTAL ? toCol : toRow;
	if (from < to) {
		for (int i = from + 1; i < to; ++i) {
			if (movement == HORIZONTAL) {
				if (board[fromRow][i] != EMPTY) {
					strcpy(error, "Invalid move. There is a piece in the way");
					return false;
				}
			} else if (movement == VERTICAL) {
				if (board[i][fromCol] != EMPTY) {
					strcpy(error, "Invalid move. There is a piece in the way");
					return false;
				}
			}
		}
	} else {
		for (int i = from - 1; i > to; --i) {
			if (movement == HORIZONTAL) {
				if (board[fromRow][i] != EMPTY) {
					strcpy(error, "Invalid move. There is a piece in the way");
					return false;
				}
			} else if (movement == VERTICAL) {
				if (board[i][fromCol] != EMPTY) {
					strcpy(error, "Invalid move. There is a piece in the way");
					return false;
				}
			}
		}
	}

	return true;
}

bool validateIfPieceCanMoveTo(int fromRow, int fromCol, int toRow, int toCol, bool player, char* error) {
	int pieceOnFrom = board[fromRow][fromCol];

	if (fromRow == toRow && fromCol == toCol) {
		strcpy(error, "Invalid move. The 'from' and 'to' positions cannot be the same");
		return false;
	}

	if (pieceOnFrom == EMPTY || pieceOnFrom == KING_GOAL) {
		strcpy(error, "Invalid move. The 'from' position is empty");
		return false;
	}

	if (player == PLAYER1 && pieceOnFrom != ATTACKER) {
		strcpy(error, "Invalid move. Only the defender can move defenders");
		return false;
	}

	if (player == PLAYER2 && pieceOnFrom != DEFENDER) {
		strcpy(error, "Invalid move. Only the attacker can move attackers");
		return false;
	}

	if (toRow == kingStartingRow && toCol == kingStartingCol && pieceOnFrom != KING) {
		strcpy(error, "Invalid move. Only the king can move to the king's starting position");
		return false;
	}

	if (board[toRow][toCol] == KING_GOAL && pieceOnFrom != KING) {
		strcpy(error, "Invalid move. Only the king can move to the king's goal position");
		return false;
	}

	if (board[toRow][toCol] != EMPTY) {
		strcpy(error, "Invalid move. The destination position is not empty");
		return false;
	}

	return true;
}

bool validateMoveCommand(char* command, char* error, bool player) {
	int fromRow, fromCol, toRow, toCol;
	int i = 4;
	if (!getNextMoveCommandCoordinates(fromRow, fromCol, command, error, i)) return false;
	if (!getNextMoveCommandCoordinates(toRow, toCol, command, error, i)) return false;

	std::cout << "fromRow: " << fromRow << " fromCol: " << fromCol << " toRow: " << toRow << " toCol: " << toCol << std::endl;

	if (!validatePositionBounds(fromRow, fromCol, toRow, toCol, command, error)) return false;

	if (fromRow != toRow && fromCol != toCol) {
		strcpy(error, "Invalid move. The move must be either vertical or horizontal");
		return false;
	}

	if (!validateIfPieceCanMoveThrough(fromRow, fromCol, toRow, toCol, error)) return false;

	if (!validateIfPieceCanMoveTo(fromRow, fromCol, toRow, toCol, player, error)) return false;

	return true;
}

bool isValidCommand(char* command, char* error, bool player) {
	if (command == nullptr) return false;

	if (strlen(command) < 4) {
		strcpy(error, "Command must be at least 4 characters long");
		return false;
	}

	int inputCommandType = getCommandType(command);

	if (inputCommandType == -1) {
		strcpy(error, "Input command not recognized. Use 'move <from> <to>', 'back <num>', 'info' or 'help'");
		return false;
	}

	switch (inputCommandType) {
	case MOVE:
		return validateMoveCommand(command, error, player);
		break;
	case BACK:
		// return validateBackCommand(command, error);
		break;
		// case INFO:
		// 	return true;
		// 	break;
		// case HELP:
		// 	return true;
		// 	break;
	}

	return true;
}

void playerMove(bool player) {
	char command[1024];
	char error[1024] = "", infoMessage[1024] = "";

	do {
		printTable();
		if (strlen(error) > 0) {
			std::cerr << error << std::endl;
			error[0] = '\0';
		}
		std::cout << ((player == PLAYER1) ? "[ATTACKER]: " : "[DEFENDER]: ");
		std::cin.getline(command, 1024);
	} while (!isValidCommand(command, error, player));
}

void startGame() {
	bool gameEnded = false;
	bool currentTurn = PLAYER1;

	while (!gameEnded) {
		playerMove(currentTurn);

		currentTurn = !currentTurn;
	}
}

int main() {

	if (!loadSkin()) return -1;

	if (!loadTable()) return -1;

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