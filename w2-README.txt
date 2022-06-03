Documentation for Warmup Assignment 2
=====================================

+------------------------+
| BUILD & RUN (Required) |
+------------------------+

Replace "(Comments?)" below with the command the grader should use to compile
your program (it should simply be "make" or "make warmup2"; minor variation is
also fine).

    To compile your code, the grader should type: (Comments?)

If you have additional instruction for the grader, replace "(Comments?)" with your
instruction (or with the word "none" if you don't have additional instructions):

    Additional instructions for building/running this assignment: (Comments?)

In the above, it may be a good idea to tell the grader how many virtual CPUs to
use when grading.  The only acceptable number of virtual CPUs are either 1 or 2.

+-------------------------+
| SELF-GRADING (Required) |
+-------------------------+

Replace each "?" below with a numeric value:

(A) Basic running of the code : ? out of 90 pts

(B) Basic handling of <Cntrl+C> : ? out of 10 pts
    (Please note that if you entered 0 for (B) above, it means that you did not
    implement <Cntrl+C>-handling and the grader will simply deduct 12 points for
    not handling <Cntrl+C>.  But if you enter anything other than 0 above, it
    would mean that you have handled <Cntrl+C>.  In this case, the grader will
    grade accordingly and it is possible that you may end up losing more than
    12 points for not handling <Cntrl+C> correctly!)

Missing required section(s) in README file : -? pts
Submission in wrong file format : -? pts
Submitted binary file : -? pts
Cannot compile : -? pts
Compiler warnings : -? pts
"make clean" : -? pts
Segmentation faults : -? pts
Separate compilation : -? pts

Delay trace printing : -? pts
Using busy-wait : -? pts
Trace output :
    1) regular packets: -? pts
    2) dropped packets: -? pts
    3) token arrival (dropped or not dropped): -? pts
    4) monotonically non-decreasing timestamps: -? pts
    5) timestamp resolution: -? pts
Remove packets : -? pts
Statistics output :
    1) inter-arrival time : -? pts
    2) service time : -? pts
    3) number of packets in Q1 : -? pts
    4) number of packets in Q2 : -? pts
    5) number of packets at a server : -? pts
    6) time in system : -? pts
    7) standard deviation for time in system : -? pts
    8) drop probability : -? pts
Output bad format : -? pts
Output wrong precision for statistics (must be 6 or more significant digits) : -? pts
Statistics in wrong units (time related statistics must be in seconds) : -? pts
Large total number of packets test : -? pts
Large total number of packets with high arrival rate test : -? pts
Dropped tokens and packets test : -? pts
<Ctrl+C> is handled but statistics are way off : -? pts
Cannot stop (or take too long to stop) packet arrival thread when required : -? pts
Cannot stop (or take too long to stop) token depositing thread when required : -? pts
Cannot stop (or takes too long to stop) a server thread when required : -? pts
Not using condition variables : -? pts
Synchronization check : -? pts
Deadlocks/freezes : -? pts
Bad commandline or command : -? pts

+---------------------------------+
| BUGS / TESTS TO SKIP (Required) |
+---------------------------------+

Are there are any tests mentioned in the grading guidelines test suite that you
know that it's not working and you don't want the grader to run it at all so you
won't get extra deductions, please replace "(Comments?)" below with your list.
(Of course, if the grader won't run such tests in the plus points section, you
will not get plus points for them; if the garder won't run such tests in the
minus points section, you will lose all the points there.)  If there's nothing
the grader should skip, please replace "(Comments?)" with "none".

Please skip the following tests: (Comments?)

+--------------------------------------------------------------------------------------------+
| ADDITIONAL INFORMATION FOR GRADER (Optional, but the grader should read what you add here) |
+--------------------------------------------------------------------------------------------+

+-----------------------------------------------+
| OTHER (Optional) - Not considered for grading |
+-----------------------------------------------+

Comments on design decisions: (Comments?)

