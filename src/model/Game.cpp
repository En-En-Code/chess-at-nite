/*
 * This file is part of the chess-at-nite project.
 *
 * Copyright (c) 2009-2010 by
 *   Franziskus Domig
 *   Panayiotis Lipiridis
 *   Radoslav Petrik
 *   Thai Gia Tuong
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "Game.h"

using namespace std;

Game::Game(Board* board, Player* new_white_player, Player* new_black_player,
        bool both_human) :
board(board), white_player(new_white_player), black_player(new_black_player),
both_human(both_human) {
    move_generator = new MoveGenerator(board);
    white_player->set_board(board);
    black_player->set_board(board);
}

Game::Game(const Game& game) {
    board = game.board;
    white_player = game.white_player;
    black_player = game.black_player;
    move_generator = game.move_generator;
    both_human = game.both_human;
    game_over = game.game_over;
}

Game::~Game() {
    delete move_generator;
}

void Game::start_game() {
    game_over = false;
    board->number_of_moves = 0;
    if (board->to_move == WHITE) {
        play(white_player, black_player);
    } else {
        play(black_player, white_player);
    }

    int wait = get_ms() + 2400;
    while (get_ms() < wait);
    print_history(board->history);
}

/*
 * Recursive play :)
 */
void Game::play(Player* player1, Player* player2) {
    string color;
    int status;
    bool success = true;
    while (!game_over) {
        color = board -> to_move == WHITE ? "White" : "Black";
        status = update_board_status(board);

        board -> set_status(status);

        //if the move (last iteration) was successfull or the undo was valid 
        if (success) {
            cout << *board << endl;
        }

#ifdef DEBUG
        cout << "threefold repititions: " << repetitions(board) << endl;
#endif
        //what to do in different cases
        switch (status) {
            case STATUS_WHITE_CHECKMATE:
            case STATUS_BLACK_CHECKMATE:
            case STATUS_WHITE_WINS:
            case STATUS_BLACK_WINS:
            case STATUS_STALEMATE:
            case STATUS_DRAW:
                game_over = true;
                break;
        }

        if (game_over) {
            break;
        }

        cout << color << "'s move " << "[" << board->number_of_moves / 2 + 1 << "]...";
        move next_move = player1->get_move();

        switch (next_move.special) {
            case MOVE_UNDO:
                success = board->undo_move();
                //that means that you have to undo 2 moves in order to give
                //the chance for the human player to move again..
                if (!both_human) {
                    success = board->undo_move();
                }

                if (success) {
                    cout << *board << endl;
                } else {
                    cout << "There is nothing in the history to undo.." << endl;
                }
                continue;
            case MOVE_RESIGN:
                status = board->to_move == WHITE ? STATUS_BLACK_WINS : STATUS_WHITE_WINS;
                board->set_status(status);

                if (status == STATUS_BLACK_WINS) {
                    cout << "White ";
                } else {
                    cout << "Black ";
                }
                cout << "resigned... GG" << endl;
                game_over = true;
                break;
        }

        if (!game_over) {
            if (next_move.moved_piece == EMPTY) {
                cerr << "If you can read this.. Something went terribly wrong :(" << endl;
                cerr << "Game tried to make an empty move.. we have to end the game.. sorry!" << endl;
                game_over = true;
                break;
            }

            board->add_pgn(move_to_algebraic(next_move, *board));
            board->play_move(next_move);
            success = true;
            play(player2, player1);
        }
    }
}
