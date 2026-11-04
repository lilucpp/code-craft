#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace cpp_idioms {

/**
 * @brief Non-intrusive polymorphic wrapper implementing the Type Erasure Idiom.
 * 
 * Demonstrates:
 * 1. Full value semantics (copyable, movable, regular class).
 * 2. Non-intrusive duck-typing: Any type with a `draw() const` method can be stored.
 * 3. Heterogeneous container storage without leaking pointers or requiring base inheritance.
 */
class AnyDrawable {
public:
    // Templated constructor accepting any compatible type
    template <typename T>
        requires (!std::is_same_v<std::decay_t<T>, AnyDrawable>)
    explicit AnyDrawable(T value)
        : concept_(std::make_unique<Model<T>>(std::move(value))) {}

    // Copy Constructor via Virtual Clone Idiom
    AnyDrawable(const AnyDrawable& other)
        : concept_(other.concept_ ? other.concept_->clone() : nullptr) {}

    // Move Constructor
    AnyDrawable(AnyDrawable&& other) noexcept = default;

    // Unified Assignment Operator (Copy-and-Swap idiom)
    AnyDrawable& operator=(AnyDrawable other) noexcept {
        concept_ = std::move(other.concept_);
        return *this;
    }

    ~AnyDrawable() = default;

    // Public domain interface
    void draw() const {
        if (concept_) {
            concept_->do_draw();
        }
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return concept_ != nullptr;
    }

private:
    // Tier 1: Internal Abstract Concept
    struct Concept {
        virtual ~Concept() = default;
        virtual void do_draw() const = 0;
        virtual std::unique_ptr<Concept> clone() const = 0;
    };

    // Tier 2: Templated Model holding concrete instance by value
    template <typename T>
    struct Model final : Concept {
        T value_;

        explicit Model(T val) : value_(std::move(val)) {}

        void do_draw() const override {
            // Duck-typing requirement: value_ must have .draw()
            value_.draw();
        }

        std::unique_ptr<Concept> clone() const override {
            return std::make_unique<Model<T>>(value_);
        }
    };

    // Tier 3: Storage
    std::unique_ptr<Concept> concept_;
};

// --- Test types with completely unrelated hierarchies ---

class Circle {
public:
    explicit Circle(double radius) : radius_(radius) {}

    void draw() const {
        std::cout << "Drawing Circle with radius: " << radius_ << '\n';
    }

private:
    double radius_;
};

class Rectangle {
public:
    Rectangle(double w, double h) : width_(w), height_(h) {}

    void draw() const {
        std::cout << "Drawing Rectangle: " << width_ << " x " << height_ << '\n';
    }

private:
    double width_;
    double height_;
};

class TextLabel {
public:
    explicit TextLabel(std::string text) : text_(std::move(text)) {}

    void draw() const {
        std::cout << "Rendering TextLabel: \"" << text_ << "\"\n";
    }

private:
    std::string text_;
};

} // namespace cpp_idioms
