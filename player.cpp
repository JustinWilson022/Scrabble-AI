#include "player.h"

#include "exceptions.h"

using namespace std;

// TODO: implement member functions

// Adds points to player's score
void Player::add_points(size_t points) { this->points += points; }

// Subtracts points from player's score
void Player::subtract_points(size_t points) { this->points -= points; }
// returns the number of points a player has
size_t Player::get_points() const { return this->points; }
// accesses the name of the player
const std::string& Player::get_name() const { return this->name; }

// Returns the number of tiles in a player's hand.
size_t Player::count_tiles() const {

    size_t count = 0;
    for (unsigned int i = 0; i < this->playerTiles.size(); i++) {
        count++;
    }
    return count;
}

// Removes tiles from player's hand.
void Player::remove_tiles(const std::vector<TileKind>& tiles) {

    int count = 0;
    for (unsigned int i = 0; i < tiles.size(); i++) {
        for (unsigned int j = 0; j < this->playerTiles.size(); j++) {
            if (tiles[i] == this->playerTiles[j]) {
                count++;
                this->playerTiles.erase(playerTiles.begin() + j);
                break;
            }
        }
    }
    if (count != tiles.size()) {
        throw MoveException("incorrect removal");
    }
}

// Adds tiles to player's hand.
void Player::add_tiles(const std::vector<TileKind>& tiles) {
    for (unsigned int i = 0; i < tiles.size(); i++) {
        this->playerTiles.push_back(tiles[i]);
    }
}

// Checks if player has a matching tile.
bool Player::has_tile(TileKind tile) {
    for (unsigned int i = 0; i < this->playerTiles.size(); i++) {
        if (this->playerTiles[i] == tile) {
            return true;
        }
    }
    return false;
}

// Returns the total points of all tiles in the players hand.
unsigned int Player::get_hand_value() const {
    unsigned int handVal = 0;
    for (unsigned int i = 0; i < this->playerTiles.size(); i++) {
        handVal += this->playerTiles[i].points;
    }
    return handVal;
}
// returns the hand size
size_t Player::get_hand_size() const { return this->hand_size; }
