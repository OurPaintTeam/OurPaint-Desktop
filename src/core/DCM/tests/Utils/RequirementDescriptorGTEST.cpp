#include <gtest/gtest.h>
#include "RequirementDescriptor.h"

using namespace OurPaintDCM::Utils;

// ==================== Default Constructor ====================

TEST(RequirementDescriptorTest, DefaultConstructor) {
    RequirementDescriptor desc;
    EXPECT_TRUE(desc.objectIds.empty());
    EXPECT_FALSE(desc.param.has_value());
}

// ==================== Full Constructor ====================

TEST(RequirementDescriptorTest, FullConstructor) {
    RequirementDescriptor desc(
        RequirementType::ET_POINTPOINTDIST,
        {ID(1), ID(2)},
        50.0
    );
    
    EXPECT_EQ(desc.type, RequirementType::ET_POINTPOINTDIST);
    EXPECT_EQ(desc.objectIds.size(), 2);
    EXPECT_EQ(desc.objectIds[0], ID(1));
    EXPECT_EQ(desc.objectIds[1], ID(2));
    EXPECT_TRUE(desc.param.has_value());
    EXPECT_DOUBLE_EQ(desc.param.value(), 50.0);
}

TEST(RequirementDescriptorTest, FullConstructorWithoutParam) {
    RequirementDescriptor desc(
        RequirementType::ET_POINTONPOINT,
        {ID(1), ID(2)}
    );
    
    EXPECT_EQ(desc.type, RequirementType::ET_POINTONPOINT);
    EXPECT_EQ(desc.objectIds.size(), 2);
    EXPECT_FALSE(desc.param.has_value());
}

// ==================== Factory Methods ====================

TEST(RequirementDescriptorTest, PointLineDist) {
    auto desc = RequirementDescriptor::pointLineDist(ID(1), ID(2), 25.0);
    
    EXPECT_EQ(desc.type, RequirementType::ET_POINTLINEDIST);
    EXPECT_EQ(desc.objectIds.size(), 2);
    EXPECT_EQ(desc.objectIds[0], ID(1));
    EXPECT_EQ(desc.objectIds[1], ID(2));
    EXPECT_DOUBLE_EQ(desc.param.value(), 25.0);
}

TEST(RequirementDescriptorTest, PointOnLine) {
    auto desc = RequirementDescriptor::pointOnLine(ID(3), ID(4));
    
    EXPECT_EQ(desc.type, RequirementType::ET_POINTONLINE);
    EXPECT_EQ(desc.objectIds.size(), 2);
    EXPECT_EQ(desc.objectIds[0], ID(3));
    EXPECT_EQ(desc.objectIds[1], ID(4));
    EXPECT_FALSE(desc.param.has_value());
}

TEST(RequirementDescriptorTest, PointPointDist) {
    auto desc = RequirementDescriptor::pointPointDist(ID(5), ID(6), 100.0);
    
    EXPECT_EQ(desc.type, RequirementType::ET_POINTPOINTDIST);
    EXPECT_EQ(desc.objectIds.size(), 2);
    EXPECT_DOUBLE_EQ(desc.param.value(), 100.0);
}

TEST(RequirementDescriptorTest, PointOnPoint) {
    auto desc = RequirementDescriptor::pointOnPoint(ID(7), ID(8));
    
    EXPECT_EQ(desc.type, RequirementType::ET_POINTONPOINT);
    EXPECT_EQ(desc.objectIds.size(), 2);
    EXPECT_FALSE(desc.param.has_value());
}

TEST(RequirementDescriptorTest, LineCircleDist) {
    auto desc = RequirementDescriptor::lineCircleDist(ID(9), ID(10), 15.5);
    
    EXPECT_EQ(desc.type, RequirementType::ET_LINECIRCLEDIST);
    EXPECT_EQ(desc.objectIds.size(), 2);
    EXPECT_DOUBLE_EQ(desc.param.value(), 15.5);
}

TEST(RequirementDescriptorTest, LineOnCircle) {
    auto desc = RequirementDescriptor::lineOnCircle(ID(11), ID(12));
    
    EXPECT_EQ(desc.type, RequirementType::ET_LINEONCIRCLE);
    EXPECT_EQ(desc.objectIds.size(), 2);
    EXPECT_FALSE(desc.param.has_value());
}

