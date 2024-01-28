#ifndef lox_exit_codes_h
#define lox_exit_codes_h

// These are bsd exit codes used in clox.
// c.f. https://man.freebsd.org/cgi/man.cgi?query=sysexits

#define EX_USAGE 64		// incorrect command usage
#define EX_DATAERR 65	// incorrect input data
#define EX_SOFTWARE 70	// internal software error
#define EX_IOERR 74		// error during i/o operation

#endif
