#include "board.h"

#include "board_square.h"
#include "exceptions.h"
#include "formatting.h"
#include <fstream>
#include <iomanip>

using namespace std;

bool Board::Position::operator==(const Board::Position& other) const {
    return this->row == other.row && this->column == other.column;
}

bool Board::Position::operator!=(const Board::Position& other) const {
    return this->row != other.row || this->column != other.column;
}

Board::Position Board::Position::translate(Direction direction) const { return this->translate(direction, 1); }

Board::Position Board::Position::translate(Direction direction, ssize_t distance) const {
    if (direction == Direction::DOWN) {
        return Board::Position(this->row + distance, this->column);
    } else {
        return Board::Position(this->row, this->column + distance);
    }
}

Board Board::read(const string& file_path) {
    ifstream file(file_path);
    if (!file) {
        throw FileException("cannot open board file!");
    }

    size_t rows;
    size_t columns;
    size_t starting_row;
    size_t starting_column;
    file >> rows >> columns >> starting_row >> starting_column;
    Board board(rows, columns, starting_row, starting_column);

    // TODO: complete implementation of reading in board from file here.

    // all sqaures that make up a scrabble board
    BoardSquare doubleLet(2, 1);
    BoardSquare tripleLet(3, 1);
    BoardSquare doubleWord(1, 2);
    BoardSquare tripleWord(1, 3);
    BoardSquare normal(1, 1);

    // reads the row and pushes back the corresponding board sqaure
    for (size_t i = 0; i < rows; i++) {
        vector<BoardSquare> rowVec;
        string rowRead;
        file >> rowRead;
        for (size_t j = 0; j < columns; j++) {
            if (rowRead[j] == '.') {
                rowVec.push_back(normal);
            } else if (rowRead[j] == '2') {
                rowVec.push_back(doubleLet);
            } else if (rowRead[j] == '3') {
                rowVec.push_back(tripleLet);
            } else if (rowRead[j] == 'd') {
                rowVec.push_back(doubleWord);
            } else {
                rowVec.push_back(tripleWord);
            }
        }
        // pushes the row vector back into a vector that holds
        // the boardsquare vectors
        board.squares.push_back(rowVec);
    }
    return board;
}

size_t Board::get_move_index() const { return this->move_index; }

