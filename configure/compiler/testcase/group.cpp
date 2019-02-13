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
#include "group.h"

static TestGroup suite_TestGroup;

static CxxTest::List Tests_TestGroup = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_TestGroup( "group.h", 20, "TestGroup", suite_TestGroup, Tests_TestGroup );

static class TestDescription_TestGroup_testgroup : public CxxTest::RealTestDescription {
public:
 TestDescription_TestGroup_testgroup() : CxxTest::RealTestDescription( Tests_TestGroup, suiteDescription_TestGroup, 23, "testgroup" ) {}
 void runTest() { suite_TestGroup.testgroup(); }
} testDescription_TestGroup_testgroup;

#include <cxxtest/Root.cpp>
