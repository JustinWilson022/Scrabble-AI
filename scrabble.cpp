#include "scrabble.h"

#include "formatting.h"
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

using namespace std;

// Given to you. this does not need to be changed
Scrabble::Scrabble(const ScrabbleConfig& config)
        : hand_size(config.hand_size),
          minimum_word_length(config.minimum_word_length),
          tile_bag(TileBag::read(config.tile_bag_file_path, config.seed)),
          board(Board::read(config.board_file_path)),
          dictionary(Dictionary::read(config.dictionary_file_path)),
          numHumanPlayers(0) {}

void Scrabble::add_players() {
    bool valid = false;
    while (!valid) {
        // the try/catch block makes sure the player enters a valid number of players
        try {
            cout << "How many players (1-8) are playing?" << endl;
            string NumPlayers;
            getline(cin, NumPlayers);
            stringstream ss(NumPlayers);
            int numPlayers;
            ss >> numPlayers;
            // exception is thrown if the number entered is invalid
            if (numPlayers < 1 || numPlayers > 8) {
                throw CommandException("invalid number of players");
            }
            valid = true;
            // the user enters the name of each player
            for (int i = 0; i < numPlayers; i++) {
                string name;
                string answer;
                cout << "What is the name of player " << i + 1 << " ?" << endl;
                getline(cin, name);
                cout << "Is this player a computer? (y/n)" << endl;
                getline(cin, answer);
                if (answer == "y") {
                    shared_ptr<ComputerPlayer> computer = make_shared<ComputerPlayer>(name, hand_size);
                    players.push_back(computer);
                } else {
                    shared_ptr<HumanPlayer> scrabbler = make_shared<HumanPlayer>(name, hand_size);
                    players.push_back(scrabbler);
                    numHumanPlayers++;
                }
            }
            // each user gets 7 random tiles from the tile bag to start the game
            for (int i = 0; i < numPlayers; i++) {
                vector<TileKind> letters = tile_bag.remove_random_tiles(7);
                players[i]->add_tiles(letters);
            }
        } catch (...) {
            cout << "please enter a valid number of players: " << endl;
        }
    }
}
// Game Loop should cycle through players and get and execute that players move
// until the game is over.
void Scrabble::game_loop() {
    // TODO: implement this.
    bool game_on = true;
    bool tileBagEmpty = false;
    unsigned int passes = 0;
    // while loop runs the duration of the game
    while (game_on) {
        for (unsigned int i = 0; i < players.size(); i++) {
            cout << "Player " << i + 1 << ", it is your turn" << endl;
            cout << "Your current score: " << SCORE_COLOR << players[i]->get_points() << rang::style::reset << endl;
            // the user enters the move of the current player
            Move playerMove = players[i]->get_move(board, dictionary);
            if (playerMove.kind == MoveKind::PLACE) {
                passes = 0;
                // the players points are increased based on the result of the move
                PlaceResult outcome = board.place(playerMove);
                unsigned int newPoints = 0;
                newPoints = outcome.points;
                // if the player uses all 7 when placing a word, they get 50 extra points
                if (playerMove.tiles.size() == 7) {
                    newPoints += 50;
                }
                players[i]->add_points(newPoints);
                cout << "You gained " << SCORE_COLOR << newPoints << rang::style::reset << " points!" << endl;
                // the tiles used during the turn are removed from their hand
                players[i]->remove_tiles(playerMove.tiles);
                if (tileBagEmpty) {
                    // check if the player is out of tiles
                    if (players[i]->count_tiles() == 0) {
                        game_on = false;
                        break;
                    }
                }
                // player recieves the rest of the tiles in the tile bag if the number of tiles they
                // used on a single turn is greater than the number of tiles left in the bag
                else if (tile_bag.count_tiles() < playerMove.tiles.size()) {
                    vector<TileKind> letters = tile_bag.remove_random_tiles(tile_bag.count_tiles());
                    players[i]->add_tiles(letters);
                    tileBagEmpty = true;
                }
                // otherwise, the player will recieve the same number of tiles they used
                else {
                    vector<TileKind> letters = tile_bag.remove_random_tiles(playerMove.tiles.size());
                    players[i]->add_tiles(letters);
                }
                cin.ignore();
                cout << endl << "Press [enter] to continue.";
                cout << endl;

            } else if (playerMove.kind == MoveKind::EXCHANGE) {
                passes = 0;
                // the tiles being exchanged are added back to the tile bag
                for (unsigned int i = 0; i < playerMove.tiles.size(); i++) {
                    tile_bag.add_tile(playerMove.tiles[i]);
                }
                // the tiles being exchanged are removed from the player's hand and the player
                // recieves new tiles from the tile bag
                players[i]->remove_tiles(playerMove.tiles);
                vector<TileKind> letters = tile_bag.remove_random_tiles(playerMove.tiles.size());
                players[i]->add_tiles(letters);
                cout << "Here are your new tiles: " << endl;
                for (unsigned int i = 0; i < letters.size(); i++) {
                    cout << letters[i].letter << " ";
                }
                cout << endl;
                cin.ignore();
                cout << endl << "Press [enter] to continue.";
                cout << endl;
            } else {
                // the passes variable is incremented when a player passes
                if (players[i]->is_human() == true) {
                    ++passes;
                }
                cout << "player " << i + 1 << " decided to pass :(" << endl;
                cin.ignore();
                cout << endl << "Press [enter] to continue.";
                cout << endl;
            }
            // if each player chooses pass in consecutive turns, the game ends
            if (passes == numHumanPlayers) {
                game_on = false;
            }
        }
    }
    // Useful cout expressions with fancy colors. Expressions in curly braces, indicate values you supply.
    // cout << "You gained " << SCORE_COLOR << {points} << rang::style::reset << " points!" << endl;
    // cout << "Your current score: " << SCORE_COLOR << {points} << rang::style::reset << endl;
    // cout << endl << "Press [enter] to continue.";
}