PlaceResult Board::test_place(const Move& move) const {
    // TODO: complete implementation here

    // test_place should not run if the move is not a placement
    if (move.kind != MoveKind::PLACE) {
        string error = "did not choose to make a placement";
        return PlaceResult(error);
    }
    // if there is a tile at the starting sqaure, an error is thrown
    Position current = Position(move.row, move.column);
    if (this->at(current).has_tile()) {
        string error = "incorrect placement";
        return PlaceResult(error);
    } else {
        int wordrange = move.tiles.size();
        // if the board's start tile does not have a tile, the following code runs
        if (!this->at(start).has_tile()) {
            int points = 0;
            string word;
            vector<std::string> wordResult;
            bool onStart = false;
            int letter_mult = 0;
            int word_mult = 1;
            // the first move must cross over the starting tile
            // an error is thrown otherwise
            if (move.row != start.row && move.column != start.column) {
                string error = "incorrect start placement";
                return PlaceResult(error);
            }
            if (move.direction == Direction::ACROSS) {
                int index = 0;
                for (unsigned int i = move.column; i < (move.column) + wordrange; i++) {
                    Position current = Position(move.row, i);
                    // the assigned value of tile is pushed back if the letter
                    // is a blank tile
                    if (!this->is_in_bounds(current)) {
                        return PlaceResult("out of bounds");
                    }
                    if (move.tiles[index].letter == '?') {
                        word.push_back(move.tiles[index].assigned);
                        letter_mult = this->at(current).letter_multiplier;
                        word_mult *= this->at(current).word_multiplier;
                        points += move.tiles[index].points * letter_mult;
                    }
                    // otherwise, letter and word multipliers are used to get the
                    // total number of points for the placement of a word
                    else {
                        letter_mult = this->at(current).letter_multiplier;
                        word_mult *= this->at(current).word_multiplier;
                        word.push_back(move.tiles[index].letter);
                        points += move.tiles[index].points * letter_mult;
                    }
                    // checks if the word crosses the start tile
                    if (current == start) {
                        onStart = true;
                    }
                    index++;
                }
            }
            // same implementation as above, but for the downward direction
            else if (move.direction == Direction::DOWN) {
                int index = 0;
                for (unsigned int i = move.row; i < (move.row) + wordrange; i++) {
                    Position current = Position(i, move.column);
                    if (!this->is_in_bounds(current)) {
                        return PlaceResult("out of bounds");
                    }
                    if (move.tiles[index].letter == '?') {
                        word.push_back(move.tiles[index].assigned);
                        letter_mult = this->at(current).letter_multiplier;
                        word_mult *= this->at(current).word_multiplier;
                        points += move.tiles[index].points * letter_mult;
                    } else {
                        letter_mult = this->at(current).letter_multiplier;
                        word_mult *= this->at(current).word_multiplier;
                        word.push_back(move.tiles[index].letter);
                        points += move.tiles[index].points * letter_mult;
                    }
                    if (current == start) {
                        onStart = true;
                    }
                    index++;
                }
            }
            if (move.tiles.size() <= 1) {
                string error = "incorrect start placement";
                return PlaceResult(error);
            }
            // an error is thrown if the starting move does not go over
            // the start tile
            if (onStart == false) {
                string error = "incorrect start placement";
                return PlaceResult(error);
            }
            points *= word_mult;         // points are multiplied by the word multiplier
            wordResult.push_back(word);  // the words is pushed back into a vector of words
            return PlaceResult(wordResult, points);
        }
        bool adjacent = false;
        // the following code runs if the move direction is across
        if (move.direction == Direction::ACROSS) {
            string word;
            vector<std::string> wordResult;
            int points = 0;
            Position current = Position(move.row, move.column);
            Position left = Position(move.row, move.column - 1);
            // if the position left of current has a tile, adjacent is set to true
            // and we continue iterating left while the position is in bounds
            // and has a tile
            if (this->in_bounds_and_has_tile(left)) {
                adjacent = true;
                int i = 1;
                left = Position(move.row, move.column - i);
                while (this->in_bounds_and_has_tile(left)) {
                    i++;
                    left = Position(move.row, move.column - i);
                    if (this->in_bounds_and_has_tile(left)) {
                        continue;
                    } else {
                        i--;
                        left = Position(move.row, move.column - i);
                        break;
                    }
                }
                // add up points and push back letters of the tiles left
                // of the current position
                while (left != current) {
                    // if the tile holds a question mark, its assigned letter is
                    // pushed back
                    if (this->at(left).get_tile_kind().letter == '?') {
                        points += this->at(left).get_tile_kind().points;
                        word.push_back(this->at(left).get_tile_kind().assigned);
                    } else {
                        points += this->at(left).get_tile_kind().points;
                        word.push_back(this->at(left).get_tile_kind().letter);
                    }
                    i--;
                    left = Position(move.row, move.column - i);
                }
            }
            int letter_mult = 0;
            int word_mult = 1;
            int index = 0;
            // iterate through the range of the new letters being placed
            for (unsigned int j = move.column; j < (move.column) + wordrange; j++) {
                current = Position(move.row, j);
                // if the current square has a tile, its point value and letter are
                // evaluated and wordrange is incremented
                if (!this->is_in_bounds(current)) {
                    return PlaceResult("out of bounds error");
                }
                if (this->at(current).has_tile()) {
                    adjacent = true;
                    // checks if the square is a blank tile
                    if (this->at(current).get_tile_kind().letter == '?') {
                        points += this->at(current).get_tile_kind().points;
                        word.push_back(this->at(current).get_tile_kind().assigned);
                    } else {
                        points += this->at(current).get_tile_kind().points;
                        word.push_back(this->at(current).get_tile_kind().letter);
                    }
                    wordrange++;
                }
                // check if the current square is in bounds
                else if (this->is_in_bounds(current)) {
                    // checks if the character in the tile vector is a question mark
                    if (move.tiles[index].letter == '?') {
                        word_mult *= this->at(current).word_multiplier;
                        letter_mult = this->at(current).letter_multiplier;
                        word.push_back(move.tiles[index].assigned);
                        points += move.tiles[index].points * letter_mult;
                        index++;
                    }
                    // if the letter is not a question mark, the boardsquare is
                    // evaluated according to letter and word multipliers.
                    // the points and letter are also evaluated
                    else {
                        letter_mult = this->at(current).letter_multiplier;
                        word_mult *= this->at(current).word_multiplier;
                        points += (move.tiles[index].points * letter_mult);
                        word.push_back(move.tiles[index].letter);
                        index++;
                    }
                }
                // an error is returned if the move is out of bounds
                else {
                    string error = "out of bounds";
                    return PlaceResult(error);
                }
            }
            int k = 0;
            // checks if the position right of new word contains a tile
            Position right = Position(move.row, (move.column) + wordrange + k);
            if (this->in_bounds_and_has_tile(right)) {
                adjacent = true;
                // iterates to the right while the position has a tile
                while (this->in_bounds_and_has_tile(right)) {
                    if (this->at(right).get_tile_kind().letter == '?') {
                        points += this->at(right).get_tile_kind().points;
                        word.push_back(this->at(right).get_tile_kind().assigned);
                    } else {
                        points += this->at(right).get_tile_kind().points;
                        word.push_back(this->at(right).get_tile_kind().letter);
                    }
                    k++;
                    right = Position(move.row, (move.column) + wordrange + k);
                }
            }
            // points are multiplied according to word multipliers and the
            // word is pushed back into the vector of words
            if (word.size() > 1) {
                wordResult.push_back(word);
                points *= word_mult;
            } else {
                points = 0;
            }
            word.clear();

            letter_mult = 0;
            index = 0;
            wordrange = move.tiles.size();
            // iterates through the range of the new word to check if new words
            // are created above or below the word
            for (unsigned int j = move.column; j < (move.column) + wordrange; j++) {
                int k = 1;
                int indiv_points = 0;
                word_mult = 1;
                Position current = Position(move.row, j);
                Position top = Position(move.row - k, j);
                Position down = Position(move.row + 1, j);

                // adjacency check for letters above or below the current tile
                if (this->in_bounds_and_has_tile(top)) {
                    adjacent = true;
                }
                if (this->in_bounds_and_has_tile(down)) {
                    adjacent = true;
                }

                if (this->in_bounds_and_has_tile(top) && !this->in_bounds_and_has_tile(down)) {
                    if (!this->at(current).has_tile()) {
                        // if there is a placed letter above, we iterate up while the squares
                        // have tiles and are in bounds
                        while (this->in_bounds_and_has_tile(top)) {
                            k++;
                            top = Position(move.row - k, j);
                            if (this->in_bounds_and_has_tile(top)) {
                                continue;
                            } else {
                                k--;
                                top = Position(move.row - k, j);
                                break;
                            }
                        }
                        // the points and letters of the above tiles are evaluated
                        while (this->in_bounds_and_has_tile(top)) {
                            // check if the tile has a question mark
                            if (this->at(top).get_tile_kind().letter == '?') {
                                points += this->at(top).get_tile_kind().points;
                                word.push_back(this->at(top).get_tile_kind().assigned);
                            } else {
                                indiv_points += this->at(top).get_tile_kind().points;
                                word.push_back(this->at(top).get_tile_kind().letter);
                            }
                            k--;
                            top = Position(move.row - k, j);
                        }
                        // checks letter and word multpliers and evaluates the current tile
                        // once more
                        letter_mult = this->at(current).letter_multiplier;
                        word_mult *= this->at(current).word_multiplier;
                        if (move.tiles[index].letter == '?') {
                            word.push_back(move.tiles[index].assigned);
                            indiv_points += move.tiles[index].points;
                        } else {
                            indiv_points += move.tiles[index].points * letter_mult;
                            word.push_back(move.tiles[index].letter);
                        }

                        points += indiv_points * word_mult;
                        wordResult.push_back(word);
                        word.clear();
                    } else {
                        wordrange++;
                        continue;
                    }
                    index++;
                }
                // checks for a tile below the current square
                else if (this->in_bounds_and_has_tile(down) && !this->in_bounds_and_has_tile(top)) {
                    k = 0;
                    down = Position(move.row + 1 + k, j);
                    if (!this->at(current).has_tile()) {
                        // evalutes the current tile before iterating through the tiles below
                        letter_mult = this->at(current).letter_multiplier;
                        word_mult *= this->at(current).word_multiplier;
                        if (move.tiles[index].letter == '?') {
                            word.push_back(move.tiles[index].assigned);
                            indiv_points += move.tiles[index].points;
                        } else {
                            indiv_points += move.tiles[index].points * letter_mult;
                            word.push_back(move.tiles[index].letter);
                        }
                        // evalutes the tiles below while the position is in bounds and has a tile
                        while (this->in_bounds_and_has_tile(down)) {
                            if (this->at(down).get_tile_kind().letter == '?') {
                                points += this->at(down).get_tile_kind().points;
                                word.push_back(this->at(down).get_tile_kind().assigned);
                            } else {
                                indiv_points += this->at(down).get_tile_kind().points;
                                word.push_back(this->at(down).get_tile_kind().letter);
                            }
                            k++;
                            down = Position(move.row + 1 + k, j);
                        }
                        points += indiv_points * word_mult;
                        wordResult.push_back(word);
                        word.clear();
                    } else {
                        wordrange++;
                        continue;
                    }
                    index++;
                } else if (this->in_bounds_and_has_tile(current)) {
                    wordrange++;
                } else if (
                        this->in_bounds_and_has_tile(top) && this->in_bounds_and_has_tile(down)
                        && !this->at(current).has_tile()) {
                    while (this->in_bounds_and_has_tile(top)) {
                        k++;
                        top = Position(move.row - k, j);
                        if (this->in_bounds_and_has_tile(top)) {
                            continue;
                        } else {
                            k--;
                            top = Position(move.row - k, j);
                            break;
                        }
                    }
                    // the points and letters of the above tiles are evaluated
                    while (this->in_bounds_and_has_tile(top)) {
                        // check if the tile has a question mark
                        if (this->at(top).get_tile_kind().letter == '?') {
                            points += this->at(top).get_tile_kind().points;
                            word.push_back(this->at(top).get_tile_kind().assigned);
                        } else {
                            indiv_points += this->at(top).get_tile_kind().points;
                            word.push_back(this->at(top).get_tile_kind().letter);
                        }
                        k--;
                        top = Position(move.row - k, j);
                    }
                    letter_mult = this->at(current).letter_multiplier;
                    word_mult *= this->at(current).word_multiplier;
                    if (move.tiles[index].letter == '?') {
                        word.push_back(move.tiles[index].assigned);
                        indiv_points += move.tiles[index].points;
                    } else {
                        indiv_points += move.tiles[index].points * letter_mult;
                        word.push_back(move.tiles[index].letter);
                    }
                    while (this->in_bounds_and_has_tile(down)) {
                        if (this->at(down).get_tile_kind().letter == '?') {
                            points += this->at(down).get_tile_kind().points;
                            word.push_back(this->at(down).get_tile_kind().assigned);
                        } else {
                            indiv_points += this->at(down).get_tile_kind().points;
                            word.push_back(this->at(down).get_tile_kind().letter);
                        }
                        k++;
                        down = Position(move.row + 1 + k, j);
                    }
                    points += indiv_points * word_mult;
                    wordResult.push_back(word);
                    word.clear();
                    index++;
                } else {
                    index++;
                }
            }
            // if there are no adjacent tiles, an error is returned
            if (adjacent == false) {
                string error = "incorrect word placement";
                return PlaceResult(error);
            }
            return PlaceResult(wordResult, points);

        }
        // the following code runs if the move direction is down
        else if (move.direction == Direction::DOWN) {
            string word;
            vector<std::string> wordResult;
            int points = 0;
            Position current = Position(move.row, move.column);
            Position up = Position(move.row - 1, move.column);
            // if the position above current has a tile, adjacent is set to true
            // and we continue iterating up while the position is in bounds
            // and has a tile
            if (this->in_bounds_and_has_tile(up)) {
                adjacent = true;
                int i = 1;
                up = Position(move.row - i, move.column);
                while (this->in_bounds_and_has_tile(up)) {
                    i++;
                    up = Position(move.row - i, move.column);
                    if (this->in_bounds_and_has_tile(up)) {
                        continue;
                    } else {
                        i--;
                        up = Position(move.row - i, move.column);
                        break;
                    }
                }
                // add up points and push back letters of the tiles above
                // of the current position
                while (up != current) {
                    // if the tile holds a question mark, its assigned letter is
                    // pushed back
                    if (this->at(up).get_tile_kind().letter == '?') {
                        points += this->at(up).get_tile_kind().points;
                        word.push_back(this->at(up).get_tile_kind().assigned);
                    } else {
                        points += this->at(up).get_tile_kind().points;
                        word.push_back(this->at(up).get_tile_kind().letter);
                    }
                    i--;
                    up = Position(move.row - i, move.column);
                }
            }
            int letter_mult = 0;
            int word_mult = 1;
            int index = 0;
            // iterate through the range of the new letters being placed
            for (unsigned int j = move.row; j < (move.row) + wordrange; j++) {
                current = Position(j, move.column);
                // if the current square has a tile, its point value and letter are
                // evaluated and wordrange is incremented
                if (!this->is_in_bounds(current)) {
                    return PlaceResult("out of bounds error");
                }
                if (this->at(current).has_tile()) {
                    adjacent = true;
                    // checks if the square holds a blank tile
                    if (this->at(current).get_tile_kind().letter == '?') {
                        points += this->at(current).get_tile_kind().points;
                        word.push_back(this->at(current).get_tile_kind().assigned);
                    } else {
                        points += this->at(current).get_tile_kind().points;
                        word.push_back(this->at(current).get_tile_kind().letter);
                    }
                    wordrange++;
                }
                // check if the current square is in bounds
                else if (this->is_in_bounds(current)) {
                    // checks if the character in the tile vector is a question mark
                    if (move.tiles[index].letter == '?') {
                        word_mult *= this->at(current).word_multiplier;
                        word.push_back(move.tiles[index].assigned);
                        points += move.tiles[index].points;
                        index++;
                    }
                    // if the letter is not a question mark, the boardsquare is
                    // evaluated according to letter and word multipliers.
                    // the points and letter are also evaluated
                    else {
                        letter_mult = this->at(current).letter_multiplier;
                        word_mult *= this->at(current).word_multiplier;
                        points += (move.tiles[index].points * letter_mult);
                        word.push_back(move.tiles[index].letter);
                        index++;
                    }
                }
                // if the word goes out of bounds, an error is thrown
                else {
                    string error = "out of bounds";
                    return PlaceResult(error);
                }
            }

            int k = 0;
            // checks if the position below the new word contains a tile
            Position down = Position((move.row) + wordrange + k, move.column);
            if (this->in_bounds_and_has_tile(down)) {
                adjacent = true;
                // iterates down while the position has a tile
                while (this->in_bounds_and_has_tile(down)) {
                    if (this->at(down).get_tile_kind().letter == '?') {
                        points += this->at(down).get_tile_kind().points;
                        word.push_back(this->at(down).get_tile_kind().assigned);
                    } else {
                        points += this->at(down).get_tile_kind().points;
                        word.push_back(this->at(down).get_tile_kind().letter);
                    }
                    k++;
                    down = Position((move.row) + wordrange + k, move.column);
                }
            }
            // points are multiplied according to word multipliers and the
            // word is pushed back into the vector of words
            if (word.size() > 1) {
                wordResult.push_back(word);
                points *= word_mult;
            } else {
                points = 0;
            }
            word.clear();

            letter_mult = 0;
            index = 0;
            wordrange = move.tiles.size();
            // iterates through the range of the new word to check if new words
            // are created right or left of the word
            for (unsigned int j = move.row; j < (move.row) + wordrange; j++) {
                int k = 1;
                int indiv_points = 0;
                word_mult = 1;
                current = Position(j, move.column);
                Position left = Position(j, move.column - k);
                Position right = Position(j, move.column + 1);
                // adjacency check for letters right or left of the current tile
                if (this->in_bounds_and_has_tile(left)) {
                    adjacent = true;
                }
                if (this->in_bounds_and_has_tile(right)) {
                    adjacent = true;
                }
                if (this->in_bounds_and_has_tile(left) && !this->in_bounds_and_has_tile(right)) {
                    if (!this->at(current).has_tile()) {
                        // if there is a placed letter to the left, we iterate left while the squares
                        // have tiles and are in bounds
                        while (this->in_bounds_and_has_tile(left)) {
                            k++;
                            left = Position(j, move.column - k);
                            if (this->in_bounds_and_has_tile(left)) {
                                continue;
                            } else {
                                k--;
                                left = Position(j, move.column - k);
                                break;
                            }
                        }
                        // the points and letters of the left tiles are evaluated
                        while (this->in_bounds_and_has_tile(left)) {
                            // check if the tile has a question mark
                            if (this->at(left).get_tile_kind().letter == '?') {
                                points += this->at(left).get_tile_kind().points;
                                word.push_back(this->at(left).get_tile_kind().assigned);
                            } else {
                                indiv_points += this->at(left).get_tile_kind().points;
                                word.push_back(this->at(left).get_tile_kind().letter);
                            }
                            k--;
                            left = Position(j, move.column - k);
                        }
                        // checks letter and word multpliers and evaluates the current tile
                        // once more
                        letter_mult = this->at(current).letter_multiplier;
                        word_mult *= this->at(current).word_multiplier;
                        if (move.tiles[index].letter == '?') {
                            word.push_back(move.tiles[index].assigned);
                            indiv_points += move.tiles[index].points;
                        } else {
                            indiv_points += move.tiles[index].points * letter_mult;
                            word.push_back(move.tiles[index].letter);
                        }
                        points += indiv_points * word_mult;
                        wordResult.push_back(word);
                        word.clear();
                    } else {
                        wordrange++;
                        continue;
                    }
                    index++;
                }
                // checks for a tile right of the current square
                else if (this->in_bounds_and_has_tile(right) && !this->in_bounds_and_has_tile(left)) {
                    k = 0;
                    Position right = Position(j, move.column + 1 + k);
                    if (!this->at(current).has_tile()) {
                        // evalutes the current tile before iterating through the tiles to the right
                        letter_mult = this->at(current).letter_multiplier;
                        word_mult *= this->at(current).word_multiplier;
                        if (move.tiles[index].letter == '?') {
                            word.push_back(move.tiles[index].assigned);
                            indiv_points += move.tiles[index].points;
                        } else {
                            indiv_points += move.tiles[index].points * letter_mult;
                            word.push_back(move.tiles[index].letter);
                        }
                        // evalutes the tiles to the right while the position is in bounds and has a tile
                        while (this->in_bounds_and_has_tile(right)) {
                            if (this->at(right).get_tile_kind().letter == '?') {
                                points += this->at(right).get_tile_kind().points;
                                word.push_back(this->at(right).get_tile_kind().assigned);
                            } else {
                                indiv_points += this->at(right).get_tile_kind().points;
                                word.push_back(this->at(right).get_tile_kind().letter);
                            }
                            k++;
                            right = Position(j, move.column + 1 + k);
                        }
                        points += indiv_points * word_mult;
                        wordResult.push_back(word);
                        word.clear();
                    } else {
                        wordrange++;
                        continue;
                    }
                    index++;
                } else if (this->in_bounds_and_has_tile(current)) {
                    wordrange++;
                } else if (
                        this->in_bounds_and_has_tile(left) && this->in_bounds_and_has_tile(right)
                        && !this->at(current).has_tile()) {
                    while (this->in_bounds_and_has_tile(left)) {
                        k++;
                        left = Position(j, move.column - k);
                        if (this->in_bounds_and_has_tile(left)) {
                            continue;
                        } else {
                            k--;
                            left = Position(j, move.column - k);
                            break;
                        }
                    }
                    while (this->in_bounds_and_has_tile(left)) {
                        // check if the tile has a question mark
                        if (this->at(left).get_tile_kind().letter == '?') {
                            points += this->at(left).get_tile_kind().points;
                            word.push_back(this->at(left).get_tile_kind().assigned);
                        } else {
                            indiv_points += this->at(left).get_tile_kind().points;
                            word.push_back(this->at(left).get_tile_kind().letter);
                        }
                        k--;
                        left = Position(j, move.column - k);
                    }
                    letter_mult = this->at(current).letter_multiplier;
                    word_mult *= this->at(current).word_multiplier;
                    if (move.tiles[index].letter == '?') {
                        word.push_back(move.tiles[index].assigned);
                        indiv_points += move.tiles[index].points;
                    } else {
                        indiv_points += move.tiles[index].points * letter_mult;
                        word.push_back(move.tiles[index].letter);
                    }
                    while (this->in_bounds_and_has_tile(right)) {
                        if (this->at(right).get_tile_kind().letter == '?') {
                            points += this->at(right).get_tile_kind().points;
                            word.push_back(this->at(right).get_tile_kind().assigned);
                        } else {
                            indiv_points += this->at(right).get_tile_kind().points;
                            word.push_back(this->at(right).get_tile_kind().letter);
                        }
                        k++;
                        right = Position(j, move.column + 1 + k);
                    }
                    points += indiv_points * word_mult;
                    wordResult.push_back(word);
                    word.clear();
                    index++;
                } else {
                    index++;
                }
            }
            // if there are no adjacent tiles, an error is returned
            if (adjacent == false) {
                string error = "incorrect word placement";
                return PlaceResult(error);
            }
            return PlaceResult(wordResult, points);
        }
    }
}

