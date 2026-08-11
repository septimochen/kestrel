#include "kestrel/movegen.hpp"

namespace kestrel {

namespace {

void addMove(std::vector<Move>& moves, Square from, Square to,
             const Board& board, PieceType promotion = PieceType::None) {
    if (to >= 64) {
        return;
    }

    const Piece& target = board.pieceAt(to);
    const Piece& moving = board.pieceAt(from);

    if (!target.empty() && target.color == moving.color) {
        return;
    }

    moves.push_back({
        from,
        to,
        promotion,
        !target.empty(),
        false,
        false
    });
}

void generateKnightMoves(const Board& board, Square from,
                         std::vector<Move>& moves) {
    static constexpr int offsets[] = {
        {1}, {2}, {2}, {1}, {-1}, {-2}, {-2}, {-1}
    };
    static constexpr int rankOffsets[] = {
        {2}, {1}, {-1}, {-2}, {-2}, {-1}, {1}, {2}
    };

    int file = fileOf(from);
    int rank = rankOf(from);

    for (int i = 0; i < 8; ++i) {
        int f = file + offsets[i];
        int r = rank + rankOffsets[i];

        if (f >= 0 && f < 8 && r >= 0 && r < 8) {
            addMove(moves, from, makeSquare(f, r), board);
        }
    }
}

void generateKingMoves(const Board& board, Square from,
                       std::vector<Move>& moves) {
    int file = fileOf(from);
    int rank = rankOf(from);

    for (int df = -1; df <= 1; ++df) {
        for (int dr = -1; dr <= 1; ++dr) {
            if (df == 0 && dr == 0) {
                continue;
            }

            int f = file + df;
            int r = rank + dr;

            if (f >= 0 && f < 8 && r >= 0 && r < 8) {
                addMove(moves, from, makeSquare(f, r), board);
            }
        }
    }
}

void generateSlidingMoves(const Board& board, Square from,
                          std::vector<Move>& moves,
                          const int directions[][2], int directionCount) {
    int startFile = fileOf(from);
    int startRank = rankOf(from);

    for (int i = 0; i < directionCount; ++i) {
        int file = startFile + directions[i][0];
        int rank = startRank + directions[i][1];

        while (file >= 0 && file < 8 && rank >= 0 && rank < 8) {
            Square to = makeSquare(file, rank);
            const Piece& target = board.pieceAt(to);

            if (target.empty()) {
                addMove(moves, from, to, board);
            } else {
                if (target.color != board.pieceAt(from).color) {
                    addMove(moves, from, to, board);
                }
                break;
            }

            file += directions[i][0];
            rank += directions[i][1];
        }
    }
}

void generatePawnMoves(const Board& board, Square from,
                       std::vector<Move>& moves) {
    const Piece& pawn = board.pieceAt(from);
    int direction = pawn.color == Color::White ? 1 : -1;
    int startRank = pawn.color == Color::White ? 1 : 6;
    int promotionRank = pawn.color == Color::White ? 7 : 0;

    int file = fileOf(from);
    int rank = rankOf(from);

    int nextRank = rank + direction;
    if (nextRank >= 0 && nextRank < 8) {
        Square one = makeSquare(file, nextRank);

        if (board.pieceAt(one).empty()) {
            if (nextRank == promotionRank) {
                for (PieceType promotion : {
                    PieceType::Queen,
                    PieceType::Rook,
                    PieceType::Bishop,
                    PieceType::Knight
                }) {
                    addMove(moves, from, one, board, promotion);
                }
            } else {
                addMove(moves, from, one, board);

                if (rank == startRank) {
                    Square two = makeSquare(file, rank + 2 * direction);
                    if (board.pieceAt(two).empty()) {
                        addMove(moves, from, two, board);
                    }
                }
            }
        }

        for (int df : {-1, 1}) {
            int captureFile = file + df;
            if (captureFile < 0 || captureFile >= 8) {
                continue;
            }

            Square capture = makeSquare(captureFile, nextRank);
            const Piece& target = board.pieceAt(capture);

            if (!target.empty() && target.color != pawn.color) {
                if (nextRank == promotionRank) {
                    for (PieceType promotion : {
                        PieceType::Queen,
                        PieceType::Rook,
                        PieceType::Bishop,
                        PieceType::Knight
                    }) {
                        addMove(moves, from, capture, board, promotion);
                    }
                } else {
                    addMove(moves, from, capture, board);
                }
            }
        }
    }
}

} // namespace

std::vector<Move> generatePseudoLegalMoves(const Board& board) {
    std::vector<Move> moves;

    static constexpr int bishopDirections[][2] = {
        {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
    };

    static constexpr int rookDirections[][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };

    for (Square from = 0; from < 64; ++from) {
        const Piece& piece = board.pieceAt(from);

        if (piece.empty() || piece.color != board.sideToMove()) {
            continue;
        }

        switch (piece.type) {
            case PieceType::Pawn:
                generatePawnMoves(board, from, moves);
                break;

            case PieceType::Knight:
                generateKnightMoves(board, from, moves);
                break;

            case PieceType::Bishop:
                generateSlidingMoves(
                    board, from, moves,
                    bishopDirections, 4
                );
                break;

            case PieceType::Rook:
                generateSlidingMoves(
                    board, from, moves,
                    rookDirections, 4
                );
                break;

            case PieceType::Queen:
                generateSlidingMoves(
                    board, from, moves,
                    bishopDirections, 4
                );
                generateSlidingMoves(
                    board, from, moves,
                    rookDirections, 4
                );
                break;

            case PieceType::King:
                generateKingMoves(board, from, moves);
                break;

            case PieceType::None:
                break;
        }
    }

    return moves;
}

} // namespace kestrel
