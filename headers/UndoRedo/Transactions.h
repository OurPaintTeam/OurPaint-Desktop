#ifndef OURPAINT_HEADERS_UNDOREDO_TRANSACTIONS_H_
#define OURPAINT_HEADERS_UNDOREDO_TRANSACTIONS_H_

#include <string>
#include <vector>
#include <stdexcept>

#include "Command.h"

class Transaction {
private:
    std::vector<Command*> commands;
    std::string name;
    bool committed = false;

public:
    Transaction(std::string&& name);
    ~Transaction();

    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    Transaction(Transaction&&);
    Transaction& operator=(Transaction&&);

    void addCommand(Command* cnd);

    void commit(); // Фиксация транзакции
    bool undo();
    bool redo();

    std::string label() const; // Название транзакции
    bool isCommitted() const;

private:
    void rollback(); // Откат (если ошибка)
};

#endif // ! OURPAINT_HEADERS_UNDOREDO_TRANSACTIONS_H_