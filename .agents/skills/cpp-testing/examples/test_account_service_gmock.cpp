#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace domain {

struct Account {
    std::string id;
    double balance;
};

// 1. Dependency Interface: Account Repository
class IAccountRepository {
public:
    virtual ~IAccountRepository() = default;
    virtual Account find_by_id(std::string_view id) = 0;
    virtual void update_balance(std::string_view id, double new_balance) = 0;
};

// 2. Dependency Interface: Notification Service
class INotificationService {
public:
    virtual ~INotificationService() = default;
    virtual void notify_transfer(std::string_view account_id, double amount) = 0;
};

// 3. System Under Test: Account Transfer Service
class TransferService {
public:
    TransferService(std::shared_ptr<IAccountRepository> repo,
                    std::shared_ptr<INotificationService> notifier)
        : repo_(std::move(repo)), notifier_(std::move(notifier)) {}

    void transfer(std::string_view from_id, std::string_view to_id, double amount) {
        if (amount <= 0.0) {
            throw std::invalid_argument("Transfer amount must be strictly positive.");
        }

        Account from = repo_->find_by_id(from_id);
        Account to = repo_->find_by_id(to_id);

        if (from.balance < amount) {
            throw std::runtime_error("Insufficient account balance.");
        }

        repo_->update_balance(from_id, from.balance - amount);
        repo_->update_balance(to_id, to.balance + amount);

        notifier_->notify_transfer(to_id, amount);
    }

private:
    std::shared_ptr<IAccountRepository> repo_;
    std::shared_ptr<INotificationService> notifier_;
};

} // namespace domain

// --- Mock Classes using Modern MOCK_METHOD ---

class MockAccountRepository : public domain::IAccountRepository {
public:
    MOCK_METHOD(domain::Account, find_by_id, (std::string_view id), (override));
    MOCK_METHOD(void, update_balance, (std::string_view id, double new_balance), (override));
};

class MockNotificationService : public domain::INotificationService {
public:
    MOCK_METHOD(void, notify_transfer, (std::string_view account_id, double amount), (override));
};

// --- Test Suite adhering to AAA Pattern ---

using ::testing::DoubleEq;
using ::testing::Eq;
using ::testing::InSequence;
using ::testing::Return;
using ::testing::StrEq;

class TransferServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        mock_repo_ = std::make_shared<MockAccountRepository>();
        mock_notifier_ = std::make_shared<MockNotificationService>();
        service_ = std::make_unique<domain::TransferService>(mock_repo_, mock_notifier_);
    }

    std::shared_ptr<MockAccountRepository> mock_repo_;
    std::shared_ptr<MockNotificationService> mock_notifier_;
    std::unique_ptr<domain::TransferService> service_;
};

TEST_F(TransferServiceTest, SuccessfulTransferUpdatesBalancesAndSendsNotification) {
    // 1. Arrange: Setup account states and expectations
    const std::string from_acc = "ACC-001";
    const std::string to_acc = "ACC-002";
    const double transfer_amount = 250.0;

    EXPECT_CALL(*mock_repo_, find_by_id(StrEq(from_acc)))
        .WillOnce(Return(domain::Account{from_acc, 1000.0}));
    EXPECT_CALL(*mock_repo_, find_by_id(StrEq(to_acc)))
        .WillOnce(Return(domain::Account{to_acc, 100.0}));

    // Expect sequential balance updates
    {
        InSequence seq;
        EXPECT_CALL(*mock_repo_, update_balance(StrEq(from_acc), DoubleEq(750.0))).Times(1);
        EXPECT_CALL(*mock_repo_, update_balance(StrEq(to_acc), DoubleEq(350.0))).Times(1);
    }

    // Expect notification
    EXPECT_CALL(*mock_notifier_, notify_transfer(StrEq(to_acc), DoubleEq(transfer_amount)))
        .Times(1);

    // 2. Act: Perform transfer
    EXPECT_NO_THROW({
        service_->transfer(from_acc, to_acc, transfer_amount);
    });

    // 3. Assert: Mock expectations verified automatically on mock destruction
}

TEST_F(TransferServiceTest, ThrowsAndDoesNotUpdateWhenBalanceIsInsufficient) {
    // 1. Arrange
    const std::string from_acc = "ACC-001";
    const std::string to_acc = "ACC-002";

    EXPECT_CALL(*mock_repo_, find_by_id(StrEq(from_acc)))
        .WillOnce(Return(domain::Account{from_acc, 50.0})); // Only 50 available
    EXPECT_CALL(*mock_repo_, find_by_id(StrEq(to_acc)))
        .WillOnce(Return(domain::Account{to_acc, 100.0}));

    // Critical: No balance updates and no notifications should occur!
    EXPECT_CALL(*mock_repo_, update_balance(testing::_, testing::_)).Times(0);
    EXPECT_CALL(*mock_notifier_, notify_transfer(testing::_, testing::_)).Times(0);

    // 2. Act & 3. Assert
    EXPECT_THROW({
        service_->transfer(from_acc, to_acc, 200.0);
    }, std::runtime_error);
}