PlaceResult Board::place(const Move& move) {
    // TODO: Complete implementation here
    PlaceResult result = test_place(move);
    int wordrange = move.tiles.size();
    int index = 0;
    // changes the boardsquare when placing a tile
    if (move.direction == Direction::ACROSS) {
        for (unsigned int j = move.column; j < (move.column) + wordrange; j++) {
            Position current = Position(move.row, j);
            if (!this->is_in_bounds(current)) {
                string error = "out of bounds";
                return PlaceResult(error);
            }
            // iterates over tiles already on the board
            else if (this->at(current).has_tile()) {
                wordrange++;
            }
            // sets the tile kind based on the given letter
            else {
                this->at(current).set_tile_kind(move.tiles[index]);
                index++;
            }
        }
    }
    // same implementation as above, but for the downward direction
    else if (move.direction == Direction::DOWN) {
        for (unsigned int j = move.row; j < (move.row) + wordrange; j++) {
            Position current = Position(j, move.column);
            if (!this->is_in_bounds(current)) {
                string error = "out of bounds";
                return PlaceResult(error);
            } else if (this->at(current).has_tile()) {
                wordrange++;
            } else {
                this->at(current).set_tile_kind(move.tiles[index]);
                index++;
            }
        }
    } else {
        Position current = Position(move.row, move.column);
        this->at(current).set_tile_kind(move.tiles[index]);
    }
    return result;
}
// returns the letter at a given position
char Board::letter_at(Position p) const {
    if (this->at(p).get_tile_kind().letter == TileKind::BLANK_LETTER) {
        return this->at(p).get_tile_kind().assigned;
    }
    return this->at(p).get_tile_kind().letter;
}