TEST(RequirementDescriptorTest, LineInCircle) {
    auto desc = RequirementDescriptor::lineInCircle(ID(13), ID(14));
    
    EXPECT_EQ(desc.type, RequirementType::ET_LINEINCIRCLE);
    EXPECT_EQ(desc.objectIds.size(), 2);
    EXPECT_FALSE(desc.param.has_value());
}

TEST(RequirementDescriptorTest, LineLineParallel) {
    auto desc = RequirementDescriptor::lineLineParallel(ID(15), ID(16));
    
    EXPECT_EQ(desc.type, RequirementType::ET_LINELINEPARALLEL);
    EXPECT_EQ(desc.objectIds.size(), 2);
    EXPECT_FALSE(desc.param.has_value());
}

TEST(RequirementDescriptorTest, LineLinePerpendicular) {
    auto desc = RequirementDescriptor::lineLinePerpendicular(ID(17), ID(18));
    
    EXPECT_EQ(desc.type, RequirementType::ET_LINELINEPERPENDICULAR);
    EXPECT_EQ(desc.objectIds.size(), 2);
    EXPECT_FALSE(desc.param.has_value());
}

TEST(RequirementDescriptorTest, LineLineAngle) {
    auto desc = RequirementDescriptor::lineLineAngle(ID(19), ID(20), 45.0);
    
    EXPECT_EQ(desc.type, RequirementType::ET_LINELINEANGLE);
    EXPECT_EQ(desc.objectIds.size(), 2);
    EXPECT_DOUBLE_EQ(desc.param.value(), 45.0);
}

TEST(RequirementDescriptorTest, Vertical) {
    auto desc = RequirementDescriptor::vertical(ID(21));
    
    EXPECT_EQ(desc.type, RequirementType::ET_VERTICAL);
    EXPECT_EQ(desc.objectIds.size(), 1);
    EXPECT_EQ(desc.objectIds[0], ID(21));
    EXPECT_FALSE(desc.param.has_value());
}

TEST(RequirementDescriptorTest, Horizontal) {
    auto desc = RequirementDescriptor::horizontal(ID(22));
    
    EXPECT_EQ(desc.type, RequirementType::ET_HORIZONTAL);
    EXPECT_EQ(desc.objectIds.size(), 1);
    EXPECT_EQ(desc.objectIds[0], ID(22));
    EXPECT_FALSE(desc.param.has_value());
}

TEST(RequirementDescriptorTest, ArcCenterOnPerpendicular) {
    auto desc = RequirementDescriptor::arcCenterOnPerpendicular(ID(23));
    
    EXPECT_EQ(desc.type, RequirementType::ET_ARCCENTERONPERPENDICULAR);
    EXPECT_EQ(desc.objectIds.size(), 1);
    EXPECT_EQ(desc.objectIds[0], ID(23));
    EXPECT_FALSE(desc.param.has_value());
}

// ==================== Validation - Valid Cases ====================

TEST(RequirementDescriptorTest, ValidateTwoObjectTypes) {
    // All types requiring 2 objects should pass validation
    auto desc1 = RequirementDescriptor::pointLineDist(ID(1), ID(2), 10.0);
    EXPECT_TRUE(desc1.validate());
    
    auto desc2 = RequirementDescriptor::pointOnLine(ID(1), ID(2));
    EXPECT_TRUE(desc2.validate());
    
    auto desc3 = RequirementDescriptor::pointPointDist(ID(1), ID(2), 10.0);
    EXPECT_TRUE(desc3.validate());
    
    auto desc4 = RequirementDescriptor::pointOnPoint(ID(1), ID(2));
    EXPECT_TRUE(desc4.validate());
    
    auto desc5 = RequirementDescriptor::lineLineParallel(ID(1), ID(2));
    EXPECT_TRUE(desc5.validate());
    
    auto desc6 = RequirementDescriptor::lineLinePerpendicular(ID(1), ID(2));
    EXPECT_TRUE(desc6.validate());
    
    auto desc7 = RequirementDescriptor::lineLineAngle(ID(1), ID(2), 90.0);
    EXPECT_TRUE(desc7.validate());
}

TEST(RequirementDescriptorTest, ValidateOneObjectTypes) {
    // All types requiring 1 object should pass validation
    auto desc1 = RequirementDescriptor::vertical(ID(1));
    EXPECT_TRUE(desc1.validate());
    
    auto desc2 = RequirementDescriptor::horizontal(ID(1));
    EXPECT_TRUE(desc2.validate());
    
    auto desc3 = RequirementDescriptor::arcCenterOnPerpendicular(ID(1));
    EXPECT_TRUE(desc3.validate());
}

