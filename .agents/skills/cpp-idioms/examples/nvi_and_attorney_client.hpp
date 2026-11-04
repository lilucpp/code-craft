#pragma once

#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace cpp_idioms {

// Forward declaration of Attorney
class OrderAttorney;

/**
 * @brief Domain Entity demonstrating the Attorney-Client idiom.
 * 
 * In standard C++, declaring an external service or unit test as a `friend`
 * exposes ALL private members (internal keys, customer data, private helpers).
 * 
 * With the Attorney-Client Idiom:
 * 1. Order grants friendship ONLY to OrderAttorney.
 * 2. OrderAttorney selectively exposes only audit-related internals.
 */
class Order {
public:
    Order(std::string_view order_id, double amount)
        : order_id_(order_id), amount_(amount), is_settled_(false) {
        if (amount <= 0.0) {
            throw std::invalid_argument("Order amount must be positive.");
        }
    }

    [[nodiscard]] std::string_view get_id() const noexcept { return order_id_; }
    [[nodiscard]] double get_amount() const noexcept { return amount_; }
    [[nodiscard]] bool is_settled() const noexcept { return is_settled_; }

private:
    // Attorney-Client Idiom: friendship granted ONLY to OrderAttorney
    friend class OrderAttorney;

    void mark_as_settled_by_audit() noexcept {
        is_settled_ = true;
    }

    [[nodiscard]] std::string get_internal_security_token() const {
        return "SECRET_TOKEN_" + order_id_;
    }

    std::string order_id_;
    double amount_;
    bool is_settled_;
    std::string sensitive_credit_card_hash_{"sha256_hash_987654321"};
};

/**
 * @brief Attorney class acting as an access gatekeeper.
 */
class OrderAttorney {
private:
    // Only PaymentAuditInspector is allowed to touch audit operations
    friend class PaymentAuditInspector;

    static void settle_order(Order& order) noexcept {
        order.mark_as_settled_by_audit(); // selectively exposed!
    }

    // Notice: sensitive_credit_card_hash_ and get_internal_security_token()
    // are NOT exposed by OrderAttorney, preserving encapsulation!
};

/**
 * @brief External collaborator that interacts with Order through the Attorney.
 */
class PaymentAuditInspector {
public:
    void audit_and_settle(Order& order) {
        if (order.get_amount() > 0.0) {
            OrderAttorney::settle_order(order);
        }
    }
};

/**
 * @brief Abstract Base Class demonstrating the Non-Virtual Interface (NVI) idiom.
 * 
 * In NVI:
 * - Public interface methods are strictly NON-VIRTUAL. They enforce class invariants,
 *   metering, exception handling, and pre/post-conditions.
 * - Virtual customization points are strictly PRIVATE (or protected).
 */
class PaymentProcessor {
public:
    virtual ~PaymentProcessor() = default;

    // Non-virtual interface enforcing invariant and audit wrapping
    bool process_transaction(Order& order) {
        // Pre-condition: Check invariants
        if (order.is_settled()) {
            throw std::logic_error("Cannot re-process an already settled order.");
        }

        std::cout << "[NVI Pre-Condition] Validating order: " << order.get_id() << '\n';

        // Call private virtual customization hook
        const bool success = do_process_transaction(order);

        // Post-condition: Audit logging and status update
        if (success) {
            std::cout << "[NVI Post-Condition] Order " << order.get_id() << " succeeded.\n";
        } else {
            std::cerr << "[NVI Post-Condition] Order " << order.get_id() << " failed!\n";
        }

        return success;
    }

private:
    // Pure virtual hook customized by derived classes
    [[nodiscard]] virtual bool do_process_transaction(Order& order) = 0;
};

/**
 * @brief Concrete derived implementation overriding private virtual hook.
 */
class CreditCardProcessor final : public PaymentProcessor {
private:
    [[nodiscard]] bool do_process_transaction(Order& order) override {
        std::cout << "  -> Charging $" << order.get_amount() << " via Credit Card Gateway\n";
        return true;
    }
};

} // namespace cpp_idioms
