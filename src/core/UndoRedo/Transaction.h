#ifndef OURPAINT_HEADERS_UNDOREDO_TRANSACTION_H_
#define OURPAINT_HEADERS_UNDOREDO_TRANSACTION_H_


#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Command.h"

namespace UndoRedo {

    class Transaction {
    private:
        enum class State {
            Building,
            Applied,
            Reverted
        };

        std::vector<std::unique_ptr<Command>> commands_;
        std::string name_;
        State state_ = State::Building;

    public:
        explicit Transaction(std::string name);

        Transaction(const Transaction&) = delete;
        Transaction& operator=(const Transaction&) = delete;

        Transaction(Transaction&&) noexcept = default;
        Transaction& operator=(Transaction&&) noexcept = default;

        [[nodiscard]] bool addCommand(std::unique_ptr<Command> command);

        // Executes all commands in order. If any command fails,
        // already executed commands are rolled back in reverse order.
        [[nodiscard]] bool commit() noexcept;

        // Reverts all commands in reverse order. If any command fails,
        // already reverted commands are restored in forward order.
        [[nodiscard]] bool undo() noexcept;

        // Reapplies all commands in forward order. If any command fails,
        // already reapplied commands are reverted in reverse order.
        [[nodiscard]] bool redo() noexcept;

        [[nodiscard]] const std::string& label() const noexcept;
        [[nodiscard]] bool isApplied() const noexcept;
        [[nodiscard]] bool isReverted() const noexcept;
        [[nodiscard]] bool isBuilding() const noexcept;
    };

}

#endif // ! OURPAINT_HEADERS_UNDOREDO_TRANSACTION_H_