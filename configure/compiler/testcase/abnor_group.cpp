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
#include "abnor_group.h"

static TestAbnorGroup suite_TestAbnorGroup;

static CxxTest::List Tests_TestAbnorGroup = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_TestAbnorGroup( "abnor_group.h", 20, "TestAbnorGroup", suite_TestAbnorGroup, Tests_TestAbnorGroup );

static class TestDescription_TestAbnorGroup_test_abnor_group : public CxxTest::RealTestDescription {
public:
 TestDescription_TestAbnorGroup_test_abnor_group() : CxxTest::RealTestDescription( Tests_TestAbnorGroup, suiteDescription_TestAbnorGroup, 23, "test_abnor_group" ) {}
 void runTest() { suite_TestAbnorGroup.test_abnor_group(); }
} testDescription_TestAbnorGroup_test_abnor_group;

#include <cxxtest/Root.cpp>
