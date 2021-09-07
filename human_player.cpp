#include "human_player.h"

#include "exceptions.h"
#include "formatting.h"
#include "move.h"
#include "place_result.h"
#include "rang.h"
#include "tile_kind.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

// This method is fully implemented.
inline string& to_upper(string& str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

Move HumanPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
    // TODO: begin your implementation here.

    // the try/catch block makes sure the user enters a valid move
    while (1) {
        try {
            board.print(cout);
            print_hand(cout);
            cout << "Please enter your move:" << endl;
            string turn;
            // get the move input and then parses through the move
            getline(cin, turn);
            Move move = parse_move(turn);
            if (move.kind == MoveKind::PASS || move.kind == MoveKind::EXCHANGE) {
                return move;
            } else {
                // checks if a valid placement was made on the board
                PlaceResult boardCheck = board.test_place(move);
                if (!boardCheck.valid) {
                    // error due to an invalid placement
                    // thrown an exception
                    throw MoveException("boardcheck is false");
                } else {
                    // checks if the word is in the dictionary
                    bool validWord = true;
                    for (unsigned int i = 0; i < boardCheck.words.size(); i++) {
                        if (!dictionary.is_word(boardCheck.words[i])) {
                            validWord = false;
                        }
                    }
                    if (!validWord) {
                        // error due to invalid word
                        // thrown an exception
                        throw CommandException("invalid word");
                    }
                    return move;
                }
            }
        } catch (...) {
            cout << "the user entered invalid move" << endl;
            cout << "please enter a valid move:" << endl;
        }
    }
}

vector<TileKind> HumanPlayer::parse_tiles(string& letters, string move) const {
    // TODO: begin implementation here.

    bool match = false;
    vector<TileKind> word;
    for (unsigned int i = 0; i < letters.length(); i++) {
        for (unsigned int j = 0; j < playerTiles.size(); j++) {
            if (move == "exchange") {
                match = false;
                // makes sure that the letters being used by the player are in their hand
                if (letters[i] == playerTiles[j].letter) {
                    match = true;
                    // creates a vector of tilekinds based on the letters
                    TileKind t = TileKind(letters[i], playerTiles[j].points);
                    word.push_back(t);
                    break;
                }
            } else {
                match = false;
                // makes sure that the letters being used by the player are in their hand
                if (letters[i] == playerTiles[j].letter) {
                    match = true;
                    if (letters[i] == '?') {
                        // creates a tilekind specifically for blank tiles
                        TileKind t = TileKind(letters[i], 0, letters[i + 1]);
                        word.push_back(t);
                        i++;
                        break;
                    } else {
                        // creates a vector of tilekinds based on the letters
                        TileKind t = TileKind(letters[i], playerTiles[j].points);
                        word.push_back(t);
                        break;
                    }
                }
            }
        }
        // throws an exception if the tiles do not match
        if (match == false) {
            throw MoveException("tiles do not match");
        }
    }

    return word;
}

Move HumanPlayer::parse_move(string& move_string) const {
    // TODO: begin implementation here.
    stringstream ss(move_string);
    string moveType;
    // gets the type of move input by the player
    ss >> moveType;
    transform(moveType.cbegin(), moveType.cend(), moveType.begin(), ::tolower);
    if (moveType == "place") {
        string direction;
        ss >> direction;
        Move move;
        // gets the direction and throws an exception if the user inputs an invalid direction
        if (direction == "-") {
            move.direction = Direction::ACROSS;
        } else if (direction == "|") {
            move.direction = Direction::DOWN;
        } else {
            throw MoveException("incorrect direction");
        }
        size_t row;
        size_t column;
        // gets the row and column input by the user
        ss >> row >> column;
        move.row = row - 1;
        move.column = column - 1;
        string letters;
        // gets the letters to be used for the move
        ss >> letters;
        transform(letters.cbegin(), letters.cend(), letters.begin(), ::tolower);
        vector<TileKind> word = parse_tiles(letters, moveType);
        move.tiles = word;
        move.kind = MoveKind::PLACE;
        // returns the final move for a placement
        return move;

    } else if (moveType == "exchange") {
        Move move;
        string letters;
        // gets the letters input by the user
        ss >> letters;
        transform(letters.cbegin(), letters.cend(), letters.begin(), ::tolower);
        vector<TileKind> word = parse_tiles(letters, moveType);
        move.kind = MoveKind::EXCHANGE;
        move.tiles = word;
        // returns the final move for an exchange
        return move;
    } else if (moveType == "pass") {
        Move move;
        move.kind = MoveKind::PASS;
        // returns the final move for a pass
        return move;
    } else {
        // throws an exception in the type of move is invalid
        throw MoveException("incorrect movetype");
    }
}

bool HumanPlayer::is_human() const { return true; }

// This function is fully implemented.
void HumanPlayer::print_hand(ostream& out) const {
    const size_t tile_count = this->count_tiles();
    const size_t empty_tile_count = this->get_hand_size() - tile_count;
    const size_t empty_tile_width = empty_tile_count * (SQUARE_OUTER_WIDTH - 1);

    for (size_t i = 0; i < HAND_TOP_MARGIN - 2; ++i) {
        out << endl;
    }

    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_HEADING << "Your Hand: " << endl << endl;

    // Draw top line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;

    // Draw middle 3 lines
    for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
        out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD << repeat(SPACE, HAND_LEFT_MARGIN);
        for (auto it = playerTiles.cbegin(); it != playerTiles.cend(); ++it) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_PLAYER_HAND;

            // Print letter
            if (line == 1) {
                out << repeat(SPACE, 2) << FG_COLOR_LETTER << (char)toupper(it->letter) << repeat(SPACE, 2);

                // Print score in bottom right
            } else if (line == SQUARE_INNER_HEIGHT - 1) {
                out << FG_COLOR_SCORE << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << setw(2) << it->points;

            } else {
                out << repeat(SPACE, SQUARE_INNER_WIDTH);
            }
        }
        if (this->count_tiles() > 0) {
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
            out << repeat(SPACE, empty_tile_width) << BG_COLOR_OUTSIDE_BOARD << endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, HAND_LEFT_MARGIN) << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE;
    print_horizontal(tile_count, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << repeat(SPACE, empty_tile_width) << rang::style::reset << endl;
}
