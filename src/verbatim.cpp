/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// verbatim
#include "Context.hpp"
#include "utility/tools.hpp"
#include "utility/Timer.hpp"

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
using verbatim::utility::Timer;
using verbatim::utility::str2int;

namespace {

void
print_usage(const char *program_name)
{
    cerr << "usage: "
         << program_name << " [options] " << "<db file> <music directory>\n\n";
    cerr << "Options (defaults in parenthesis):\n"
         << "-h/--help             "
         << "Print this help message you're reading, then terminate\n"
         << "-v/--verbose          "
         << "Print noisy verbose messages to stdout (false)\n"
         << "-c/--concurrency <N>  "
         << "No. of worker threads to run in parallel (2)\n";
}

} // anonymous

int main(int argc, char *argv[])
{
    /*
     * Default values for optional flags - read help message in print_usage()!
     */
    bool verbose = false;
    uint16_t threads = 2;
    const char *db_path = NULL, *music_path = NULL;

    try {
        int option_index, c = 0;
        const char *short_options = "+hvc:";
        struct option const long_options[] = {
            {"help", 0, NULL, 'h'},
            {"verbose", 0, NULL, 'v'},
            {"concurrency", 1, NULL, 'c'},
            {NULL, 0, NULL, 0}
        };

        do {
            c = getopt_long(argc,argv,short_options,long_options,&option_index);

            switch (c) {
                case 'v':
                    verbose = true;
                    break;
                case 'c': {
                        static const uint16_t min = 1, max = 256;
                        threads = str2int<uint16_t>(optarg, &min, &max);
                    }
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

    if ((argc - optind) < 2) {
        print_usage(argv[0]);
        return 1;
    }

    db_path = argv[optind++];
    music_path = argv[optind];

    Context c(threads);

    c.database().open(db_path);
    c.traverser().scan(music_path);

    c.wait();
    c.print_metrics(cout);

    return 0;
}

