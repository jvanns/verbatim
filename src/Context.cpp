/*
 * vim: set smartindent autoindent expandtab tabstop=4 shiftwidth=4:
 */

// Interface
#include "Context.hpp"

namespace verbatim {

Context::Context(size_t concurrency) : threads(concurrency), db(tv, threads)
{
}

} // verbatim
