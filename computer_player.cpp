
#include "computer_player.h"

#include <iostream>
#include <memory>
#include <string>
using namespace std;

void ComputerPlayer::left_part(
        Board::Position anchor_pos,
        std::string partial_word,
        Move partial_move,
        std::shared_ptr<Dictionary::TrieNode> node,
        size_t limit,
        TileCollection& remaining_tiles,
        std::vector<Move>& legal_moves,
        const Board& board) const {
    // extend right is called to find the postfix of the partial word
    extend_right(anchor_pos, partial_word, partial_move, node, remaining_tiles, legal_moves, board);
    // base case is hit when the size of the word is equal to the limit
    if (partial_word.size() == limit || partial_move.column == 0 || partial_move.row == 0) {
        return;
    }
    // base case if the dictionary node is nullptr
    if (node == nullptr) {
        return;
    }
    // copy of the remaining tiles is made
    TileCollection remaining = remaining_tiles;
    TileCollection::const_iterator it(remaining_tiles.cbegin());
    // loop iterates through the computer's remaining tiles
    for (; it != remaining_tiles.cend(); it++) {
        // decrements the column or row based on the move direction
        if (partial_move.direction == Direction::ACROSS) {
            partial_move.column--;
        } else if (partial_move.direction == Direction::DOWN) {
            partial_move.row--;
        }
        // case if the current letter is a blank tile
        if (it->letter == TileKind::BLANK_LETTER) {
            remaining.remove_tile(*it);
            // itereate through all possibilities of the blank tile
            std::map<char, std::shared_ptr<Dictionary::TrieNode>>::iterator new_it;
            for (new_it = node->nexts.begin(); new_it != node->nexts.end(); new_it++) {
                // new_it->first will take on the assigned value of the blank tile
                partial_word.push_back(new_it->first);
                partial_move.tiles.push_back(TileKind(TileKind::BLANK_LETTER, it->points, new_it->first));
                // left part is called with the newly assigned blank tile
                left_part(
                        anchor_pos,
                        partial_word,
                        partial_move,
                        node->nexts[new_it->first],
                        limit,
                        remaining,
                        legal_moves,
                        board);
                // once leftpart returns, the letter is removed from the word and move
                partial_word.pop_back();
                partial_move.tiles.pop_back();
            }
            // blank tile is added back to the remaining tiles
            remaining.add_tile(*it);
        } else {
            // code executes if the letter is in the node->nexts map
            if (node->nexts.find(it->letter) != node->nexts.end()) {
                remaining.remove_tile(*it);
                // word and move push back the letter
                partial_word.push_back(it->letter);
                partial_move.tiles.push_back(*it);
                left_part(
                        anchor_pos,
                        partial_word,
                        partial_move,
                        node->nexts[it->letter],
                        limit,
                        remaining,
                        legal_moves,
                        board);
                // when left part returns, the letter is removed from the partial word and move
                partial_word.pop_back();
                partial_move.tiles.pop_back();
                // tile is added back to the remaining tiles
                remaining.add_tile(*it);
            }
        }
        // the position if the partial move is adjusted accordingly
        if (partial_move.direction == Direction::ACROSS) {
            partial_move.column++;
        } else if (partial_move.direction == Direction::DOWN) {
            partial_move.row++;
        }
    }
}

