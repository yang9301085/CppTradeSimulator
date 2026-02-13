#pragma once

#include <stdexcept>
#include <string>

namespace trade_sim {

/** 错误码（训练：异常类型选择 vs 错误码） */
enum class ErrorCode {
    InvalidArgument,
    NotFound,
    Duplicate,
    InsufficientFunds,
    InsufficientPosition,
    IOError,
    ParseError,
    InvalidState
};

class TradeSimException : public std::runtime_error {
public:
    TradeSimException(ErrorCode code, const std::string& msg)
        : std::runtime_error(msg), code_(code) {}

    ErrorCode code() const noexcept { return code_; }

private:
    ErrorCode code_;
};

class NotFoundException : public TradeSimException {
public:
    explicit NotFoundException(const std::string& msg)
        : TradeSimException(ErrorCode::NotFound, msg) {}
};

class InvalidArgumentException : public TradeSimException {
public:
    explicit InvalidArgumentException(const std::string& msg)
        : TradeSimException(ErrorCode::InvalidArgument, msg) {}
};

class InsufficientFundsException : public TradeSimException {
public:
    explicit InsufficientFundsException(const std::string& msg)
        : TradeSimException(ErrorCode::InsufficientFunds, msg) {}
};

class IOErrorException : public TradeSimException {
public:
    explicit IOErrorException(const std::string& msg)
        : TradeSimException(ErrorCode::IOError, msg) {}
};

class ParseErrorException : public TradeSimException {
public:
    explicit ParseErrorException(const std::string& msg)
        : TradeSimException(ErrorCode::ParseError, msg) {}
};

} // namespace trade_sim