// ==================== Validation - Invalid Cases ====================

TEST(RequirementDescriptorTest, ValidateThrowsWrongObjectCount_TwoExpected) {
    // Types requiring 2 objects, but given 1
    RequirementDescriptor desc(
        RequirementType::ET_POINTPOINTDIST,
        {ID(1)},  // Only 1 ID
        50.0
    );
    
    EXPECT_THROW(desc.validate(), std::invalid_argument);
}

TEST(RequirementDescriptorTest, ValidateThrowsWrongObjectCount_OneExpected) {
    // Types requiring 1 object, but given 2
    RequirementDescriptor desc(
        RequirementType::ET_VERTICAL,
        {ID(1), ID(2)}  // 2 IDs instead of 1
    );
    
    EXPECT_THROW(desc.validate(), std::invalid_argument);
}

TEST(RequirementDescriptorTest, ValidateThrowsMissingParam) {
    // Types requiring param, but param not provided
    RequirementDescriptor desc(
        RequirementType::ET_POINTPOINTDIST,
        {ID(1), ID(2)}
        // No param!
    );
    
    EXPECT_THROW(desc.validate(), std::invalid_argument);
}

TEST(RequirementDescriptorTest, ValidateThrowsMissingParamPointLineDist) {
    RequirementDescriptor desc(
        RequirementType::ET_POINTLINEDIST,
        {ID(1), ID(2)}
    );
    
    EXPECT_THROW(desc.validate(), std::invalid_argument);
}

TEST(RequirementDescriptorTest, ValidateThrowsMissingParamLineCircleDist) {
    RequirementDescriptor desc(
        RequirementType::ET_LINECIRCLEDIST,
        {ID(1), ID(2)}
    );
    
    EXPECT_THROW(desc.validate(), std::invalid_argument);
}

TEST(RequirementDescriptorTest, ValidateThrowsMissingParamLineLineAngle) {
    RequirementDescriptor desc(
        RequirementType::ET_LINELINEANGLE,
        {ID(1), ID(2)}
    );
    
    EXPECT_THROW(desc.validate(), std::invalid_argument);
}

TEST(RequirementDescriptorTest, ValidateEmptyObjectIds) {
    RequirementDescriptor desc(
        RequirementType::ET_POINTONPOINT,
        {}  // Empty
    );
    
    EXPECT_THROW(desc.validate(), std::invalid_argument);
}

TEST(RequirementDescriptorTest, ValidateTooManyObjects) {
    RequirementDescriptor desc(
        RequirementType::ET_POINTONPOINT,
        {ID(1), ID(2), ID(3)}  // 3 instead of 2
    );
    
    EXPECT_THROW(desc.validate(), std::invalid_argument);
}

// ==================== Edge Cases ====================

TEST(RequirementDescriptorTest, ZeroDistance) {
    auto desc = RequirementDescriptor::pointPointDist(ID(1), ID(2), 0.0);
    EXPECT_TRUE(desc.validate());
    EXPECT_DOUBLE_EQ(desc.param.value(), 0.0);
}

TEST(RequirementDescriptorTest, NegativeDistance) {
    // Note: validation doesn't check for negative distance
    auto desc = RequirementDescriptor::pointPointDist(ID(1), ID(2), -10.0);
    EXPECT_TRUE(desc.validate());  // Still valid structurally
    EXPECT_DOUBLE_EQ(desc.param.value(), -10.0);
}

TEST(RequirementDescriptorTest, SameIdTwice) {
    // Same ID for both objects (might be invalid logically, but not structurally)
    auto desc = RequirementDescriptor::pointOnPoint(ID(5), ID(5));
    EXPECT_TRUE(desc.validate());  // Structurally valid
    EXPECT_EQ(desc.objectIds[0], desc.objectIds[1]);
}

TEST(RequirementDescriptorTest, LargeIds) {
    auto desc = RequirementDescriptor::pointPointDist(
        ID(999999), 
        ID(1000000), 
        12345.6789
    );
    
    EXPECT_TRUE(desc.validate());
    EXPECT_EQ(desc.objectIds[0], ID(999999));
    EXPECT_EQ(desc.objectIds[1], ID(1000000));
}
