#include "FlexTests.h"

#include <iostream>
#include <cstdlib>

namespace vkapp::Layout::Test {

static TestResult testRowFlexGrowSplit()
{
    TestResult result;
    result.name = "RowFlexGrowSplit";

    LayoutNode root{"root"};
    root.flex.display = Display::Flex;
    root.flex.direction = FlexDirection::Row;
    root.flex.justify = JustifyContent::FlexStart;
    root.flex.gapRow = 0.0f;
    root.isFlexContainer = true;

    LayoutNode child1{"child1"};
    child1.flex.flexGrow = 1.0f;
    child1.flex.minWidth = 0.0f;
    child1.flex.maxWidth = 1000.0f;
    child1.hasExplicitHeight = true;

    LayoutNode child2{"child2"};
    child2.flex.flexGrow = 2.0f;
    child2.flex.minWidth = 0.0f;
    child2.flex.maxWidth = 1000.0f;
    child2.hasExplicitHeight = true;

    LayoutNode child3{"child3"};
    child3.flex.flexGrow = 1.0f;
    child3.flex.minWidth = 0.0f;
    child3.flex.maxWidth = 1000.0f;
    child3.hasExplicitHeight = true;

    root.addChild(&child1);
    root.addChild(&child2);
    root.addChild(&child3);

    FlexLayoutEngine engine;
    engine.computeLayout(root, 300.0f, 100.0f);

    std::string error;
    if (!assertRect(child1, 0.0f, 0.0f, 75.0f, 100.0f, error) &&
        !assertRect(child2, 75.0f, 0.0f, 150.0f, 100.0f, error) &&
        !assertRect(child3, 225.0f, 0.0f, 75.0f, 100.0f, error)) {
        result.error = error;
        result.passed = false;
        return result;
    }

    if (!assertNoIssues(root, error)) {
        result.error = error;
        result.passed = false;
        return result;
    }

    result.passed = true;
    return result;
}

static TestResult testColumnReverse()
{
    TestResult result;
    result.name = "ColumnReverse";

    LayoutNode root{"root"};
    root.flex.display = Display::Flex;
    root.flex.direction = FlexDirection::ColumnReverse;
    root.flex.gapColumn = 0.0f;
    root.isFlexContainer = true;

    LayoutNode child1{"child1"};
    child1.flex.flexGrow = 1.0f;
    child1.flex.minHeight = 0.0f;
    child1.flex.maxHeight = 1000.0f;

    LayoutNode child2{"child2"};
    child2.flex.flexGrow = 1.0f;
    child2.flex.minHeight = 0.0f;
    child2.flex.maxHeight = 1000.0f;

    root.addChild(&child1);
    root.addChild(&child2);

    FlexLayoutEngine engine;
    engine.computeLayout(root, 200.0f, 100.0f);

    std::string error;
    if (!assertRect(child1, 0.0f, 50.0f, 200.0f, 50.0f, error) &&
        !assertRect(child2, 0.0f, 0.0f, 200.0f, 50.0f, error)) {
        result.error = error;
        result.passed = false;
        return result;
    }

    if (!assertNoIssues(root, error)) {
        result.error = error;
        result.passed = false;
        return result;
    }

    result.passed = true;
    return result;
}

static TestResult testWrapAndGap()
{
    TestResult result;
    result.name = "WrapAndGap";

    LayoutNode root{"root"};
    root.flex.display = Display::Flex;
    root.flex.direction = FlexDirection::Row;
    root.flex.wrap = FlexWrap::Wrap;
    root.flex.gapRow = 10.0f;
    root.flex.gapColumn = 10.0f;
    root.isFlexContainer = true;

    for (int i = 0; i < 4; ++i) {
        LayoutNode child{"child" + std::to_string(i)};
        child.flex.flexBasis = 100.0f;
        child.flex.flexGrow = 0.0f;
        child.flex.flexShrink = 0.0f;
        child.flex.minWidth = 0.0f;
        child.flex.maxWidth = 100.0f;
        child.hasExplicitHeight = true;
        root.addChild(&child);
    }

    FlexLayoutEngine engine;
    engine.computeLayout(root, 210.0f, 200.0f);

    std::string error;
    if (!assertRect(*root.children[0], 0.0f, 0.0f, 100.0f, 95.0f, error) &&
        !assertRect(*root.children[1], 110.0f, 0.0f, 100.0f, 95.0f, error) &&
        !assertRect(*root.children[2], 0.0f, 105.0f, 100.0f, 95.0f, error) &&
        !assertRect(*root.children[3], 110.0f, 105.0f, 100.0f, 95.0f, error)) {
        result.error = error;
        result.passed = false;
        return result;
    }

    if (!assertNoIssues(root, error)) {
        result.error = error;
        result.passed = false;
        return result;
    }

    result.passed = true;
    return result;
}

static TestResult testCenterAlignment()
{
    TestResult result;
    result.name = "CenterAlignment";

    LayoutNode root{"root"};
    root.flex.display = Display::Flex;
    root.flex.direction = FlexDirection::Row;
    root.flex.justify = JustifyContent::Center;
    root.flex.alignItems = AlignItems::Center;
    root.isFlexContainer = true;

    LayoutNode child{"child"};
    child.flex.flexBasis = 50.0f;
    child.flex.flexGrow = 0.0f;
    child.flex.flexShrink = 0.0f;
    child.hasExplicitHeight = true;
    root.addChild(&child);

    FlexLayoutEngine engine;
    engine.computeLayout(root, 200.0f, 100.0f);

    std::string error;
    if (!assertRect(child, 75.0f, 0.0f, 50.0f, 100.0f, error)) {
        result.error = error;
        result.passed = false;
        return result;
    }

    if (!assertNoIssues(root, error)) {
        result.error = error;
        result.passed = false;
        return result;
    }

    result.passed = true;
    return result;
}

static TestResult testMinMaxConstraints()
{
    TestResult result;
    result.name = "MinMaxConstraints";

    LayoutNode root{"root"};
    root.flex.display = Display::Flex;
    root.flex.direction = FlexDirection::Row;
    root.flex.gapRow = 0.0f;
    root.isFlexContainer = true;

    LayoutNode child1{"child1"};
    child1.flex.flexGrow = 1.0f;
    child1.flex.minWidth = 80.0f;
    child1.flex.maxWidth = 120.0f;

    LayoutNode child2{"child2"};
    child2.flex.flexGrow = 1.0f;
    child2.flex.minWidth = 80.0f;
    child2.flex.maxWidth = 120.0f;

    root.addChild(&child1);
    root.addChild(&child2);

    FlexLayoutEngine engine;
    engine.computeLayout(root, 500.0f, 100.0f);

    std::string error;
    if (!assertRect(child1, 0.0f, 0.0f, 120.0f, 100.0f, error) &&
        !assertRect(child2, 120.0f, 0.0f, 120.0f, 100.0f, error)) {
        result.error = error;
        result.passed = false;
        return result;
    }

    if (!assertNoIssues(root, error)) {
        result.error = error;
        result.passed = false;
        return result;
    }

    result.passed = true;
    return result;
}

static TestResult testBoxBleedDetection()
{
    TestResult result;
    result.name = "BoxBleedDetection";

    LayoutNode root{"root"};
    root.flex.display = Display::Flex;
    root.flex.direction = FlexDirection::Row;
    root.isFlexContainer = true;

    LayoutNode child{"child"};
    child.box.paddingTop = 20.0f;
    child.box.paddingBottom = 20.0f;
    child.box.borderTop = 10.0f;
    child.box.borderBottom = 10.0f;
    child.hasExplicitHeight = true;
    root.addChild(&child);

    FlexLayoutEngine engine;
    engine.computeLayout(root, 100.0f, 30.0f);

    std::string issues = LayoutDumper::detectIssues(child);
    if (issues.find("BOX_BLEED") == std::string::npos) {
        result.error = "Expected BOX_BLEED issue for child with padding+border > height";
        result.passed = false;
        return result;
    }

    result.passed = true;
    return result;
}

static TestResult testZeroSizeChild()
{
    TestResult result;
    result.name = "ZeroSizeChild";

    LayoutNode root{"root"};
    root.flex.display = Display::Flex;
    root.flex.direction = FlexDirection::Row;
    root.isFlexContainer = true;

    LayoutNode child{"child"};
    child.flex.flexBasis = 0.0f;
    child.flex.flexGrow = 0.0f;
    child.flex.flexShrink = 0.0f;
    root.addChild(&child);

    FlexLayoutEngine engine;
    engine.computeLayout(root, 100.0f, 100.0f);

    std::string error;
    if (!assertRect(child, 0.0f, 0.0f, 0.0f, 100.0f, error)) {
        result.error = error;
        result.passed = false;
        return result;
    }

    result.passed = true;
    return result;
}

int runAllTests()
{
    FlexTestRunner runner;
    runner.addTest("RowFlexGrowSplit", testRowFlexGrowSplit);
    runner.addTest("ColumnReverse", testColumnReverse);
    runner.addTest("WrapAndGap", testWrapAndGap);
    runner.addTest("CenterAlignment", testCenterAlignment);
    runner.addTest("MinMaxConstraints", testMinMaxConstraints);
    runner.addTest("BoxBleedDetection", testBoxBleedDetection);
    runner.addTest("ZeroSizeChild", testZeroSizeChild);

    return runner.runAll();
}

} // namespace vkapp::Layout::Test
