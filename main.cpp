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
	const int linesToClear = 100;
	for (int i = 0; i < linesToClear; ++i) {
		std::cout << '\n';
	}
	std::cout.flush();
#else
	std::cout << "\033[2J\033[H" << std::flush;
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

void toLowerStr(char* str) {
	if (str == nullptr) return;
	while (*str) {
		if (*str >= 'A' && *str <= 'Z') {
			*str = *str + ('a' - 'A');
		}
		str++;
	}
}
char* strcpy(char* destination, const char* source) {
	if (destination == nullptr || source == nullptr) return nullptr;
	char* ptr = destination;
	while (*source != '\0') {
		*ptr = *source;
		ptr++;
		source++;
	}
	*ptr = '\0';
	return destination;
}

char* strcat(char* destination, const char* source) {
	if (destination == nullptr || source == nullptr) return nullptr;
	char* ptr = destination;

	while (*ptr != '\0') {
		ptr++;
	}

	while (*source != '\0') {
		*ptr = *source;
		ptr++;
		source++;
	}

	*ptr = '\0';
	return destination;
}

int strcmp(const char* str1, const char* str2) {
	if (str1 == nullptr || str2 == nullptr) return -1;
	while (*str1 != '\0' && *str2 != '\0' && *str1 == *str2) {
		str1++;
		str2++;
	}

	return *str1 - *str2;
}

size_t strlen(const char* str) {
	if (str == nullptr) return 0;
	size_t length = 0;
	while (str[length] != '\0') {
		length++;
	}
	return length;
}

// The term 'Position' refers to a group consisting of a piece and its square
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
	KING_ESCAPE,
	KING_THRONE,
};

enum Players {
	ATTACKING_PLAYER,
	DEFENDING_PLAYER,
};

enum CommandTypes {
	MOVE,
	BACK,
	INFO,
	HELP,
	QUIT,
};

const size_t TOTAL_SQUARES = 5;
const size_t STR_MAX_LENGTH = 512;
const size_t NUM_STR_MAX_LENGTH = 10;

