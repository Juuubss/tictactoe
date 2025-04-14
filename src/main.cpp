#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <cstdlib>
#include <ctime>

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Estado do jogo
    char winner; // Vencedor do jogo

public:
    TicTacToe() {
        // Inicializar o tabuleiro e as variáveis do jogo
        current_player = 'X'; // Jogador inicial
        winner = ' '; // Sem vencedor ainda
        game_over = false; // Jogo não terminou

        for (auto& row : board) {
            row.fill(' '); // Inicializa o tabuleiro vazio
        }
    }

    void display_board() {
        // Exibir o tabuleiro no console
        std::cout << "\nTabuleiro atual:\n";
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                std::cout << " " << board[i][j] << " ";
                if (j < 2) {
                    std::cout << "|"; // Separador de colunas
                }
            }
            std::cout << std::endl;
            if (i < 2) {
                std::cout << "---+---+---" << std::endl; // Separador de linhas
            }
        }
        std::cout << "Jogador atual: " << current_player << "\n";
    }

    bool make_move(char player, int row, int col) {
        std::unique_lock<std::mutex> lock(board_mutex);

        // Espera até ser a vez do jogador ou o jogo terminar
        turn_cv.wait(lock, [this, player]() {
            return current_player == player || game_over;
        });

        if (game_over || row < 0 || row > 2 || col < 0 || col > 2 || board[row][col] != ' ') {
            return false; // Jogada inválida
        }

        // Faz a jogada
        board[row][col] = player;
        display_board();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Pausa para visualização

        // Verifica se o jogo terminou
        if (check_win(player)) {
            game_over = true;
            winner = player;
            turn_cv.notify_all();
            return true;
        }

        if (check_draw()) {
            game_over = true;
            winner = 'D';
            turn_cv.notify_all();
            return true;
        }

        // Alterna o jogador
        current_player = (player == 'X') ? 'O' : 'X';
        turn_cv.notify_all();
        return true;
    }

    bool check_win(char player) {
        // Verifica linhas
        for (int i = 0; i < 3; i++) {
            if (board[i][0] == player && board[i][1] == player && board[i][2] == player)
                return true;
        }
        // Verifica colunas
        for (int j = 0; j < 3; j++) {
            if (board[0][j] == player && board[1][j] == player && board[2][j] == player)
                return true;
        }
        // Verifica diagonais
        if ((board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
            (board[0][2] == player && board[1][1] == player && board[2][0] == player)) {
            return true;
        }
        return false;
    }

    bool check_draw() {
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == ' ') {
                    return false;
                }
            }
        }
        return true;
    }

    bool is_game_over() {
        return game_over;
    }

    char get_winner() {
        return winner;
    }

    char get_current_player() const {
        return current_player;
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador

public:
    Player(TicTacToe& g, char s, std::string strat)
        : game(g), symbol(s), strategy(strat) {}

    void play() {
        while (!game.is_game_over()) {
            if (game.get_current_player() == symbol) {
                if (strategy == "sequential") {
                    play_sequential();
                } else if (strategy == "random") {
                    play_random();
                }
            }
        }
    }

private:
    void play_sequential() {
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                if (game.make_move(symbol, row, col)) {
                    return; // Sai após jogar
                }
            }
        }
    }

    void play_random() {
        int row, col;
        std::srand(std::time(0));
        do {
            row = std::rand() % 3;
            col = std::rand() % 3;
        } while (!game.make_move(symbol, row, col));
    }
};

// Função principal
int main() {
    TicTacToe game;

    Player player1(game, 'X', "sequential");
    Player player2(game, 'O', "random");

    std::cout << "Iniciando o jogo...\n";

    std::thread thread1(&Player::play, &player1);
    std::thread thread2(&Player::play, &player2);

    thread1.join();
    thread2.join();

    char winner = game.get_winner();
    if (winner == 'D') {
        std::cout << "O jogo terminou em empate!\n";
    } else {
        std::cout << "O vencedor é o jogador: " << winner << "!\n";
    }

    return 0;
}
