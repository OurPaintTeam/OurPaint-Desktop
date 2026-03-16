#include <gtest/gtest.h>
#include "Circle.h"
#include "Point2D.h"

using namespace OurPaintDCM::Figures;
TEST(CircleTest, ConstructorAndDefaults) {
    Point2D center({1.0, 2.0});
    Circle<Point2D> circle(&center, 5.0);

    EXPECT_EQ(circle.center, &center);
    EXPECT_DOUBLE_EQ(circle.radius, 5.0);
}

// Проверка вычисления площади
TEST(CircleTest, AreaCalculation) {
    Point2D center({0.0, 0.0});
    Circle<Point2D> circle(&center, 3.0);

    double expectedArea = std::numbers::pi * 3.0 * 3.0;
    EXPECT_NEAR(circle.area(), expectedArea, 1e-9);
}

// Проверка длины окружности
TEST(CircleTest, LengthCalculation) {
    Point2D center({0.0, 0.0});
    Circle<Point2D> circle(&center, 2.0);

    double expectedLength = 2 * std::numbers::pi * 2.0;
    EXPECT_NEAR(circle.lenght(), expectedLength, 1e-9);
}

// Проверка изменения радиуса
TEST(CircleTest, RadiusModification) {
    Point2D center({0.0, 0.0});
    Circle<Point2D> circle(&center, 4.0);

    circle.radius = 10.0;
    EXPECT_DOUBLE_EQ(circle.radius, 10.0);
}

// Проверка изменения координат центра
TEST(CircleTest, CenterModification) {
    Point2D center({1.0, 1.0});
    Circle<Point2D> circle(&center, 3.0);

    center[0] = 5.0;
    center[1] = 7.0;

    EXPECT_DOUBLE_EQ(circle.center->coords[0], 5.0);
    EXPECT_DOUBLE_EQ(circle.center->coords[1], 7.0);
}