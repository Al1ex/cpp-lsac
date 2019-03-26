#pragma once

#include <exception>
#include "Common.h"

namespace eth
{

class Exception: public std::exception
{
public:
	virtual std::string description() const { return typeid(*this).name(); }
	virtual char const* what() const noexcept { return typeid(*this).name(); }
};

class BadHexCharacter: public Exception {};
class NotEnoughCash: public Exception {};
class BadInstruction: public Exception {};
class StackTooSmall: public Exception { public: StackTooSmall(u256 _req, u256 _got): req(_req), got(_got) {} u256 req; u256 got; };
class OperandOutOfRange: public Exception { public: OperandOutOfRange(u256 _min, u256 _max, u256 _got): mn(_min), mx(_max), got(_got) {} u256 mn; u256 mx; u256 got; };
class ExecutionException: public Exception {};
class NoSuchContract: public Exception {};
class ContractAddressCollision: public Exception {};
class FeeTooSmall: public Exception {};
class InvalidSignature: public Exception {};
class InvalidTransactionFormat: public Exception {};
class InvalidBlockFormat: public Exception { public: InvalidBlockFormat(int _f, bytesConstRef _d): m_f(_f), m_d(_d.toBytes()) {} int m_f; bytes m_d; virtual std::string description() const { return "Invalid block format: Bad field " + toString(m_f) + " (" + asHex(m_d) + ")"; } };
class InvalidBlockHeaderFormat: public Exception { public: InvalidBlockHeaderFormat(int _f, bytesConstRef _d): m_f(_f), m_d(_d.toBytes()) {} int m_f; bytes m_d; virtual std::string description() const { return "Invalid block header format: Bad field " + toString(m_f) + " (" + asHex(m_d) + ")"; } };
class InvalidUnclesHash: public Exception {};
class InvalidUncle: public Exception {};
class InvalidStateRoot: public Exception {};
class InvalidTransactionsHash: public Exception {};
class InvalidTransaction: public Exception {};
class InvalidDifficulty: public Exception {};
class InvalidTimestamp: public Exception {};
class InvalidNonce: public Exception { public: InvalidNonce(u256 _required = 0, u256 _candidate = 0): required(_required), candidate(_candidate) {} u256 required; u256 candidate; virtual std::string description() const { return "Invalid nonce (r: " + toString(required) + " c:" + toString(candidate) + ")"; } };
class InvalidBlockNonce: public Exception { public: InvalidBlockNonce(h256 _h = h256(), h256 _n = h256(), u256 _d = 0): h(_h), n(_n), d(_d) {} h256 h; h256 n; u256 d; virtual std::string description() const { return "Invalid nonce (h: " + toString(h) + " n:" + toString(n) + " d:" + toString(d) + ")"; } };
class InvalidParentHash: public Exception {};
class InvalidContractAddress: public Exception {};
class NoNetworking: public Exception {};
class NoUPnPDevice: public Exception {};
class RootNotFound: public Exception {};

}
