# Viking Chess

Viking Chess is a console-based implementation of the ancient strategy game Hnefatafl, often referred to as "Viking Chess." This project was developed as a solution for **Course Project 7** in the **Introduction to Programming** course at the **Faculty of Mathematics and Informatics of Sofia University** during the Winter Semester 2024/2025.

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [How to Play](#how-to-play)
- [Project Structure](#project-structure)
- [Setup and Compilation](#setup-and-compilation)
- [Usage](#usage)

## Overview
Viking Chess is a two-player strategy board game where players take on the roles of attackers and defenders. The defender’s goal is to help their king escape to a safe corner, while the attacker’s goal is to capture the king.

## Features
- Multiple predefined board sizes and layouts (e.g., 7x7, 9x9, etc.)
- Support for custom board layouts and piece skins
- Validations for moves to ensure compliance with game rules
- Game status updates
- Command-based gameplay

## How to Play
### Objective
- **Attackers**: Capture the king by surrounding it on all four sides.
- **Defenders**: Help the king escape to one of the designated escape positions (marked on the board).

### Commands
- **Move**: Use `move <from> <to>` to move a piece (e.g., `move a1 a3`).
- **Back**: Use `back <num>` to revert the game state by the specified number of moves.
- **Info**: Check the game status using `info`.
- **Help**: Get a list of available commands with `help`.
- **Quit**: Exit the game using `quit`.

## Project Structure
```plaintext
.
├── main.cpp               # Main game logic
├── pieceSkins/            # Folder containing predefined piece skins
├── startingTables/        # Folder containing predefined board layouts
├── moves.vch              # File to store recorded moves
└── README.md              # Project documentation
```

## Setup and Compilation

### Prerequisites
- **Compiler**: GCC or any C++ compiler
- **Operating System**: Windows, macOS, or Linux

### Compilation
```bash
g++ -o VikingChess main.cpp
```

### Running the Game
```bash
./VikingChess
```

## Usage
### Main Menu Options
1. **New Game**: Start a new game with the current settings.
2. **Select Table**: Choose a predefined or custom board layout.
3. **Select Skin**: Choose a predefined or custom piece skin.
4. **Quit**: Exit the game.

### Customizing Board Layouts and Skins
- **Custom Boards**: Place your `.vch` file in the `startingTables/` directory.
- **Custom Skins**: Place your `.vch` file in the `pieceSkins/` directory.

Ensure that custom files adhere to the required format.
