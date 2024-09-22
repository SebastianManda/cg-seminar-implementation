
//              Copyright Catch2 Authors
// Distributed under the Boost Software License, Version 1.0.
//   (See accompanying file LICENSE_1_0.txt or copy at
//        https://www.boost.org/LICENSE_1_0.txt)

// SPDX-License-Identifier: BSL-1.0
#ifndef CATCH_REPORTER_STREAMING_BASE_HPP_INCLUDED
#define CATCH_REPORTER_STREAMING_BASE_HPP_INCLUDED

#include <catch2/interfaces/catch_interfaces_reporter.hpp>

#include <iosfwd>
#include <string>
#include <vector>

namespace Catch {

    class StreamingReporterBase : public IStreamingReporter {
    public:
        StreamingReporterBase( ReporterConfig const& _config ):
            IStreamingReporter( _config.fullConfig() ),
            m_stream( _config.stream() ) {}


        ~StreamingReporterBase() override;

        void benchmarkPreparing( StringRef ) override {}
        void benchmarkStarting( BenchmarkInfo const& ) override {}
        void benchmarkEnded( BenchmarkStats<> const& ) override {}
        void benchmarkFailed( StringRef ) override {}

        void fatalErrorEncountered( StringRef /*error*/ ) override {}
        void noMatchingTestCases( StringRef /*unmatchedSpec*/ ) override {}
        void reportInvalidTestSpec( StringRef /*invalidArgument*/ ) override {}

        void testRunStarting( TestRunInfo const& _testRunInfo ) override;

        void testCaseStarting(TestCaseInfo const& _testInfo) override  {
            currentTestCaseInfo = &_testInfo;
        }
        void testCasePartialStarting( TestCaseInfo const&, uint64_t ) override {}
        void sectionStarting(SectionInfo const& _sectionInfo) override {
            m_sectionStack.push_back(_sectionInfo);
        }

        void assertionStarting( AssertionInfo const& ) override {}
        void assertionEnded( AssertionStats const& ) override {}

        void sectionEnded(SectionStats const& /* _sectionStats */) override {
            m_sectionStack.pop_back();
        }
        void testCasePartialEnded( TestCaseStats const&, uint64_t ) override {}
        void testCaseEnded(TestCaseStats const& /* _testCaseStats */) override {
            currentTestCaseInfo = nullptr;
        }
        void testRunEnded( TestRunStats const& /* _testRunStats */ ) override;

        void skipTest(TestCaseInfo const&) override {
            // Don't do anything with this by default.
            // It can optionally be overridden in the derived class.
        }

        void listReporters( std::vector<ReporterDescription> const& descriptions ) override;
        void listTests( std::vector<TestCaseHandle> const& tests ) override;
        void listTags( std::vector<TagInfo> const& tags ) override;

    protected:
        //! Stream that the reporter output should be written to
        std::ostream& m_stream;

        TestRunInfo currentTestRunInfo{ "test run has not started yet"_sr };
        TestCaseInfo const* currentTestCaseInfo = nullptr;

        //! Stack of all _active_ sections in the _current_ test case
        std::vector<SectionInfo> m_sectionStack;
    };

} // end namespace Catch

#endif // CATCH_REPORTER_STREAMING_BASE_HPP_INCLUDED
