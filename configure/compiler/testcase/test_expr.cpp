/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#define _CXXTEST_HAVE_STD
#define _CXXTEST_HAVE_EH
#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <cxxtest/ErrorPrinter.h>

int main() {
 return CxxTest::ErrorPrinter().run();
}
#include "test_expr.h"

static test_normal suite_test_normal;

static CxxTest::List Tests_test_normal = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_test_normal( "test_expr.h", 29, "test_normal", suite_test_normal, Tests_test_normal );

static class TestDescription_test_normal_test_test1 : public CxxTest::RealTestDescription {
public:
 TestDescription_test_normal_test_test1() : CxxTest::RealTestDescription( Tests_test_normal, suiteDescription_test_normal, 32, "test_test1" ) {}
 void runTest() { suite_test_normal.test_test1(); }
} testDescription_test_normal_test_test1;

static class TestDescription_test_normal_test_test2 : public CxxTest::RealTestDescription {
public:
 TestDescription_test_normal_test_test2() : CxxTest::RealTestDescription( Tests_test_normal, suiteDescription_test_normal, 45, "test_test2" ) {}
 void runTest() { suite_test_normal.test_test2(); }
} testDescription_test_normal_test_test2;

static class TestDescription_test_normal_test_test3 : public CxxTest::RealTestDescription {
public:
 TestDescription_test_normal_test_test3() : CxxTest::RealTestDescription( Tests_test_normal, suiteDescription_test_normal, 56, "test_test3" ) {}
 void runTest() { suite_test_normal.test_test3(); }
} testDescription_test_normal_test_test3;

static class TestDescription_test_normal_test_test4 : public CxxTest::RealTestDescription {
public:
 TestDescription_test_normal_test_test4() : CxxTest::RealTestDescription( Tests_test_normal, suiteDescription_test_normal, 67, "test_test4" ) {}
 void runTest() { suite_test_normal.test_test4(); }
} testDescription_test_normal_test_test4;

static class TestDescription_test_normal_test_fail : public CxxTest::RealTestDescription {
public:
 TestDescription_test_normal_test_fail() : CxxTest::RealTestDescription( Tests_test_normal, suiteDescription_test_normal, 83, "test_fail" ) {}
 void runTest() { suite_test_normal.test_fail(); }
} testDescription_test_normal_test_fail;

static class TestDescription_test_normal_test_double2 : public CxxTest::RealTestDescription {
public:
 TestDescription_test_normal_test_double2() : CxxTest::RealTestDescription( Tests_test_normal, suiteDescription_test_normal, 95, "test_double2" ) {}
 void runTest() { suite_test_normal.test_double2(); }
} testDescription_test_normal_test_double2;

#include <cxxtest/Root.cpp>
