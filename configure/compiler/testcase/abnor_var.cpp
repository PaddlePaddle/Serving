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
#include "abnor_var.h"

static TestAbnorVar suite_TestAbnorVar;

static CxxTest::List Tests_TestAbnorVar = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_TestAbnorVar( "abnor_var.h", 20, "TestAbnorVar", suite_TestAbnorVar, Tests_TestAbnorVar );

static class TestDescription_TestAbnorVar_test_abnor_var : public CxxTest::RealTestDescription {
public:
 TestDescription_TestAbnorVar_test_abnor_var() : CxxTest::RealTestDescription( Tests_TestAbnorVar, suiteDescription_TestAbnorVar, 23, "test_abnor_var" ) {}
 void runTest() { suite_TestAbnorVar.test_abnor_var(); }
} testDescription_TestAbnorVar_test_abnor_var;

#include <cxxtest/Root.cpp>