void ComputerPlayer::extend_right(
        Board::Position square,
        std::string partial_word,
        Move partial_move,
        std::shared_ptr<Dictionary::TrieNode> node,
        TileCollection& remaining_tiles,
        std::vector<Move>& legal_moves,
        const Board& board) const {
    // base case if the dictionary node is nullptr
    if (node == nullptr) {
        return;
    }
    // if the node is for a valid word in the dictionary, the move is push into legal moves
    if (node->is_final) {
        legal_moves.push_back(partial_move);
    }
    // returns if the square goes out of bounds
    if (!board.is_in_bounds(square)) {
        return;
    }
    // returns if there are no remaining tiles to create words with
    if (remaining_tiles.count_tiles() == 0) {
        return;
    }

    // code executes if the position on the board does not have a square
    if (!board.in_bounds_and_has_tile(square)) {
        // copy of the computer's remaining tiles is made
        TileCollection remaining = remaining_tiles;
        TileCollection::const_iterator it(remaining_tiles.cbegin());
        // iterate through the remaining tiles in the computer's hand
        for (; it != remaining_tiles.cend(); it++) {
            // case of the current letter is a blank tile
            if (it->letter == TileKind::BLANK_LETTER) {
                // blank tile is removed from the hand
                remaining.remove_tile(*it);
                std::map<char, std::shared_ptr<Dictionary::TrieNode>>::iterator new_it;
                // iterate through each possible letter that the blank tile can take on
                for (new_it = node->nexts.begin(); new_it != node->nexts.end(); new_it++) {
                    partial_word.push_back(new_it->first);
                    partial_move.tiles.push_back(TileKind(it->letter, it->points, new_it->first));
                    // extend in the direction of the move
                    if (partial_move.direction == Direction::ACROSS) {
                        square.column++;
                        extend_right(
                                square,
                                partial_word,
                                partial_move,
                                node->nexts[new_it->first],
                                remaining,
                                legal_moves,
                                board);
                        square.column--;
                    } else if (partial_move.direction == Direction::DOWN) {
                        square.row++;
                        extend_right(
                                square,
                                partial_word,
                                partial_move,
                                node->nexts[new_it->first],
                                remaining,
                                legal_moves,
                                board);
                        square.row--;
                    }
                    // when extend right returns, the letter is removed from the word and move
                    partial_word.pop_back();
                    partial_move.tiles.pop_back();
                }
                // blank tile is added back to the remaining tiles
                remaining.add_tile(*it);
            } else {
                // code executes if the letter is in the node->nexts map
                if (node->nexts.find(it->letter) != node->nexts.end()) {
                    // removes letter from the hand and pushes the letter to the word and move
                    remaining.remove_tile(*it);
                    partial_word.push_back(it->letter);
                    partial_move.tiles.push_back(*it);
                    // extends in the direction of the move
                    if (partial_move.direction == Direction::ACROSS) {
                        square.column++;
                        extend_right(
                                square,
                                partial_word,
                                partial_move,
                                node->nexts[it->letter],
                                remaining,
                                legal_moves,
                                board);
                        square.column--;
                    } else if (partial_move.direction == Direction::DOWN) {
                        square.row++;
                        extend_right(
                                square,
                                partial_word,
                                partial_move,
                                node->nexts[it->letter],
                                remaining,
                                legal_moves,
                                board);
                        square.row--;
                    }
                    // when extend right returns, the letter is removed from the word and move
                    partial_word.pop_back();
                    partial_move.tiles.pop_back();
                    // letter is added back to the hand
                    remaining.add_tile(*it);
                }
            }
        }
    }
    // case if there is a tile at the current position
    else if (board.in_bounds_and_has_tile(square)) {
        char tile = board.letter_at(square);
        // code executes if the letter is in the node->nexts map
        if (node->nexts.find(tile) != node->nexts.end()) {
            // letter is pushed to the partial word
            partial_word.push_back(tile);
            // extend in the direction of the move
            if (partial_move.direction == Direction::ACROSS) {
                square.column++;
                extend_right(
                        square, partial_word, partial_move, node->nexts[tile], remaining_tiles, legal_moves, board);
            } else if (partial_move.direction == Direction::DOWN) {
                square.row++;
                extend_right(
                        square, partial_word, partial_move, node->nexts[tile], remaining_tiles, legal_moves, board);
            }
        }
    }
}
bool ComputerPlayer::is_human() const { return false; }