// Performs final score subtraction. Players lose points for each tile in their
// hand. The player who cleared their hand receives all the points lost by the
// other players.
void Scrabble::final_subtraction(vector<shared_ptr<Player>>& plrs) {
    // TODO: implement this method.
    // Do not change the method signature.
    int total = 0;
    for (unsigned int i = 0; i < plrs.size(); i++) {
        // if the player has a hand value of zero when the game ends, they will add to
        // their score the total of the other player's hand values
        if (plrs[i]->get_hand_value() == 0) {
            for (unsigned int j = i + 1; j < plrs.size(); j++) {
                int handVal = 0;
                handVal = plrs[j]->get_hand_value();
                int playerPoints = plrs[j]->get_points();
                if (playerPoints - handVal <= 0) {
                    plrs[j]->subtract_points(plrs[j]->get_points());
                    total += handVal;
                } else {
                    plrs[j]->subtract_points(handVal);
                    total += handVal;
                }
            }
            // the player adds the total of all others' hand value
            plrs[i]->add_points(total);
            return;
        } else {
            // the player's subtract the value of their tiles from their total score
            int handVal = 0;
            handVal = plrs[i]->get_hand_value();
            int playerPoints = plrs[i]->get_points();
            // minimum number of points at the end of the game is zero
            if (playerPoints - handVal <= 0) {
                plrs[i]->subtract_points(plrs[i]->get_points());
                total += handVal;
            } else {
                plrs[i]->subtract_points(handVal);
                total += handVal;  // keeps the total of all hand values combined
            }
        }
    }
}

// You should not need to change this function.
void Scrabble::print_result() {
    // Determine highest score
    size_t max_points = 0;
    for (auto player : this->players) {
        if (player->get_points() > max_points) {
            max_points = player->get_points();
        }
    }

    // Determine the winner(s) indexes
    vector<shared_ptr<Player>> winners;
    for (auto player : this->players) {
        if (player->get_points() >= max_points) {
            winners.push_back(player);
        }
    }

    cout << (winners.size() == 1 ? "Winner:" : "Winners: ");
    for (auto player : winners) {
        cout << SPACE << PLAYER_NAME_COLOR << player->get_name();
    }
    cout << rang::style::reset << endl;

    // now print score table
    cout << "Scores: " << endl;
    cout << "---------------------------------" << endl;

    // Justify all integers printed to have the same amount of character as the high score, left-padding with spaces
    cout << setw(static_cast<uint32_t>(floor(log10(max_points) + 1)));

    for (auto player : this->players) {
        cout << SCORE_COLOR << player->get_points() << rang::style::reset << " | " << PLAYER_NAME_COLOR
             << player->get_name() << rang::style::reset << endl;
    }
}

// You should not need to change this.
void Scrabble::main() {
    add_players();
    game_loop();
    final_subtraction(this->players);
    print_result();
}