bool loadSkin(const char* skin, char squareChars[TOTAL_SQUARES]) {
	char filePath[STR_MAX_LENGTH] = "./pieceSkins/";
	strcat(filePath, skin);
	strcat(filePath, ".vch");
	std::ifstream file(filePath);

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

void freeBoard(int***& board, int& boardSize) {
	if (!board || boardSize == 0) return;
	for (int i = 0; i < boardSize; ++i) {
		for (int j = 0; j < boardSize; ++j) {
			delete[] board[i][j];
		}
		delete[] board[i];
	}
	delete[] board;
	board = nullptr;
}

bool determineBoardSize(std::ifstream& file, int& boardSize) {
	const int MAX_BOARD_SIZE = 256;
	char* line = new char[MAX_BOARD_SIZE];
	if (!file.getline(line, MAX_BOARD_SIZE)) {
		std::cerr << "Error: File is empty" << std::endl;
		delete[] line;
		return false;
	}

	boardSize = 0;
	size_t len = strlen(line);
	for (size_t i = 0; i < len; ++i) {
		if (line[i] != ' ') {
			++boardSize;
		}
	}
	delete[] line;
	return true;
}

void allocateBoard(int***& board, int boardSize) {
	board = new int**[boardSize];
	for (int i = 0; i < boardSize; ++i) {
		board[i] = new int*[boardSize];
		for (int j = 0; j < boardSize; ++j) {
			// As the king can be on the throne or escaping positions, the position should
			// have infomation about the type of piece and type of square
			// 0 - piece, 1 - square
			board[i][j] = new int[2];
		}
	}
}

bool parseBoardLine(const char* line, int row, int boardSize, int*** board, int& totalAttackers, int& totalDefenders, int& totalKings,
                    int& totalEscapePos) {
	int col = 0;
	for (size_t i = 0; i < strlen(line); ++i) {
		if (line[i] == ' ') continue;

		board[row][col][SQUARE] = NORMAL;

		switch (line[i]) {
		case '*':
			board[row][col][PIECE] = EMPTY;
			break;
		case 'A':
			board[row][col][PIECE] = ATTACKER;
			totalAttackers++;
			break;
		case 'D':
			board[row][col][PIECE] = DEFENDER;
			totalDefenders++;
			break;
		case 'K':
			board[row][col][PIECE] = KING;
			board[row][col][SQUARE] = KING_THRONE;
			totalKings++;
			break;
		case 'X':
			board[row][col][PIECE] = EMPTY;
			board[row][col][SQUARE] = KING_ESCAPE;
			totalEscapePos++;
			break;
		default:
			std::cerr << "Error: Unrecognized character '" << line[i] << "' at row " << row << ", col " << col << std::endl;
			return false;
		}
		col++;
	}

	if (col != boardSize) {
		std::cerr << "Error: Inconsistent number of columns at row " << row + 1 << std::endl;
		return false;
	}

	return true;
}

bool validateBoard(int boardSize, int totalAttackers, int totalDefenders, int totalKings, int totalEscapePos) {
	if (totalKings != 1) {
		std::cerr << "Error: Expected 1 king, got " << totalKings << std::endl;
		return false;
	}

	if (totalAttackers < 3) {
		std::cerr << "Error: Expected at least 3 attackers, got " << totalAttackers << std::endl;
		return false;
	}

	if (totalDefenders < 3) {
		std::cerr << "Error: Expected at least 3 defenders, got " << totalDefenders << std::endl;
		return false;
	}

	if (totalEscapePos < 1) {
		std::cerr << "Error: Expected at least 1 escape position, got " << totalEscapePos << std::endl;
		return false;
	}

	return true;
}

bool loadTable(int***& board, int& boardSize, int& totalAttackers, int& totalDefenders, const char* tableToLoad) {
	char filePath[STR_MAX_LENGTH] = "./startingTables/";
	strcat(filePath, tableToLoad);
	strcat(filePath, ".vch");

	std::ifstream file(filePath);
	if (!file.is_open()) {
		std::cerr << "Error: Couldn't open table file" << std::endl;
		return false;
	}

	if (!determineBoardSize(file, boardSize)) {
		file.close();
		return false;
	}

	allocateBoard(board, boardSize);

	file.clear();
	file.seekg(0);

	int totalKings = 0, totalEscapePos = 0;
	totalAttackers = 0;
	totalDefenders = 0;
	char* line = new char[STR_MAX_LENGTH];
	int row = 0;
	while (file.getline(line, STR_MAX_LENGTH) && row < boardSize) {
		if (!parseBoardLine(line, row, boardSize, board, totalAttackers, totalDefenders, totalKings, totalEscapePos)) {
			delete[] line;
			freeBoard(board, boardSize);
			file.close();
			return false;
		}
		row++;
	}

	delete[] line;
	file.close();

	if (row != boardSize) {
		std::cerr << "Error: Inconsistent number of rows. Expected " << boardSize << ", got " << row << std::endl;
		freeBoard(board, boardSize);
		return false;
	}

	if (!validateBoard(boardSize, totalAttackers, totalDefenders, totalKings, totalEscapePos)) {
		freeBoard(board, boardSize);
		return false;
	}

	return true;
}

int getCommandType(char* command) {
	char commandTypeStr[STR_MAX_LENGTH];
	int i = 0;
	while (command[i] != ' ' && command[i] != '\0') {
		commandTypeStr[i] = command[i];
		i++;
	}
	commandTypeStr[i] = '\0';

	if (strcmp(commandTypeStr, "move") == 0) {
		return MOVE;
	} else if (strcmp(commandTypeStr, "back") == 0) {
		return BACK;
	} else if (strcmp(commandTypeStr, "info") == 0) {
		return INFO;
	} else if (strcmp(commandTypeStr, "help") == 0) {
		return HELP;
	} else if (strcmp(commandTypeStr, "quit") == 0) {
		return QUIT;
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

bool validatePositionBounds(int boardSize, int fromRow, int fromCol, int toRow, int toColumn, char* command, char* error) {
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

bool validateIfPieceCanMoveThrough(int*** board, int fromRow, int fromCol, int toRow, int toCol, char* error) {
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

bool validateIfPieceCanMoveTo(int*** board, int fromRow, int fromCol, int toRow, int toCol, bool player, char* error) {
	int pieceOnFrom = board[fromRow][fromCol][PIECE];

	if (fromRow == toRow && fromCol == toCol) {
		strcpy(error, "Invalid move. The 'from' and 'to' positions cannot be the same");
		return false;
	}

	if (pieceOnFrom == EMPTY) {
		strcpy(error, "Invalid move. The 'from' position is empty");
		return false;
	}

	if (player == ATTACKING_PLAYER && pieceOnFrom != ATTACKER) {
		strcpy(error, "Invalid move. Only the defender can move defenders and the king");
		return false;
	}

	if (player == DEFENDING_PLAYER && pieceOnFrom != DEFENDER && pieceOnFrom != KING) {
		strcpy(error, "Invalid move. Only the attacker can move attackers");
		return false;
	}

	if (board[toRow][toCol][SQUARE] == KING_THRONE && pieceOnFrom != KING) {
		strcpy(error, "Invalid move. Only the king can move to the king's throne");
		return false;
	}

	if (board[toRow][toCol][SQUARE] == KING_ESCAPE && pieceOnFrom != KING) {
		strcpy(error, "Invalid move. Only the king can move to the king's escape positions");
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

bool validateMoveCommand(int*** board, int boardSize, char* command, char* error, bool player) {
	if (!canOpenMoveFile(error)) return false;

	int fromRow, fromCol, toRow, toCol;
	// Skip the 'move'
	int i = 4;
	if (!getNextMoveCommandCoordinates(fromRow, fromCol, command, error, i)) return false;
	if (!getNextMoveCommandCoordinates(toRow, toCol, command, error, i)) return false;

	if (!validatePositionBounds(boardSize, fromRow, fromCol, toRow, toCol, command, error)) return false;

	if (fromRow != toRow && fromCol != toCol) {
		strcpy(error, "Invalid move. The move must be either vertical or horizontal");
		return false;
	}

	if (!validateIfPieceCanMoveTo(board, fromRow, fromCol, toRow, toCol, player, error)) return false;

	if (!validateIfPieceCanMoveThrough(board, fromRow, fromCol, toRow, toCol, error)) return false;

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

bool isValidCommand(int*** board, int boardSize, char* command, char* error, bool player) {
	if (command == nullptr) return false;

	if (strlen(command) < 4) {
		strcpy(error, "Command must be at least 4 characters long");
		return false;
	}

	toLowerStr(command);

	int inputCommandType = getCommandType(command);

	if (inputCommandType == -1) {
		strcpy(error, "Input command not recognized. Use 'move <from> <to>', 'back <num>', 'info', 'help' or 'quit'");
		return false;
	}

	switch (inputCommandType) {
	case MOVE:
		return validateMoveCommand(board, boardSize, command, error, player);
		break;
	case BACK:
		return validateBackCommand(command, error);
		break;
		// info, help and quit are always valid
	}

	return true;
}

bool canCaptureKing(int*** board, int boardSize, int row, int col) {
	const int DIRECTIONS_COUNT = 4;
	const int directions[DIRECTIONS_COUNT][2] = {
	    {0, -1}, // Left
	    {0, 1},  // Right
	    {1, 0},  // Up
	    {-1, 0}  // Down
	};

	for (int i = 0; i < DIRECTIONS_COUNT; ++i) {
		int dRow = directions[i][0], dCol = directions[i][1];

		bool isEdge = row + dRow < 0 || row + dRow >= boardSize || col + dCol < 0 || col + dCol >= boardSize;
		if (isEdge) continue;

		bool isAttacker = board[row + dRow][col + dCol][PIECE] == ATTACKER;
		bool isKingEscape = board[row + dRow][col + dCol][SQUARE] == KING_ESCAPE;
		bool isKingThrone = board[row + dRow][col + dCol][SQUARE] == KING_THRONE;

		if (!(isAttacker || isKingEscape || isKingThrone)) return false;
	}

	return true;
}

bool canCapture(int*** board, int boardSize, int row, int col, int dRow, int dCol) {
	int neighbourRow = row + dRow;
	int neighbourCol = col + dCol;
	int twoTileAwayRow = row + 2 * dRow;
	int twoTileAwayCol = col + 2 * dCol;

	if (neighbourRow < 0 || neighbourRow >= boardSize || neighbourCol < 0 || neighbourCol >= boardSize) return false;

	const int neighbourPiece = board[neighbourRow][neighbourCol][PIECE];

	if (neighbourPiece == EMPTY) return false;

	if (neighbourPiece == KING) return canCaptureKing(board, boardSize, neighbourRow, neighbourCol);

	if (twoTileAwayRow < 0 || twoTileAwayRow >= boardSize || twoTileAwayCol < 0 || twoTileAwayCol >= boardSize) return false;

	const int* twoTileAwayPosition = board[twoTileAwayRow][twoTileAwayCol];
	const int currentPiece = board[row][col][PIECE];

	if (currentPiece == neighbourPiece) return false;

	bool samePiece = (twoTileAwayPosition[PIECE] == currentPiece) || (twoTileAwayPosition[PIECE] == KING && currentPiece == DEFENDER);
	bool kingThrone = (twoTileAwayPosition[SQUARE] == KING_THRONE && twoTileAwayPosition[PIECE] == EMPTY);
	bool kingEscape = (twoTileAwayPosition[SQUARE] == KING_ESCAPE);

	return samePiece || kingThrone || kingEscape;
}

void appendCaptureInfoToMessage(char* infoMessage, int row, int col) {
	char colChar[2] = {(char)(col + 'a'), '\0'};
	char rowChar[NUM_STR_MAX_LENGTH];

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

void executeMoveCommand(int*** board, int boardSize, char* command, char* infoMessage, bool& player) {
	int fromRow, fromCol, toRow, toCol;
	int i = 4;
	getNextMoveCommandCoordinates(fromRow, fromCol, command, infoMessage, i);
	getNextMoveCommandCoordinates(toRow, toCol, command, infoMessage, i);

	board[toRow][toCol][PIECE] = board[fromRow][fromCol][PIECE];
	board[fromRow][fromCol][PIECE] = EMPTY;

	const int DIRECTIONS_COUNT = 4;
	const int directions[DIRECTIONS_COUNT][2] = {
	    {0, -1}, // Left
	    {0, 1},  // Right
	    {1, 0},  // Up
	    {-1, 0}  // Down
	};

	std::ofstream movesFile("moves.vch", std::ios::app);

	movesFile << (char)('a' + fromCol) << (fromRow + 1) << (char)('a' + toCol) << (toRow + 1);

	bool atLeastOneCapture = false;
	for (int i = 0; i < DIRECTIONS_COUNT; ++i) {
		int dRow = directions[i][0], dCol = directions[i][1];
		int captureRow = toRow + dRow;
		int captureCol = toCol + dCol;
		if (canCapture(board, boardSize, toRow, toCol, dRow, dCol)) {
			if (!atLeastOneCapture) {
				strcat(infoMessage, (player == ATTACKING_PLAYER) ? "Attacker captured: " : "Defender captured: ");
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

void reverseMove(int*** board, char* move, bool& player, char* infoMessage) {
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

void executeBackCommand(int*** board, char* command, char* infoMessage, bool& player) {
	std::ifstream movesFile("moves.vch");

	int stepsBack = 1;
	getBackCommandParameter(command, infoMessage, stepsBack);

	// From, to moves - max 6 characters
	// x - 1 character
	// Max captures - 3 x 3 characters each
	const int MAX_MOVE_LENGTH = 16;
	const int MAX_MOVES = 300;
	char moves[MAX_MOVES][MAX_MOVE_LENGTH];
	int moveCount = 0;
	char move[MAX_MOVE_LENGTH];
	while (movesFile.getline(move, MAX_MOVE_LENGTH)) {
		strcpy(moves[moveCount++], move);
	}

	movesFile.close();

	for (int i = 1; i <= stepsBack; ++i) {
		reverseMove(board, moves[moveCount - i], player, infoMessage);
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

	strcat(infoMessage, "Number of moves so far: ");
	if (moveCount == 0) {
		strcat(infoMessage, "0");
		return;
	}
	char moveCountChar[NUM_STR_MAX_LENGTH];
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
	strcat(infoMessage, (player == ATTACKING_PLAYER) ? "Attacker's turn" : "Defender's turn");
}

void getPiecesInfo(int*** board, int boardSize, int& attackerCount, int& defenderCount, bool& isKingAlive, bool& isKingOnEscape) {
	attackerCount = 0;
	defenderCount = 0;
	isKingAlive = false;
	isKingOnEscape = false;

	for (int i = 0; i < boardSize; ++i) {
		for (int j = 0; j < boardSize; ++j) {
			if (board[i][j][PIECE] == ATTACKER) {
				attackerCount++;
			} else if (board[i][j][PIECE] == DEFENDER) {
				defenderCount++;
			} else if (board[i][j][PIECE] == KING) {
				isKingAlive = true;
				if (board[i][j][SQUARE] == KING_ESCAPE) {
					isKingOnEscape = true;
				}
			}
		}
	}
}

void intToStr(int num, char* str) {
	int i = 0;
	char temp[NUM_STR_MAX_LENGTH];
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

void appendLeftPiecesInfo(int*** board, int boardSize, int totalAttackers, int totalDefenders, char* infoMessage) {
	strcat(infoMessage, "Pieces left: ");
	bool isKingAlive, unusedKingEscape;
	int attackerCount, defenderCount;
	getPiecesInfo(board, boardSize, attackerCount, defenderCount, isKingAlive, unusedKingEscape);

	strcat(infoMessage, "Attackers: ");
	char numStr[STR_MAX_LENGTH];
	intToStr(attackerCount, numStr);
	strcat(infoMessage, numStr);
	strcat(infoMessage, "/");
	intToStr(totalAttackers, numStr);
	strcat(infoMessage, numStr);

	strcat(infoMessage, ", Defenders: ");
	intToStr(defenderCount + isKingAlive, numStr);
	strcat(infoMessage, numStr);
	strcat(infoMessage, "/");
	const int TOTAL_KINGS = 1;
	intToStr(totalDefenders + TOTAL_KINGS, numStr);
	strcat(infoMessage, numStr);
}

void executeInfoCommand(int*** board, int boardSize, int& totalAttackers, int& totalDefenders, char* infoMessage, bool player) {
	strcat(infoMessage, "Current game status: [");
	appendPlayerTurnInfo(infoMessage, player);
	strcat(infoMessage, " | ");
	appendMoveCountInfo(infoMessage);
	strcat(infoMessage, " | ");
	appendLeftPiecesInfo(board, boardSize, totalAttackers, totalDefenders, infoMessage);
	strcat(infoMessage, "]");
}

void executeCommand(int*** board, int boardSize, int totalAttackers, int totalDefenders, char* command, char* infoMessage, bool& player,
                    bool& gameEnded) {
	int inputCommandType = getCommandType(command);
	switch (inputCommandType) {
	case MOVE:
		executeMoveCommand(board, boardSize, command, infoMessage, player);
		break;
	case BACK:
		executeBackCommand(board, command, infoMessage, player);
		break;
	case INFO:
		executeInfoCommand(board, boardSize, totalAttackers, totalDefenders, infoMessage, player);
		break;
	case HELP:
		strcat(infoMessage, "Available commands:\n");
		strcat(infoMessage, "  move <from> <to> - Moves a piece from one position to another\n");
		strcat(infoMessage, "  back <num> - Reverts the game state by a specified number of moves\n");
		strcat(infoMessage, "  info - Displays current game information\n");
		strcat(infoMessage, "  help - Displays a list of available commands\n");
		strcat(infoMessage, "  quit - Quits the game to the main menu");
		break;
	case QUIT:
		gameEnded = true;
		break;
	}
}

bool hasGameEnded(int*** board, int boardSize, char* infoMessage) {
	bool isKingAlive = false;
	bool isKingOnEscape = false;
	int attackerCount = 0, defenderCount = 0;
	getPiecesInfo(board, boardSize, attackerCount, defenderCount, isKingAlive, isKingOnEscape);

	if (attackerCount == 0) {
		strcpy(infoMessage, "No more attackers left");
		return true;
	}

	if (!isKingAlive) {
		strcpy(infoMessage, "The King is dead");
		return true;
	}

	if (isKingOnEscape) {
		strcpy(infoMessage, "The King escaped from the board");
		return true;
	}

	return false;
}

void printTable(int*** board, int boardSize, const char* squareChars) {
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
			if (board[i][j][PIECE] == EMPTY && (board[i][j][SQUARE] == KING_ESCAPE || board[i][j][SQUARE] == KING_THRONE)) {
				const int INDEX_FOR_THRONE_AND_ESCAPE = 4;
				std::cout << squareChars[INDEX_FOR_THRONE_AND_ESCAPE];
			} else {
				std::cout << squareChars[board[i][j][PIECE]];
			}
			if (j != boardSize - 1) std::cout << ' ';
		}
		std::cout << "| " << rowHelper++;
		std::cout << std::endl;
	}
}

void playerMove(int*** board, int boardSize, int totalAttackers, int totalDefenders, char* squareChars, bool& player, char* infoMessage,
                bool& gameEnded) {
	char command[STR_MAX_LENGTH];

	char error[STR_MAX_LENGTH] = "";
	do {
		printTable(board, boardSize, squareChars);
		if (strlen(error) > 0) {
			std::cerr << error << std::endl;
			error[0] = '\0';
		}
		if (strlen(infoMessage) > 0) {
			std::cout << infoMessage << std::endl;
			infoMessage[0] = '\0';
		}
		std::cout << ((player == ATTACKING_PLAYER) ? "[ATTACKER]: " : "[DEFENDER]: ");
		std::cin.getline(command, STR_MAX_LENGTH);
	} while (!isValidCommand(board, boardSize, command, error, player));

	executeCommand(board, boardSize, totalAttackers, totalDefenders, command, infoMessage, player, gameEnded);
}

bool clearMovesFile() {
	std::ofstream movesFile("moves.vch", std::ios::trunc);
	if (!movesFile.is_open()) {
		std::cerr << "Error: Couldn't open moves file" << std::endl;
		return false;
	}
	movesFile.close();
	return true;
}

void startGame(int***& board, int& boardSize, int& totalAttackers, int& totalDefenders, char* squareChars, char* table) {
	const char* tableToLoad = (strcmp(table, "") == 0) ? "9x9" : table;
	if (!loadTable(board, boardSize, totalAttackers, totalDefenders, tableToLoad)) return;

	clearMovesFile();

	bool gameEnded = false;
	bool currentTurn = ATTACKING_PLAYER;
	char infoMessage[STR_MAX_LENGTH] = "";

	while (!gameEnded) {
		playerMove(board, boardSize, totalAttackers, totalDefenders, squareChars, currentTurn, infoMessage, gameEnded);

		if (hasGameEnded(board, boardSize, infoMessage)) gameEnded = true;
	}

	printTable(board, boardSize, squareChars);
	std::cout << "~Game ended~" << std::endl;
	if (strlen(infoMessage) > 0) {
		// Opposite player because after the last playMove() currentTurn gets changed
		std::cout << ((!currentTurn == ATTACKING_PLAYER) ? "Attackers" : "Defenders") << " won!" << std::endl;
		std::cout << infoMessage << std::endl;
	}
	std::cout << "Press Enter to continue...";
	std::cin.get();
}

char* inputFilePath(const char* startingPath, const char* message) {
	char* filePath = new char[STR_MAX_LENGTH];
	char* chosenFile = new char[STR_MAX_LENGTH];
	do {
		std::cout << message;
		std::cin.getline(chosenFile, STR_MAX_LENGTH);
		strcpy(filePath, startingPath);
		strcat(filePath, chosenFile);
		strcat(filePath, ".vch");
		std::ifstream file(filePath);
		if (!file.is_open()) {
			std::cerr << "Error: Couldn't open file inside " << startingPath << ". Please check if the file exists and try again."
			          << std::endl;
			chosenFile[0] = '\0';
		} else {
			file.close();
		}
	} while (strlen(chosenFile) == 0);
	return chosenFile;
}

int multipleChoice(const char* choices[], int numChoices) {
	for (int i = 0; i < numChoices; ++i) {
		std::cout << "[" << i + 1 << "]: " << choices[i] << std::endl;
	}

	bool validChoice = false;
	int choice = 0;
	const int IGNORE_CHARS = 10000;

	while (!validChoice) {
		std::cout << "Choice: ";
		std::cin >> choice;

		if (std::cin.fail() || choice <= 0 || choice > numChoices) {
			std::cin.clear();
			std::cin.ignore(IGNORE_CHARS, '\n');
			std::cout << "Invalid choice, please enter a number between 1 and " << numChoices << "." << std::endl;
		} else {
			std::cin.ignore(IGNORE_CHARS, '\n');
			validChoice = true;
		}
	}
	return choice;
}

void chooseTable(int***& board, int& boardSize, int& totalAttackers, int& totalDefenders, char* chosenTable) {
	clearTerminal();
	std::cout << "~Table Selection~" << std::endl;
	const char* choices[] = {"7x7", "9x9", "11x11", "13x13", "Alea-Evangelii", "Ealdfaeder", "Input your own"};
	const int CHOISES_COUNT = 7;
	int choice;
	do {
		choice = multipleChoice(choices, CHOISES_COUNT);
		if (choice == CHOISES_COUNT) {
			do {
				strcpy(chosenTable, inputFilePath("./startingTables/", "Enter only the name (without .vch) of your table file: "));
			} while (!loadTable(board, boardSize, totalAttackers, totalDefenders, chosenTable));
			return;
		}
		strcpy(chosenTable, choices[choice - 1]);
		toLowerStr(chosenTable);
	} while (!loadTable(board, boardSize, totalAttackers, totalDefenders, chosenTable));
}

void chooseSkin(char*& squareChars) {
	clearTerminal();
	std::cout << "~Skin Selection~" << std::endl;
	const char* choices[] = {"Alphabet",       "Default", "Frost",   "Matrix",        "Minimalistic",
	                         "Nordic-Minimal", "Runic",   "valhala", "Input your own"};
	char chosenSkin[STR_MAX_LENGTH];
	const int CHOISES_COUNT = 9;
	int choice;
	do {
		choice = multipleChoice(choices, CHOISES_COUNT);
		if (choice == CHOISES_COUNT) {
			do {
				strcpy(chosenSkin, inputFilePath("./pieceSkins/", "Enter only the name (without .vch) of your skin file: "));
			} while (!loadSkin(chosenSkin, squareChars));
			return;
		}
		strcpy(chosenSkin, choices[choice - 1]);
		toLowerStr(chosenSkin);
	} while (!loadSkin(chosenSkin, squareChars));
}

void MainMenu(int***& board, int& boardSize, char* squareChars, int& totalAttackers, int& totalDefenders) {
	bool quit = false;
	char chosenTable[STR_MAX_LENGTH] = {0};

	while (!quit) {
		clearTerminal();
		std::cout << "~Main Menu~" << std::endl;
		const char* choices[] = {"New game", "Select Table", "Select Skin", "Quit"};
		int choice = multipleChoice(choices, 4);
		switch (choice) {
		case 1:
			startGame(board, boardSize, totalAttackers, totalDefenders, squareChars, chosenTable);
			break;
		case 2:
			chooseTable(board, boardSize, totalAttackers, totalDefenders, chosenTable);
			break;
		case 3:
			chooseSkin(squareChars);
			break;
		case 4:
			freeBoard(board, boardSize);
			quit = true;
			return;
		}
	}
}

void VikingChess() {
	char squareChars[TOTAL_SQUARES];
	int*** board;
	int boardSize;
	int totalAttackers, totalDefenders;
	if (!loadSkin("default", squareChars)) return;
	MainMenu(board, boardSize, squareChars, totalAttackers, totalDefenders);
}

int main() {
	VikingChess();
	return 0;
}