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
#include "var.h"

static TestVar suite_TestVar;

static CxxTest::List Tests_TestVar = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_TestVar( "var.h", 20, "TestVar", suite_TestVar, Tests_TestVar );

static class TestDescription_TestVar_testvar : public CxxTest::RealTestDescription {
public:
 TestDescription_TestVar_testvar() : CxxTest::RealTestDescription( Tests_TestVar, suiteDescription_TestVar, 23, "testvar" ) {}
 void runTest() { suite_TestVar.testvar(); }
} testDescription_TestVar_testvar;

#include <cxxtest/Root.cpp>