Move ComputerPlayer::get_move(const Board& board, const Dictionary& dictionary) const {
    std::vector<Move> legal_moves;
    std::vector<Board::Anchor> anchors = board.get_anchors();
    // iterates through the vector of anchors
    for (size_t i = 0; i < anchors.size(); i++) {
        TileCollection t;
        // adds the players tiles to the tile collection
        for (size_t i = 0; i < this->playerTiles.size(); i++) {
            t.add_tile(this->playerTiles[i]);
        }
        // left part is called on anchors with a limit greater than 0
        if (anchors[i].limit > 0) {
            Move move;
            // inititalizes the move based on the anchor location and direction
            move.row = anchors[i].position.row;
            move.column = anchors[i].position.column;
            move.direction = anchors[i].direction;
            left_part(anchors[i].position, "", move, dictionary.get_root(), anchors[i].limit, t, legal_moves, board);
        }
        // code for when the limit is equal to 0
        else {
            if (anchors[i].direction == Direction::ACROSS) {
                std::string word;
                // initializes the move based on the anchor position and direction
                Move move;
                move.row = anchors[i].position.row;
                move.column = anchors[i].position.column;
                move.direction = Direction::ACROSS;
                // iterates to the left and adds placed tiles to the partial move
                Board::Position left = anchors[i].position;
                left.column--;
                while (board.in_bounds_and_has_tile(left)) {
                    left.column--;
                }
                left.column++;
                while (left != anchors[i].position) {
                    word.push_back(board.letter_at(left));
                    left.column++;
                }
                extend_right(anchors[i].position, word, move, dictionary.find_prefix(word), t, legal_moves, board);
            } else if (anchors[i].direction == Direction::DOWN) {
                std::string word;
                // initializes the move based on the anchor position and direction
                Move move;
                move.row = anchors[i].position.row;
                move.column = anchors[i].position.column;
                move.direction = Direction::DOWN;
                // iterates up and adds placed tiles to the partial move
                Board::Position up = anchors[i].position;
                up.row--;
                while (board.in_bounds_and_has_tile(up)) {
                    up.row--;
                }
                up.row++;
                while (up != anchors[i].position) {
                    word.push_back(board.letter_at(up));
                    up.row++;
                }
                extend_right(anchors[i].position, word, move, dictionary.find_prefix(word), t, legal_moves, board);
            }
        }
    }
    return get_best_move(legal_moves, board, dictionary);
}

Move ComputerPlayer::get_best_move(
        std::vector<Move> legal_moves, const Board& board, const Dictionary& dictionary) const {
    // if there are no legal moves, the computer passes
    if (legal_moves.empty()) {
        cout << "legal move empty" << endl;
        Move move;
        move.kind = MoveKind::PASS;
        return move;
    }
    // this vector will contain legal moves that do not create invalid words on the board
    vector<Move> elite_legal_moves;
    for (size_t i = 0; i < legal_moves.size(); i++) {
        // invalid moves are caught with the try catch block
        try {
            Move move = legal_moves[i];
            move.kind = MoveKind::PLACE;
            PlaceResult boardCheck = board.test_place(move);
            // an exception is thrown if the PlaceResult is invalid
            if (!boardCheck.valid) {
                throw MoveException("boardcheck is false");
            }
            // checks if the words created are in the dictionary
            bool validWord = true;
            for (unsigned int i = 0; i < boardCheck.words.size(); i++) {
                if (!dictionary.is_word(boardCheck.words[i])) {
                    validWord = false;
                    break;
                }
            }
            // if no invalid words are created, the move is pushed to the new vector
            if (validWord) {
                elite_legal_moves.push_back(move);
            }
        } catch (...) {
            // continues if an invalid move comes up
        }
    }
    // the following code finds the move with the highest possible score
    Move highest = elite_legal_moves[0];
    PlaceResult highMove = board.test_place(highest);
    unsigned int max_points = highMove.points;
    if (highest.tiles.size() == get_hand_size()) {
        max_points += 50;
    }
    // iterates through the new vector of legal moves
    for (size_t i = 1; i < elite_legal_moves.size(); i++) {
        PlaceResult wordCheck = board.test_place(elite_legal_moves[i]);
        unsigned int current_points = wordCheck.points;
        // 50 points bonus if computer uses all tiles in their hand
        if (elite_legal_moves[i].tiles.size() == get_hand_size()) {
            current_points += 50;
        }
        // compares moves and sets "highest" to the move with the most points
        if (current_points > max_points) {
            highest = elite_legal_moves[i];
            max_points = current_points;
        }
    }
    // highest move is returned and played
    highest.kind = MoveKind::PLACE;
    return highest;
}
