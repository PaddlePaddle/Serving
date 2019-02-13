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
#include "abnor_array.h"

static TestAbnorArray suite_TestAbnorArray;

static CxxTest::List Tests_TestAbnorArray = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_TestAbnorArray( "abnor_array.h", 19, "TestAbnorArray", suite_TestAbnorArray, Tests_TestAbnorArray );

static class TestDescription_TestAbnorArray_test_abnor_array : public CxxTest::RealTestDescription {
public:
 TestDescription_TestAbnorArray_test_abnor_array() : CxxTest::RealTestDescription( Tests_TestAbnorArray, suiteDescription_TestAbnorArray, 22, "test_abnor_array" ) {}
 void runTest() { suite_TestAbnorArray.test_abnor_array(); }
} testDescription_TestAbnorArray_test_abnor_array;

#include <cxxtest/Root.cpp>
