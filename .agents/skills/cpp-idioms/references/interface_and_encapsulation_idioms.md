# Interface and Encapsulation Idioms

This reference details core design idioms from *More C++ Idioms* focused on class encapsulation, interface boundaries, and member accessibility control.

---

## 1. Non-Virtual Interface (NVI)

### Intent & Problem
In traditional object-oriented designs, public virtual functions conflate two orthogonal responsibilities:
1. **Interface Contract**: Public API specification, invariants, pre-conditions, and post-conditions.
2. **Implementation Customization**: Specific derived class behavior variations.

When a public virtual function is overridden, the derived class can bypass base class invariant checks, state locks, or logging mechanisms.

### Modern C++ Solution
Apply the **Template Method Design Pattern** at the class interface level:
- Declare public API methods as **non-virtual**.
- Place virtual customization hooks in `private` (or `protected` if derived classes need to chain base calls).
- Wrap virtual customization points with invariant assertions, audit logs, or mutex locks in the public non-virtual method.

```cpp
#include <concepts>
#include <iostream>
#include <stdexcept>

class PaymentGateway {
public:
    virtual ~PaymentGateway() = default;

    // Non-virtual public interface enforcing invariants
    void process_payment(double amount) {
        if (amount <= 0.0) {
            throw std::invalid_argument("Payment amount must be positive.");
        }
        // Pre-condition: Logging, metrics, rate-limiting
        std::cout << "[Audit] Initiating transaction for: " << amount << '\n';

        do_process_payment(amount); // Virtual customization point

        // Post-condition: Invariant validation, audit event publishing
        std::cout << "[Audit] Transaction finalized successfully.\n";
    }

private:
    // Pure virtual or virtual hook customized by derived classes
    virtual void do_process_payment(double amount) = 0;
};

class CreditCardGateway final : public PaymentGateway {
private:
    void do_process_payment(double amount) override {
        std::cout << "[Gateway] Charging credit card: $" << amount << '\n';
    }
};
```

### When to Use
- Any polymorphic base class where pre-/post-conditions or lifecycle sequencing must be strictly enforced.
- **Exception**: Base class virtual destructors must remain public virtual (or protected non-virtual).

---

## 2. Attorney-Client Idiom

### Intent & Problem
In C++, `friend` is an "all-or-nothing" construct: granting `friend` access gives the friend class complete visibility into *all* private and protected members of the granter class. This frequently compromises encapsulation when a collaborator or unit test needs access to only one specific internal function.

### Solution
Introduce an **Attorney** (proxy) class:
1. The **Client** class declares only the `Attorney` as its `friend`.
2. The `Attorney` provides static proxy inline methods that forward only the specifically allowed operations to the `Client`.
3. The `Attorney` grants `friend` access to the external collaborator or test suite.

```cpp
class BankAccount;

// Attorney class mediating access
class BankAccountAttorney {
private:
    // Only BankAuditService is allowed to call audit inspect methods
    friend class BankAuditService;

    static void inspect_internal_ledger(const BankAccount& account);
};

class BankAccount {
public:
    explicit BankAccount(long account_id, double balance)
        : account_id_(account_id), balance_(balance) {}

    double get_balance() const noexcept { return balance_; }

private:
    // Client grants friendship ONLY to the Attorney
    friend class BankAccountAttorney;

    void audit_dump() const {
        // Highly sensitive internal dump
    }

    long account_id_;
    double balance_;
    std::string secret_pin_hash_{"sha256_hash_here"};
};

inline void BankAccountAttorney::inspect_internal_ledger(const BankAccount& account) {
    account.audit_dump(); // Only audit_dump is exposed, secret_pin_hash_ is NOT leaked
}

class BankAuditService {
public:
    void audit_account(const BankAccount& acc) {
        // Calls via Attorney proxy without having full access to BankAccount internals
        BankAccountAttorney::inspect_internal_ledger(acc);
    }
};
```

### Benefits
- Granular, fine-grained friendship without breaking complete class encapsulation.
- Clear separation between public API, internal implementation, and subsystem-internal access.

---

## 3. Interface Class Idiom

### Intent
Define a pure protocol without state or implementation inheritance, decoupling components completely across architectural layers.

### Modern C++ Guidelines
- Declare all member functions as `virtual ... = 0` (pure virtual).
- Define a `virtual ~Interface() = default;` virtual destructor.
- **Do not include non-static data members**.
- Prefer passing by `std::unique_ptr<Interface>` or `Interface&` to preserve runtime polymorphism.

```cpp
class INetworkClient {
public:
    virtual ~INetworkClient() = default;

    virtual void connect(std::string_view endpoint) = 0;
    virtual void send(std::span<const std::byte> payload) = 0;
    virtual void disconnect() noexcept = 0;
};
```

---

## 4. Capability Query Idiom (Cross-Cast)

### Intent
Determine dynamically at runtime whether an object implements a secondary, optional interface without requiring fat interfaces in the base class.

### Implementation
Use multiple inheritance of pure interface classes and test capabilities via `dynamic_cast`:

```cpp
class IComponent {
public:
    virtual ~IComponent() = default;
};

class ISerializable {
public:
    virtual ~ISerializable() = default;
    virtual std::string serialize() const = 0;
};

void persist_if_capable(IComponent* comp) {
    if (auto* serializable = dynamic_cast<ISerializable*>(comp)) {
        std::string data = serializable->serialize();
        // save data...
    }
}
```

---

## 5. Thin Template Idiom

### Intent & Problem
Templates can cause massive binary code bloat when instantiated across many pointer types or trivial types that share identical underlying representations (`std::vector<int*>`, `std::vector<char*>`, `std::vector<MyClass*>`).

### Solution
Factor the underlying implementation into a non-template base class operating on `void*` or untyped memory, and provide a thin, inline, type-safe template derived wrapper:

```cpp
// Non-template base class holding the compiled logic once
class ThinVectorBase {
protected:
    void** data_{nullptr};
    std::size_t size_{0};
    std::size_t capacity_{0};

    void push_back_ptr(void* ptr);
    void* pop_back_ptr();
    ~ThinVectorBase();
};

// Thin, inline, zero-overhead template wrapper
template <typename T>
class ThinVector : private ThinVectorBase {
public:
    void push_back(T* item) {
        push_back_ptr(static_cast<void*>(item));
    }

    T* back() const noexcept {
        return static_cast<T*>(data_[size_ - 1]);
    }
};
```
