/**
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

void swapChars(char& a, char& b) {
	char temp = a;
	a = b;
	b = temp;
}

enum PositionTypes {
	PIECE,
	SQUARE,
};

enum Pieces {
	EMPTY,
	ATTACKER,
	DEFENDER,
	KING,
};

enum Squares {
	NORMAL,
	KING_GOAL,
	KING_THRONE,
};

enum Players {
	PLAYER1,
	PLAYER2,
};

const size_t TOTAL_SQUARES = 5;
char squareChars[TOTAL_SQUARES];

int*** board;
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

void freeBoard() {
	if (!board) return;
	for (int i = 0; i < boardSize; ++i) {
		for (int j = 0; j < boardSize; ++j) {
			delete[] board[i][j];
		}
		delete[] board[i];
	}
	delete[] board;
	board = nullptr;
}

int totalAttackers, totalDefenders, totalKings;
bool loadTable() {
	std::ifstream file("./startingTables/test.vch");

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

	board = new int**[boardSize];
	for (int i = 0; i < boardSize; ++i) {
		board[i] = new int*[boardSize];
		for (int j = 0; j < boardSize; ++j) {
			// As the king can be on the throne or his goal, the position should
			// have infomation about the type of piece and type of square
			// 0 - piece, 1 - square
			board[i][j] = new int[2];
		}
	}

	file.clear();
	file.seekg(0);

	totalAttackers = 0;
	totalDefenders = 0;
	totalKings = 0;
	int row = 0;
	while (file.getline(line, MAX_BOARD_SIZE) && row < boardSize) {
		int col = 0;
		for (size_t i = 0; i < strlen(line); ++i) {
			if (line[i] == ' ') continue;

			board[row][col][SQUARE] = NORMAL;

			if (line[i] == '*')
				board[row][col][PIECE] = EMPTY;
			else if (line[i] == 'A') {
				board[row][col][PIECE] = ATTACKER;
				totalAttackers++;
			} else if (line[i] == 'D') {
				board[row][col][PIECE] = DEFENDER;
				totalDefenders++;
			} else if (line[i] == 'K') {
				board[row][col][PIECE] = KING;
				board[row][col][SQUARE] = KING_THRONE;
				totalKings++;
			} else if (line[i] == 'X') {
				board[row][col][PIECE] = EMPTY;
				board[row][col][SQUARE] = KING_GOAL;
			} else {
				std::cerr << "Error: Unrecognized character '" << line[i] << "' at row " << row << ", col " << col << std::endl;
				delete[] line;
				freeBoard();
				return false;
			}
			col++;
		}

		if (col != boardSize) {
			std::cerr << "Error: Inconsistent number of columns at row " << row << std::endl;
			delete[] line;
			freeBoard();
			return false;
		}
		row++;
	}

	delete[] line;

	if (row != boardSize) {
		std::cerr << "Error: Inconsistent number of rows. Expected " << boardSize << ", got " << row << std::endl;
		freeBoard();
		return false;
	}

	if (totalKings != 1) {
		std::cerr << "Error: Expected 1 king, got " << totalKings << std::endl;
		freeBoard();
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
			if (board[i][j][PIECE] == EMPTY && (board[i][j][SQUARE] == KING_GOAL || board[i][j][SQUARE] == KING_THRONE)) {
				const int INDEX_FOR_THRONE_AND_GOAL = 4;
				std::cout << squareChars[INDEX_FOR_THRONE_AND_GOAL];
			} else {
				std::cout << squareChars[board[i][j][PIECE]];
			}
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

enum CommandTypes {
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
		strcpy(error, "Not enough parameters. Use 'move <from> <to>' format");
		return false;
	}

	col = command[i] - 'a';

	i++;

	row = 0;
	while (command[i] != '\0' && command[i] != ' ') {
		if (command[i] < '0' || command[i] > '9') {
			strcpy(error, "Number missing from position. Each position must end with a number from the table");
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
	enum Movement {
		VERTICAL,
		HORIZONTAL,
	};

	bool movement = fromRow == toRow ? HORIZONTAL : VERTICAL;
	int from = movement == HORIZONTAL ? fromCol : fromRow;
	int to = movement == HORIZONTAL ? toCol : toRow;
	if (from < to) {
		for (int i = from + 1; i < to; ++i) {
			if (movement == HORIZONTAL) {
				if (board[fromRow][i][PIECE] != EMPTY) {
					strcpy(error, "Invalid move. There is a piece in the way");
					return false;
				}
			} else if (movement == VERTICAL) {
				if (board[i][fromCol][PIECE] != EMPTY) {
					strcpy(error, "Invalid move. There is a piece in the way");
					return false;
				}
			}
		}
	} else {
		for (int i = from - 1; i > to; --i) {
			if (movement == HORIZONTAL) {
				if (board[fromRow][i][PIECE] != EMPTY) {
					strcpy(error, "Invalid move. There is a piece in the way");
					return false;
				}
			} else if (movement == VERTICAL) {
				if (board[i][fromCol][PIECE] != EMPTY) {
					strcpy(error, "Invalid move. There is a piece in the way");
					return false;
				}
			}
		}
	}

	return true;
}

bool validateIfPieceCanMoveTo(int fromRow, int fromCol, int toRow, int toCol, bool player, char* error) {
	int pieceOnFrom = board[fromRow][fromCol][PIECE];

	if (fromRow == toRow && fromCol == toCol) {
		strcpy(error, "Invalid move. The 'from' and 'to' positions cannot be the same");
		return false;
	}

	if (pieceOnFrom == EMPTY) {
		strcpy(error, "Invalid move. The 'from' position is empty");
		return false;
	}

	if (player == PLAYER1 && pieceOnFrom != ATTACKER) {
		strcpy(error, "Invalid move. Only the defender can move defenders and the king");
		return false;
	}

	if (player == PLAYER2 && pieceOnFrom != DEFENDER && pieceOnFrom != KING) {
		strcpy(error, "Invalid move. Only the attacker can move attackers");
		return false;
	}

	if (board[toRow][toCol][SQUARE] == KING_THRONE && pieceOnFrom != KING) {
		strcpy(error, "Invalid move. Only the king can move to the king's throne");
		return false;
	}

	if (board[toRow][toCol][SQUARE] == KING_GOAL && pieceOnFrom != KING) {
		strcpy(error, "Invalid move. Only the king can move to the king's goal positions");
		return false;
	}

	if (board[toRow][toCol][PIECE] != EMPTY) {
		strcpy(error, "Invalid move. The destination position is not empty");
		return false;
	}

	return true;
}

bool canOpenMoveFile(char* error) {
	std::ifstream movesFile("moves.vch");
	if (!movesFile.is_open()) {
		std::cerr << "Error: Couldn't open moves.vch file" << std::endl;
		return false;
	}
	movesFile.close();
	return true;
}

bool validateMoveCommand(char* command, char* error, bool player) {
	if (!canOpenMoveFile(error)) return false;

	int fromRow, fromCol, toRow, toCol;
	int i = 4;
	if (!getNextMoveCommandCoordinates(fromRow, fromCol, command, error, i)) return false;
	if (!getNextMoveCommandCoordinates(toRow, toCol, command, error, i)) return false;

	if (!validatePositionBounds(fromRow, fromCol, toRow, toCol, command, error)) return false;

	if (fromRow != toRow && fromCol != toCol) {
		strcpy(error, "Invalid move. The move must be either vertical or horizontal");
		return false;
	}

	if (!validateIfPieceCanMoveTo(fromRow, fromCol, toRow, toCol, player, error)) return false;

	if (!validateIfPieceCanMoveThrough(fromRow, fromCol, toRow, toCol, error)) return false;

	return true;
}

bool getBackCommandParameter(char* command, char* error, int& num) {
	int i = 4;
	while (command[i] != '\0' && command[i] == ' ') {
		i++;
	}

	if (command[i] == '\0') return true;

	num = 0;
	while (command[i] != '\0') {
		if (command[i] < '0' || command[i] > '9') {
			strcpy(error, "Invalid parameter. The parameter must be a number");
			return false;
		}
		num = num * 10 + charToInt(command[i]);
		i++;
	}

	return true;
}

bool validateBackCommand(char* command, char* error) {
	if (!canOpenMoveFile(error)) return false;

	std::ifstream movesFile("moves.vch");

	int stepsBack = 1;

	if (!getBackCommandParameter(command, error, stepsBack)) return false;

	int recordedMovesCount = 0;
	const int MAX_MOVE_LENGTH = 16;
	char buffer[MAX_MOVE_LENGTH];

	while (movesFile.getline(buffer, MAX_MOVE_LENGTH)) {
		++recordedMovesCount;
	}

	if (stepsBack > recordedMovesCount) {
		strcpy(error, "Invalid parameter. The parameter must be less than the number of moves");
		return false;
	}

	movesFile.close();

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
		return validateBackCommand(command, error);
		break;
		// info and help are always valid
	}

	return true;
}

bool canCaptureKing(int row, int col) {
	const int directions[4][2] = {
	    {0, -1}, // Left
	    {0, 1},  // Right
	    {1, 0},  // Up
	    {-1, 0}  // Down
	};

	for (int i = 0; i < 4; ++i) {
		int dRow = directions[i][0], dCol = directions[i][1];

		bool isEdge = row + dRow < 0 || row + dRow >= boardSize || col + dCol < 0 || col + dCol >= boardSize;
		if (isEdge) continue;

		bool isAttacker = board[row + dRow][col + dCol][PIECE] == ATTACKER;
		bool isKingGoal = board[row + dRow][col + dCol][SQUARE] == KING_GOAL;
		bool isKingThrone = board[row + dRow][col + dCol][SQUARE] == KING_THRONE;

		if (!(isAttacker || isKingGoal || isKingThrone)) return false;
	}

	return true;
}

bool canCapture(int row, int col, int dRow, int dCol) {
	int neighbourRow = row + dRow;
	int neighbourCol = col + dCol;
	int twoTileAwayRow = row + 2 * dRow;
	int twoTileAwayCol = col + 2 * dCol;

	if (neighbourRow < 0 || neighbourRow >= boardSize || neighbourCol < 0 || neighbourCol >= boardSize) return false;

	if (board[neighbourRow][neighbourCol][PIECE] == EMPTY) return false;

	if (board[neighbourRow][neighbourCol][PIECE] == KING) return canCaptureKing(neighbourRow, neighbourCol);

	if (twoTileAwayRow < 0 || twoTileAwayRow >= boardSize || twoTileAwayCol < 0 || twoTileAwayCol >= boardSize) return false;

	int* twoTileAwayPosition = board[twoTileAwayRow][twoTileAwayCol];
	int currentPiece = board[row][col][PIECE];

	bool samePiece = (twoTileAwayPosition[PIECE] == currentPiece);
	bool kingThrone = (twoTileAwayPosition[SQUARE] == KING_THRONE);
	bool kingGoal = (twoTileAwayPosition[SQUARE] == KING_GOAL);

	return samePiece || kingThrone || kingGoal;
}

void appendCaptureInfoToMessage(char* infoMessage, int row, int col) {
	char colChar[2] = {(char)(col + 'a'), '\0'};
	char rowChar[10];

	int index = 0;
	int rowNumber = row + 1;
	while (rowNumber > 0) {
		rowChar[index++] = (char)((rowNumber % 10) + '0');
		rowNumber /= 10;
	}
	rowChar[index] = '\0';

	for (int i = 0; i < index / 2; ++i) {
		swapChars(rowChar[i], rowChar[index - i - 1]);
	}

	strcat(infoMessage, colChar);
	strcat(infoMessage, rowChar);
	strcat(infoMessage, " ");
}

void executeMoveCommand(char* command, char* infoMessage, bool& player) {
	int fromRow, fromCol, toRow, toCol;
	int i = 4;
	getNextMoveCommandCoordinates(fromRow, fromCol, command, infoMessage, i);
	getNextMoveCommandCoordinates(toRow, toCol, command, infoMessage, i);

	board[toRow][toCol][PIECE] = board[fromRow][fromCol][PIECE];
	board[fromRow][fromCol][PIECE] = EMPTY;

	const int directions[4][2] = {
	    {0, -1}, // Left
	    {0, 1},  // Right
	    {1, 0},  // Up
	    {-1, 0}  // Down
	};

	std::ofstream movesFile("moves.vch", std::ios::app);

	movesFile << (char)('a' + fromCol) << (fromRow + 1) << (char)('a' + toCol) << (toRow + 1);

	bool atLeastOneCapture = false;
	for (int i = 0; i < 4; ++i) {
		int dRow = directions[i][0], dCol = directions[i][1];
		int captureRow = toRow + dRow;
		int captureCol = toCol + dCol;
		if (canCapture(toRow, toCol, dRow, dCol)) {
			if (!atLeastOneCapture) {
				strcat(infoMessage, (player == PLAYER1) ? "Attacker captured: " : "Defender captured: ");

				movesFile << 'x';
				atLeastOneCapture = true;
			}

			appendCaptureInfoToMessage(infoMessage, captureRow, captureCol);
			movesFile << (char)('a' + captureCol) << (captureRow + 1);

			board[toRow + dRow][toCol + dCol][PIECE] = EMPTY;
		}
	}

	movesFile << std::endl;
	movesFile.close();

	player = !player;
}

void getNextMoveFileCoordinates(int& row, int& col, char* move, int& i) {
	col = move[i] - 'a';

	i++;

	row = 0;
	while (move[i] != '\0' && move[i] >= '0' && move[i] <= '9') {
		row = row * 10 + charToInt(move[i]);
		i++;
	}
	row--;
}

void reverseMove(char* move, bool& player, char* infoMessage) {
	int i = 0;
	int fromRow, fromCol, toRow, toCol;
	getNextMoveFileCoordinates(fromRow, fromCol, move, i);
	getNextMoveFileCoordinates(toRow, toCol, move, i);

	board[fromRow][fromCol][PIECE] = board[toRow][toCol][PIECE];
	board[toRow][toCol][PIECE] = EMPTY;

	if (move[i] != 'x') {
		player = !player;
		return;
	}

	i++;

	while (move[i] != '\0') {
		int capturedCol, capturedRow;
		getNextMoveFileCoordinates(capturedRow, capturedCol, move, i);

		board[capturedRow][capturedCol][PIECE] = (player ? DEFENDER : ATTACKER);
	}

	player = !player;
}

void executeBackCommand(char* command, char* infoMessage, bool& player) {
	std::ifstream movesFile("moves.vch");

	int stepsBack = 1;
	getBackCommandParameter(command, infoMessage, stepsBack);

	// From, to moves - max 6 characters
	// x - 1 character
	// Max captures - 3 x 3 characters each
	const int MAX_MOVE_LENGTH = 16;
	char moves[300][MAX_MOVE_LENGTH];
	int moveCount = 0;
	char move[MAX_MOVE_LENGTH];
	while (movesFile.getline(move, MAX_MOVE_LENGTH)) {
		strcpy(moves[moveCount++], move);
	}

	movesFile.close();

	for (int i = 1; i <= stepsBack; ++i) {
		reverseMove(moves[moveCount - i], player, infoMessage);
	}

	std::ofstream newMovesFile("moves.vch", std::ios::trunc);

	for (int i = 0; i < moveCount - stepsBack; ++i) {
		newMovesFile << moves[i] << std::endl;
	}

	newMovesFile.close();
}

void appendMoveCountInfo(char* infoMessage) {
	std::ifstream movesFile("moves.vch");

	const int MAX_MOVE_LENGTH = 16;
	char move[MAX_MOVE_LENGTH];
	int moveCount = 0;
	while (movesFile.getline(move, MAX_MOVE_LENGTH)) {
		moveCount++;
	}

	movesFile.close();

	std::cout << "Number of moves: " << moveCount << std::endl;

	strcat(infoMessage, "Number of moves: ");
	char moveCountChar[10];
	int index = 0;
	while (moveCount > 0) {
		moveCountChar[index++] = (char)((moveCount % 10) + '0');
		moveCount /= 10;
	}
	moveCountChar[index] = '\0';

	for (int i = 0; i < index / 2; ++i) {
		std::swap(moveCountChar[i], moveCountChar[index - i - 1]);
	}

	strcat(infoMessage, moveCountChar);
}

void appendPlayerTurnInfo(char* infoMessage, bool player) {
	strcat(infoMessage, (player == PLAYER1) ? "Attacker's turn" : "Defender's turn");
}

void getPiecesCount(int& attackerCount, int& defenderCount, int& kingCount) {
	attackerCount = 0;
	defenderCount = 0;
	kingCount = 0;

	for (int i = 0; i < boardSize; ++i) {
		for (int j = 0; j < boardSize; ++j) {
			if (board[i][j][PIECE] == ATTACKER) {
				attackerCount++;
			} else if (board[i][j][PIECE] == DEFENDER) {
				defenderCount++;
			} else if (board[i][j][PIECE] == KING) {
				kingCount++;
			}
		}
	}
}

void intToStr(int num, char* str) {
	int i = 0;
	char temp[20];
	if (num == 0) {
		str[i++] = '0';
	} else {
		while (num > 0) {
			temp[i++] = (num % 10) + '0';
			num /= 10;
		}
	}
	temp[i] = '\0';

	int j;
	for (j = 0; j < i; j++) {
		str[j] = temp[i - j - 1];
	}
	str[j] = '\0';
}

void appendLeftPiecesInfo(char* infoMessage) {
	strcat(infoMessage, "Pieces left: ");
	int attackerCount, defenderCount, kingCount;
	getPiecesCount(attackerCount, defenderCount, kingCount);

	strcat(infoMessage, "Attackers: ");
	char numStr[30];
	intToStr(attackerCount, numStr);
	strcat(infoMessage, numStr);
	strcat(infoMessage, "/");
	intToStr(totalAttackers, numStr);
	strcat(infoMessage, numStr);

	strcat(infoMessage, ", Defenders: ");
	intToStr(defenderCount, numStr);
	strcat(infoMessage, numStr);
	strcat(infoMessage, "/");
	intToStr(totalDefenders, numStr);
	strcat(infoMessage, numStr);
}

void executeInfoCommand(char* infoMessage, bool player) {
	strcat(infoMessage, "INFO: ");
	appendPlayerTurnInfo(infoMessage, player);
	strcat(infoMessage, " | ");
	appendMoveCountInfo(infoMessage);
	strcat(infoMessage, " | ");
	appendLeftPiecesInfo(infoMessage);
}

void executeCommand(char* command, char* infoMessage, bool& player) {
	int inputCommandType = getCommandType(command);
	switch (inputCommandType) {
	case MOVE:
		executeMoveCommand(command, infoMessage, player);
		break;
	case BACK:
		executeBackCommand(command, infoMessage, player);
		break;
	case INFO:
		executeInfoCommand(infoMessage, player);
		break;
	case HELP:
		break;
	}
}

bool hasGameEnded() {
	bool isKingAlive = false;
	bool isKingOnGoal = false;
	int attackerCount = 0, defenderCount = 0;

	for (int i = 0; i < boardSize; ++i) {
		for (int j = 0; j < boardSize; ++j) {
			int piece = board[i][j][PIECE];
			int square = board[i][j][SQUARE];

			if (piece == ATTACKER) {
				attackerCount++;
			} else if (piece == DEFENDER) {
				defenderCount++;
			} else if (piece == KING) {
				isKingAlive = true;
				if (square == KING_GOAL) {
					isKingOnGoal = true;
				}
			}
		}
		if (isKingOnGoal) break;
	}

	return (attackerCount == 0) || (!isKingAlive) || (defenderCount == 0 && !isKingAlive) || isKingOnGoal;
}

void playerMove(bool& player, char* infoMessage) {
	char command[1024];

	char error[1024] = "";
	do {
		printTable();
		if (strlen(error) > 0) {
			std::cerr << error << std::endl;
			error[0] = '\0';
		}
		if (strlen(infoMessage) > 0) {
			std::cout << infoMessage << std::endl;
			infoMessage[0] = '\0';
		}
		std::cout << ((player == PLAYER1) ? "[ATTACKER]: " : "[DEFENDER]: ");
		std::cin.getline(command, 1024);
	} while (!isValidCommand(command, error, player));

	executeCommand(command, infoMessage, player);
}

void resetMovesFile() {
	std::ofstream movesFile("moves.vch", std::ios::trunc);
	movesFile.close();
}

void startGame() {
	resetMovesFile();
	bool gameEnded = false;
	bool currentTurn = PLAYER1;
	char infoMessage[1024] = "";

	while (!gameEnded) {
		playerMove(currentTurn, infoMessage);

		gameEnded = hasGameEnded();
	}

	printTable();
	std::cout << infoMessage << std::endl;
	std::cout << "Game ended" << std::endl;
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
		freeBoard();
		return 0;
		break;
	}

	return 0;
}