TileKind Board::getTileKind(Position p) const { return this->at(p).get_tile_kind(); }

bool Board::is_anchor_spot(Position p) const {
    // returns true if the start location is vacant and p is the start position
    if (!this->at(start).has_tile() && p == start) {
        return true;
    }
    // returns true if any positions adjacent to the position have a tile
    // Position p must be vacant
    if (this->is_in_bounds(p) && !this->at(p).has_tile()) {
        Position down = Position(p.row + 1, p.column);
        Position up = Position(p.row - 1, p.column);
        Position left = Position(p.row, p.column - 1);
        Position right = Position(p.row, p.column + 1);
        if (this->in_bounds_and_has_tile(down) || this->in_bounds_and_has_tile(up) || this->in_bounds_and_has_tile(left)
            || this->in_bounds_and_has_tile(right)) {
            return true;
        }
    }
    return false;
}
vector<Board::Anchor> Board::get_anchors() const {
    vector<Anchor> anchors;
    // iterates from the top left corner of the board to the bottom right corner
    for (size_t i = 0; i < this->rows; i++) {
        for (size_t j = 0; j < this->columns; j++) {
            Position p = Position(i, j);
            // code executes if position p is an anchor spot
            if (is_anchor_spot(p)) {
                int k = 1;
                Position left = Position(i, j - k);
                size_t anchorLimitLeft = 0;
                // iterates left to determine the anchors left limit
                while (this->is_in_bounds(left) && !is_anchor_spot(left) && !this->at(left).has_tile()) {
                    anchorLimitLeft++;
                    k++;
                    left = Position(i, j - k);
                }
                // pushes the anchor into the vector of anchors
                anchors.push_back(Anchor(p, Direction::ACROSS, anchorLimitLeft));
                k = 1;
                Position up = Position(i - k, j);
                size_t anchorLimitUp = 0;
                // iterates up to determine the anchors up limit
                while (this->is_in_bounds(up) && !is_anchor_spot(up) && !this->at(up).has_tile()) {
                    anchorLimitUp++;
                    k++;
                    up = Position(i - k, j);
                }
                // pushes the anchor into the vector of anchors
                anchors.push_back(Anchor(p, Direction::DOWN, anchorLimitUp));
            }
        }
    }
    return anchors;
}

