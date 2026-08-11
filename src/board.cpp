#include "kestrel/board.hpp"

#include <sstream>
#include <vector>

namespace kestrel {

namespace {

Piece pieceFromFen(char c) {
    Color color = c >= 'A' && c <= 'Z' ? Color::White : Color::Black;

    switch (static_cast<char>(c | 32)) {
        case 'p': return {PieceType::Pawn, color};
        case 'n': return {PieceType::Knight, color};
        case 'b': return {PieceType::Bishop, color};
        case 'r': return {PieceType::Rook, color};
        case 'q': return {PieceType::Queen, color};
        case 'k': return {PieceType::King, color};
        default: return {};
    }
}

} // namespace

Board::Board() {
    setStartingPosition();
}

void Board::clear() {
    squares_.fill({});
    sideToMove_ = Color::White;
    enPassantSquare_ = -1;
    castlingRights_ = 0;
}

void Board::setStartingPosition() {
    setFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

bool Board::setFromFen(const std::string& fen) {
    clear();

    std::istringstream input(fen);
    std::string placement;
    std::string side;
    std::string castling;
    std::string enPassant;

    if (!(input >> placement >> side >> castling >> enPassant)) {
        return false;
    }

    int rank = 7;
    int file = 0;

    for (char c : placement) {
        if (c == '/') {
            if (file != 8 || rank == 0) {
                return false;
            }
            --rank;
            file = 0;
            continue;
        }

        if (c >= '1' && c <= '8') {
            file += c - '0';
        } else {
            if (file >= 8 || rank < 0) {
                return false;
            }
            squares_[makeSquare(file, rank)] = pieceFromFen(c);
            ++file;
        }

        if (file > 8) {
            return false;
        }
    }

    if (rank != 0 || file != 8) {
        return false;
    }

    if (side == "w") {
        sideToMove_ = Color::White;
    } else if (side == "b") {
        sideToMove_ = Color::Black;
    } else {
        return false;
    }

    for (char c : castling) {
        switch (c) {
            case 'K': castlingRights_ |= WhiteKingSide; break;
            case 'Q': castlingRights_ |= WhiteQueenSide; break;
            case 'k': castlingRights_ |= BlackKingSide; break;
            case 'q': castlingRights_ |= BlackQueenSide; break;
            case '-': break;
            default: return false;
        }
    }

    if (enPassant == "-") {
        enPassantSquare_ = -1;
    } else if (enPassant.size() == 2 &&
               enPassant[0] >= 'a' && enPassant[0] <= 'h' &&
               enPassant[1] >= '1' && enPassant[1] <= '8') {
        enPassantSquare_ = makeSquare(enPassant[0] - 'a', enPassant[1] - '1');
    } else {
        return false;
    }

    return true;
}

const Piece& Board::pieceAt(Square square) const {
    return squares_[square];
}

Piece& Board::pieceAt(Square square) {
    return squares_[square];
}

BoardState Board::makeMove(const Move& move) {
    BoardState state{
        sideToMove_,
        enPassantSquare_,
        castlingRights_
    };

    Piece moving = squares_[move.from];
    squares_[move.to] = moving;
    squares_[move.from] = {};

    if (move.promotion != PieceType::None) {
        squares_[move.to].type = move.promotion;
    }

    if (move.isEnPassant) {
        int captureOffset = moving.color == Color::White ? -8 : 8;
        squares_[move.to + captureOffset] = {};
    }

    if (move.isCastling) {
        if (move.to > move.from) {
            Square rookFrom = move.from + 3;
            Square rookTo = move.from + 1;
            squares_[rookTo] = squares_[rookFrom];
            squares_[rookFrom] = {};
        } else {
            Square rookFrom = move.from - 4;
            Square rookTo = move.from - 1;
            squares_[rookTo] = squares_[rookFrom];
            squares_[rookFrom] = {};
        }
    }

    enPassantSquare_ = -1;
    if (moving.type == PieceType::Pawn &&
        std::abs(static_cast<int>(move.to) - static_cast<int>(move.from)) == 16) {
        enPassantSquare_ =
            (static_cast<int>(move.from) + static_cast<int>(move.to)) / 2;
    }

    sideToMove_ = opposite(sideToMove_);
    return state;
}

void Board::undoMove(const Move& move, const BoardState& state) {
    Piece moving = squares_[move.to];

    if (move.isCastling) {
        if (move.to > move.from) {
            Square rookFrom = move.from + 3;
            Square rookTo = move.from + 1;
            squares_[rookFrom] = squares_[rookTo];
            squares_[rookTo] = {};
        } else {
            Square rookFrom = move.from - 4;
            Square rookTo = move.from - 1;
            squares_[rookFrom] = squares_[rookTo];
            squares_[rookTo] = {};
        }
    }

    squares_[move.from] = moving;
    squares_[move.to] = {};

    // Bootstrap limitation: captured pieces are not yet restored here.
    // This will be replaced by a complete reversible state in the next milestone.
    if (move.isEnPassant) {
        int captureOffset = moving.color == Color::White ? -8 : 8;
        squares_[move.to + captureOffset] = {
            PieceType::Pawn,
            opposite(moving.color)
        };
    }

    sideToMove_ = state.sideToMove;
    enPassantSquare_ = state.enPassantSquare;
    castlingRights_ = state.castlingRights;
}

} // namespace kestrel
