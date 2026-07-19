#pragma once

#include "Layout/LayoutNode.h"
#include "Layout/FlexLayoutEngine.h"
#include "Layout/LayoutDumper.h"
#include "Graphics/RenderCommandBuilder.h"

#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace vkapp::Layout::Test {

struct TestResult {
    std::string name;
    bool passed = false;
    std::string dump;
    std::string error;
};

class FlexTestRunner {
public:
    using TestFunc = std::function<TestResult()>;

    void addTest(const std::string& name, TestFunc func);
    int runAll();

private:
    std::vector<std::pair<std::string, TestFunc>> m_tests;
};

inline void FlexTestRunner::addTest(const std::string& name, TestFunc func)
{
    m_tests.emplace_back(name, std::move(func));
}

inline bool fuzzyEqual(float a, float b, float epsilon = 0.5f)
{
    return std::fabs(a - b) < epsilon;
}

inline bool assertRect(const LayoutNode& node, float expectedX, float expectedY, float expectedWidth, float expectedHeight, std::string& outError)
{
    if (!fuzzyEqual(node.computedRect.x, expectedX)) {
        outError = "Expected x=" + std::to_string(expectedX) + " got " + std::to_string(node.computedRect.x);
        return false;
    }
    if (!fuzzyEqual(node.computedRect.y, expectedY)) {
        outError = "Expected y=" + std::to_string(expectedY) + " got " + std::to_string(node.computedRect.y);
        return false;
    }
    if (!fuzzyEqual(node.computedRect.width, expectedWidth)) {
        outError = "Expected width=" + std::to_string(expectedWidth) + " got " + std::to_string(node.computedRect.width);
        return false;
    }
    if (!fuzzyEqual(node.computedRect.height, expectedHeight)) {
        outError = "Expected height=" + std::to_string(expectedHeight) + " got " + std::to_string(node.computedRect.height);
        return false;
    }
    return true;
}

inline bool assertNoIssues(const LayoutNode& node, std::string& outError)
{
    std::string issues = LayoutDumper::detectIssues(node);
    if (!issues.empty()) {
        outError = "Node '" + node.name + "' has issues: " + issues;
        return false;
    }
    return true;
}

inline int FlexTestRunner::runAll()
{
    int passed = 0;
    for (const auto& [name, func] : m_tests) {
        TestResult result = func();
        result.name = name;
        result.dump = LayoutDumper::dumpTree(result.dump.empty() ? LayoutNode{} : LayoutNode{});

        if (result.passed) {
            ++passed;
            std::cout << "[PASS] " << name << "\n";
        } else {
            std::cout << "[FAIL] " << name << "\n";
            std::cout << "  Error: " << result.error << "\n";
        }
    }
    std::cout << "\nResults: " << passed << "/" << m_tests.size() << " passed\n";
    return passed == static_cast<int>(m_tests.size()) ? 0 : 1;
}

int runAllTests();

} // namespace vkapp::Layout::Test
