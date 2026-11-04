#pragma once

#include <iostream>
#include <memory>
#include <string>

/**
 * @brief 现代 C++ 虚拟构造器模式 (Virtual Copy Constructor / Clone Idiom)
 *
 * 理论依据：
 * - 《More Effective C++》条款 25：将构造函数与非成员函数虚拟化
 * - 《Effective Modern C++》条款 18：使用 std::unique_ptr 表达独占所有权
 *
 * 核心机制：
 * 解决面向对象中“仅持有 Base 指针/引用时，如何安全精确拷贝出一个派生类副本”的经典问题。
 */
class CloneableShape {
public:
    virtual ~CloneableShape() = default;

    // 虚拟拷贝函数：返回 std::unique_ptr 转移独占所有权
    [[nodiscard]] virtual std::unique_ptr<CloneableShape> clone() const = 0;

    virtual void draw() const = 0;
};

class CircleShape : public CloneableShape {
public:
    explicit CircleShape(double radius) : radius_(radius) {}

    // 派生类重写 clone：多态化生成自身副本
    [[nodiscard]] std::unique_ptr<CloneableShape> clone() const override {
        return std::make_unique<CircleShape>(*this);
    }

    void draw() const override {
        std::cout << "Drawing Circle with radius: " << radius_ << '\n';
    }

private:
    double radius_;
};

class RectangleShape : public CloneableShape {
public:
    RectangleShape(double w, double h) : width_(w), height_(h) {}

    [[nodiscard]] std::unique_ptr<CloneableShape> clone() const override {
        return std::make_unique<RectangleShape>(*this);
    }

    void draw() const override {
        std::cout << "Drawing Rectangle: " << width_ << "x" << height_ << '\n';
    }

private:
    double width_;
    double height_;
};
