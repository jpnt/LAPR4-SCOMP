# ApplicationsFileBot (US 2001)

![ApplicationsFileBot Diagram](./svg/diagram.drawio.svg)

## Description

**US 2001 As Product Owner, I want the system to, continuously, process the files
produced by the Applications Email Bot, so that they can be imported into the system
by initiative of the Operator**

– Priority: 1

– References: See NFR12(SCOMP).

#### NFR12
The “Applications File Bot” must be developed in C and utilize processes, signals,
pipes, and exec function primitives.

A child process should be created to periodically monitor an input directory for new
files related to the 'Application' phase of the recruitment process. If new files are
detected, a signal should be sent to the parent process.

Please refer to Section 2.2.3 of the “System Specification” document for a
description of the input directory, input files, output directory, and their expected
subdirectories.

Upon receiving a signal, the parent process should distribute the new files among a
fixed number of worker child processes. Each child process will be responsible for
copying all files related to a specific candidate to its designated subdirectory in the
output directory.

Once a child has finished copying all files for a candidate, it should inform its parent
that it is ready to perform additional work. Child workers do not terminate unless they
are specifically terminated by the parent process.

Once all files for all candidates have been copied, the parent process should
generate a report file in the output directory. This report should list, for each
candidate, the name of the output subdirectory and the names of all files that were
copied.

To terminate the application, the parent process must handle the SIGINT signal.
Upon reception, it should terminate all children and wait for their termination.

The names of the input and output directories, the number of worker children, the
time interval for periodic checking of new files, etc., should be configurable. This
configuration can be achieved either through input parameters provided when
running the application or by reading from a configuration file.

Unit and integration tests are highly valued.

#### Section 2.2.3 - Applications

Candidates submit their applications for job openings by email.

There is an Applications Email Bot (outside of scope for this project) that is continuously
processing these emails. The Bot processes the emails and produces (in a predefined folder)
the following content/files (using the same file prefix for files of the same application):

+ A text file with the contents of the email

+ A file for each file attached to the email (usually PDF files)

+ A text file with the contents of each file attached to the email (processed by an OCR tool)

+ A text file with the data of the application and candidate, with at least:
  + job reference
  + email of the candidate
  + name of the candidate
  + phone number of the candidate

There is a second bot application, named **Applications File Bot**, that processes these files for
integration in the system. The Applications File Bot is continuously monitoring the previous
referred folder for new applications to be processed.

The Bot should **copy the files for a shared folder**. This shared folder should be organized by job
reference (top folders) and then by application (sub folder inside the job reference folder).

The Bot should produce a **text report** of all the processed applications (including applications for
job references and files available).

The Operator of the Backoffice will import the files produced by the Applications File Bot and 
register the applications, creating candidates that dot not exist in the system.

---

## Processes

![ApplicationsFileBot Processes](./svg/processes.drawio.svg)

---

## Monitoring Input Directory: Polling vs Inotify

Polling means regularly checking the state of something else, to see whether something has changed.
Using inotify can be a more efficient and robust way to monitor directories for changes compared to 
periodically polling the directory. However polling is easier and may not be required for our system.
Also, to note, inotify is Linux specific, whereas polling works on almost any OS.

This article sums up everything very well: https://helpful.knobs-dials.com/index.php/File_polling,_event_notification,_and_asynchronous_IO

---

## Error Handling

`die()` provides a unified way to handle errors, combining custom error messages
with system error information when needed. It simplifies error handling and 
ensures consistent behavior across the codebase, making debugging easier.

Example on how it can be used: https://git.suckless.org/ii/commit/71c1e50da069b17e9e5073b32e83a9be8672b954.html
