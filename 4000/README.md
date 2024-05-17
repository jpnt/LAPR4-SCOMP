# Top20MostReferemcedWords (US 4000)

## Description

**US 4000 As a Customer Manager, when displaying the candidate data, I expect the
system to present a top 20 list of the most frequently referenced words from files
uploaded by a candidate. Additionally, I require a comprehensive list of the files in
which these words appear**

– Priority: 1

– References: See NFR14(SCOMP).

#### NFR14

This US should be developed in Java and utilize threads along with the
synchronization mechanisms outlined in SCOMP.

Utilization of any Java concurrency mechanisms beyond those covered in class,
including thread pools, streams, etc., is strictly prohibited. Similarly, avoid employing
data types that inherently support concurrent access.

The objective is to compile a list of the top 20 most frequently referenced words from
files submitted by a candidate. This list should include the number of occurrences of
each word and the corresponding files in which they appear.

Multiple approaches can be taken for processing files: either assigning a thread per
file or allowing multiple threads to process a single file. Nevertheless,
synchronization among threads is imperative to guarantee the accuracy of the
obtained results.

Unit and integration tests are highly valued.
