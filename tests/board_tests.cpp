#include "kestrel/board.hpp"
#include "kestrel/movegen.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

using namespace kestrel;

void require(bool condition, const std::string &context) {
    if (!condition)
        throw std::runtime_error(context);
}

Board position(const std::string &fen) {
    Board board;
    require(board.setFromFen(fen), "Fixture rejected: " + fen);
    return board;
}

void roundTrip(Board &board, const Move &move, const std::string &context) {
    const auto original = board;
    const auto state = board.makeMove(move);
    require(board.sideToMove() == opposite(original.sideToMove()),
            context + ": side toggle");
    require(board.pieceAt(move.to).type ==
                (move.promotion == PieceType::None
                     ? original.pieceAt(move.from).type
                     : move.promotion),
            context + ": moving piece");
    if (move.isEnPassant)
        require(board.pieceAt(state.capturedSquare).empty(),
                context + ": EP capture");
    board.undoMove(move, state);
    require(board == original, context + ": full state restoration");
}

int main() {
    try {
        Board board;
        const auto original = board;
        const auto first = board.makeMove({makeSquare(4, 1), makeSquare(4, 3)});
        require(board.enPassantSquare() == makeSquare(4, 2),
                "Double push target");
        const auto second =
            board.makeMove({makeSquare(3, 6), makeSquare(3, 4)});
        const auto third = board.makeMove(
            {makeSquare(4, 3), makeSquare(3, 4), PieceType::None, true});
        require(board.halfmoveClock() == 0 && board.fullmoveNumber() == 2,
                "Pawn/capture clocks");
        board.undoMove(
            {makeSquare(4, 3), makeSquare(3, 4), PieceType::None, true}, third);
        board.undoMove({makeSquare(3, 6), makeSquare(3, 4)}, second);
        board.undoMove({makeSquare(4, 1), makeSquare(4, 3)}, first);
        require(board == original, "Nested capture restoration");

        for (Color color : {Color::White, Color::Black}) {
            const bool white = color == Color::White;
            auto capture =
                position(white ? "4k3/8/8/8/3p4/4P3/8/4K3 w - - 7 12"
                               : "4k3/8/4p3/3P4/8/8/8/4K3 b - - 7 12");
            roundTrip(capture,
                      {makeSquare(4, white ? 2 : 5),
                       makeSquare(3, white ? 3 : 4), PieceType::None, true},
                      "Ordinary capture");
            auto ep = position(white ? "4k3/8/8/3pP3/8/8/8/4K3 w - d6 0 12"
                                     : "4k3/8/8/8/3Pp3/8/8/4K3 b - d3 0 12");
            roundTrip(ep,
                      {makeSquare(4, white ? 4 : 3),
                       makeSquare(3, white ? 5 : 2), PieceType::None, true,
                       true},
                      "En passant");
            for (auto type : {PieceType::Queen, PieceType::Rook,
                              PieceType::Bishop, PieceType::Knight}) {
                auto promotion =
                    position(white ? "1r2k3/P7/8/8/8/8/8/4K3 w - - 4 12"
                                   : "4k3/8/8/8/8/8/p7/1R2K3 b - - 4 12");
                roundTrip(promotion,
                          {makeSquare(0, white ? 6 : 1),
                           makeSquare(0, white ? 7 : 0), type},
                          "Quiet promotion");
                roundTrip(promotion,
                          {makeSquare(0, white ? 6 : 1),
                           makeSquare(1, white ? 7 : 0), type, true},
                          "Capture promotion");
            }
            auto castle = position(std::string("r3k2r/8/8/8/8/8/8/R3K2R ") +
                                   (white ? "w" : "b") + " KQkq - 9 12");
            const int rank = white ? 0 : 7;
            for (int to : {2, 6}) {
                const auto before = castle;
                Move move{makeSquare(4, rank),
                          makeSquare(to, rank),
                          PieceType::None,
                          false,
                          false,
                          true};
                auto state = castle.makeMove(move);
                require(
                    castle.pieceAt(makeSquare(to == 6 ? 5 : 3, rank)).type ==
                        PieceType::Rook,
                    "Castling rook");
                require((castle.castlingRights() & (white ? 3 : 12)) == 0,
                        "King clears rights");
                require(castle.halfmoveClock() == 10 &&
                            castle.fullmoveNumber() == (white ? 12 : 13),
                        "Quiet clocks");
                castle.undoMove(move, state);
                require(castle == before, "Castle restoration");
            }
            for (int file : {0, 7}) {
                const auto before = castle;
                Move rook{makeSquare(file, rank),
                          makeSquare(file, white ? 1 : 6)};
                auto state = castle.makeMove(rook);
                const auto right =
                    white ? (file == 0 ? WhiteQueenSide : WhiteKingSide)
                          : (file == 0 ? BlackQueenSide : BlackKingSide);
                require((castle.castlingRights() & right) == 0,
                        "Rook clears its right");
                castle.undoMove(rook, state);
                require(castle == before, "Rook restoration");
                Move take{makeSquare(file, rank),
                          makeSquare(file, white ? 7 : 0), PieceType::None,
                          true};
                state = castle.makeMove(take);
                const auto enemyRight =
                    white ? (file == 0 ? BlackQueenSide : BlackKingSide)
                          : (file == 0 ? WhiteQueenSide : WhiteKingSide);
                require((castle.castlingRights() & (right | enemyRight)) == 0,
                        "Home rook capture rights");
                castle.undoMove(take, state);
                require(castle == before, "Home rook capture restoration");
            }
        }
        for (const std::string fen :
             {"8/8/8/8/8/8/8/8 w - -", "8/8/8/8/8/8/8/7X w - - 0 1",
              "8/8/8/8/8/8/8/44 w - - 0 1", "8/8/8/8/8/8/8/8 x - - 0 1",
              "8/8/8/8/8/8/8/8 w KK - 0 1", "8/8/8/8/8/8/8/8 w K- - 0 1",
              "8/8/8/8/8/8/8/8 w - e3 0 1", "8/8/8/8/8/8/8/8 w - - -1 1",
              "8/8/8/8/8/8/8/8 w - - -0 1", "8/8/8/8/8/8/8/8 w - - 0 0",
              "8/8/8/8/8/8/8/8 w - - 0 1 extra",
              "8/8/8/8/8/8/8/8 w - - 99999999999999999999 1"}) {
            const auto result = board.loadFen(fen);
            require(!result && board == original,
                    "FEN rejection/preservation: " + fen);
        }
        require(board.loadFen("8/8/8/8/8/8/8/8 w - - 0 1").has_value(),
                "Structural-only FEN permits teaching fixture");
        auto maxClock =
            position("4k3/8/8/8/8/8/8/4K3 b - - 2147483647 2147483647");
        const auto clockState = maxClock.makeMove({E8, makeSquare(3, 7)});
        require(maxClock.halfmoveClock() == std::numeric_limits<int>::max() &&
                    maxClock.fullmoveNumber() ==
                        std::numeric_limits<int>::max(),
                "Clock saturation");
        maxClock.undoMove({E8, makeSquare(3, 7)}, clockState);
        auto attack = position("4k3/4R3/8/8/8/8/8/4K3 w - - 0 1");
        for (const auto &move : generatePseudoLegalMoves(attack))
            require(move.to != E8, "King capture excluded");
        std::cout << "Board round trips, rights, clocks, FEN errors, and king "
                     "capture checks passed.\n";
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