// The rest of this file is provided for you. No need to make changes.

BoardSquare& Board::at(const Board::Position& position) { return this->squares.at(position.row).at(position.column); }

const BoardSquare& Board::at(const Board::Position& position) const {
    return this->squares.at(position.row).at(position.column);
}

bool Board::is_in_bounds(const Board::Position& position) const {
    return position.row < this->rows && position.column < this->columns;
}

bool Board::in_bounds_and_has_tile(const Position& position) const {
    return is_in_bounds(position) && at(position).has_tile();
}

void Board::print(ostream& out) const {
    // Draw horizontal number labels
    for (size_t i = 0; i < BOARD_TOP_MARGIN - 2; ++i) {
        out << std::endl;
    }
    out << FG_COLOR_LABEL << repeat(SPACE, BOARD_LEFT_MARGIN);
    const size_t right_number_space = (SQUARE_OUTER_WIDTH - 3) / 2;
    const size_t left_number_space = (SQUARE_OUTER_WIDTH - 3) - right_number_space;
    for (size_t column = 0; column < this->columns; ++column) {
        out << repeat(SPACE, left_number_space) << std::setw(2) << column + 1 << repeat(SPACE, right_number_space);
    }
    out << std::endl;

    // Draw top line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_TOP_LEFT, T_DOWN, L_TOP_RIGHT, out);
    out << endl;

    // Draw inner board
    for (size_t row = 0; row < this->rows; ++row) {
        if (row > 0) {
            out << repeat(SPACE, BOARD_LEFT_MARGIN);
            print_horizontal(this->columns, T_RIGHT, PLUS, T_LEFT, out);
            out << endl;
        }

        // Draw insides of squares
        for (size_t line = 0; line < SQUARE_INNER_HEIGHT; ++line) {
            out << FG_COLOR_LABEL << BG_COLOR_OUTSIDE_BOARD;

            // Output column number of left padding
            if (line == 1) {
                out << repeat(SPACE, BOARD_LEFT_MARGIN - 3);
                out << std::setw(2) << row + 1;
                out << SPACE;
            } else {
                out << repeat(SPACE, BOARD_LEFT_MARGIN);
            }

            // Iterate columns
            for (size_t column = 0; column < this->columns; ++column) {
                out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL;
                const BoardSquare& square = this->squares.at(row).at(column);
                bool is_start = this->start.row == row && this->start.column == column;

                // Figure out background color
                if (square.word_multiplier == 2) {
                    out << BG_COLOR_WORD_MULTIPLIER_2X;
                } else if (square.word_multiplier == 3) {
                    out << BG_COLOR_WORD_MULTIPLIER_3X;
                } else if (square.letter_multiplier == 2) {
                    out << BG_COLOR_LETTER_MULTIPLIER_2X;
                } else if (square.letter_multiplier == 3) {
                    out << BG_COLOR_LETTER_MULTIPLIER_3X;
                } else if (is_start) {
                    out << BG_COLOR_START_SQUARE;
                }

                // Text
                if (line == 0 && is_start) {
                    out << "  \u2605  ";
                } else if (line == 0 && square.word_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'W' << std::setw(1)
                        << square.word_multiplier;
                } else if (line == 0 && square.letter_multiplier > 1) {
                    out << FG_COLOR_MULTIPLIER << repeat(SPACE, SQUARE_INNER_WIDTH - 2) << 'L' << std::setw(1)
                        << square.letter_multiplier;
                } else if (line == 1 && square.has_tile()) {
                    char l = square.get_tile_kind().letter == TileKind::BLANK_LETTER ? square.get_tile_kind().assigned
                                                                                     : ' ';
                    out << repeat(SPACE, 2) << FG_COLOR_LETTER << square.get_tile_kind().letter << l
                        << repeat(SPACE, 1);
                } else if (line == SQUARE_INNER_HEIGHT - 1 && square.has_tile()) {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH - 1) << FG_COLOR_SCORE << square.get_points();
                } else {
                    out << repeat(SPACE, SQUARE_INNER_WIDTH);
                }
            }

            // Add vertical line
            out << FG_COLOR_LINE << BG_COLOR_NORMAL_SQUARE << I_VERTICAL << BG_COLOR_OUTSIDE_BOARD << std::endl;
        }
    }

    // Draw bottom line
    out << repeat(SPACE, BOARD_LEFT_MARGIN);
    print_horizontal(this->columns, L_BOTTOM_LEFT, T_UP, L_BOTTOM_RIGHT, out);
    out << endl << rang::style::reset << std::endl;
}
