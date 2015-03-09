/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// verbatim
#include "Context.hpp"

// libstdc++
#include <iostream>
#include <exception>

// libc
#include <getopt.h>

// libstdc++
using std::cout;
using std::cerr;
using std::endl;
using std::exception;

// verbatim
using verbatim::Context;

namespace {

void
print_usage(const char *program_name)
{
    cerr << "usage: "
         << program_name << " [options] " << "<db file>\n\n";
    cerr << "Options (defaults in parenthesis):\n"
         << "-h/--help             "
         << "Print this help message you're reading, then terminate\n"
         << "-v/--verbose          "
         << "Print noisy verbose messages to stdout (false)\n";
}

} // anonymous

int main(int argc, char *argv[])
{
    /*
     * Default values for optional flags - read help message in print_usage()!
     */
    bool verbose = false;
    const char *db_path = NULL;

    try {
        int option_index, c = 0;
        const char *short_options = "+hv";
        struct option const long_options[] = {
            {"help", 0, NULL, 'h'},
            {"verbose", 0, NULL, 'v'},
            {NULL, 0, NULL, 0}
        };

        do {
            c = getopt_long(argc,argv,short_options,long_options,&option_index);

            switch (c) {
                case 'v':
                    verbose = true;
                    break;
                case 'h':
                    print_usage(argv[0]);
                    return 1;
            }
        } while (c != -1);
    } catch (const exception &e) {
        cerr << e.what() << endl;
        return 1;
    }

    if ((argc - optind) < 1) {
        print_usage(argv[0]);
        return 1;
    }

    db_path = argv[optind++];

    Context c(0);
    size_t count = 0;

    c.database().open(db_path);
    count = c.database().list_entries(cout);

    if (verbose)
        cout << "Total #entries: " << count << endl;

    return 0;
}

