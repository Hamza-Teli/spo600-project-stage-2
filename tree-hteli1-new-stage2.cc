/* This pass was creatd by Hamza Teli with the help of
Professor Chris Tyler's SPO600 wiki and lecture. 

This pass accomplishes the following:


*/
// These headers were taken from Professor Chris Tyler's Week 7 Lecture
// These headers were taken from Professor Chris Tyler's Week 7 Lecture
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "tree.h"
#include "gimple.h"
#include "tree-pass.h"
#include "ssa.h"
#include "tree-pretty-print.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "internal-fn.h"
#include "gimple-pretty-print.h"
#include "cgraph.h"

// Added headers
#include "gimple-ssa.h"
#include "attribs.h"
#include "pretty-print.h"
#include "tree-inline.h"
#include "intl.h"
#include "function.h"
#include "basic-block.h"
#include <string>
#include <map>
#include <vector>
#include <cstdio>


// Namespace <--- This section I learned from SPO600 Week 7 - Class 1 Lecture from Professor Chris Tyler
namespace{
    
    struct FunctionMeta {
        std::string name;          // the name of the function
        function *funcPtr;         // pointer to function
        size_t gimpleCount;        // number of gimple
        size_t basicBlockCount;    // number of basic blocks
    };

    // Then we can store it in a map
    std::map<std::string, FunctionMeta> functionMap;


    const pass_data pass_data_hteli1 = {
            GIMPLE_PASS, /* type */
            "hteli1", /* name of my pass [We will use this inside passes.def as pass_hteli1_pass]*/ 
            OPTGROUP_NONE, /* optinfo_ flags */
            TV_NONE, /* tv_id */
            PROP_cfg, /* specify that we need properties */
            0, /* the properties provided */
            0, /* the properties destroyed */
            0, /* todo_flags_start */
            0, /* todo_flags_finish */
    };

    /*
        Please refer to the instructions below from Professor CHris Tyler that helped me build the class:
        Pass Code
            Each pass provides a C++ source file which provides:
            -   A pass_data structure which defines the pass details (defined in tree-pass.h);
            -   A class which defines a gimple_opt_pass and includes the public methods gate and execute.
            -   The gate method controls whether the pass is active. It can consider multiple factors, including command-line arguments, source language, and target. This method returns a boolean.
            -   The execute method executes the pass. It accepts a pointer to a function object and will be called once for each function in the unit being compiled (which is not always what you want!).
            -   A method in the anon namespace named make_pass_name which returns a pointer to the gimple_opt_pass described above.
    */


    // This is where you identify the class
    class pass_hteli1 : public gimple_opt_pass {
        public:

            // Constructor -------------------
            pass_hteli1(gcc::context *ctxt) : gimple_opt_pass(pass_data_hteli1, ctxt) {

            }

            // The gate function 
            bool gate(function *) final override {
                // return 
                return true;

            }

            // The execute function: this is where the magic happens
            unsigned int execute (function * func) override {

                // Get function name
                std::string functionName = IDENTIFIER_POINTER(DECL_NAME(func->decl));
                
                // Get basic blocks
                size_t basicBlockCount = get_number_of_basic_blocks(func);
                // Get gimple count
                size_t gimpleCount = get_number_of_gimple_statements(func);

                functionMap[functionName] = {
                    functionName,
                    func,
                    gimpleCount,
                    basicBlockCount
                };

                // This is where we will get started with identifying the functions that have been cloned
                if (dump_file) {
                    fprintf(dump_file, "Function: %s\n", functionName.c_str());
                    fprintf(dump_file, "  Basic Blocks: %zu\n", basicBlockCount);
                    fprintf(dump_file, "  GIMPLE Stmts: %zu\n", gimpleCount);
                    fprintf(dump_file, "---------------------------\n");
                }

                // Return value
                return 0;
            }
            
            

            // Get the number of basic blocks
            size_t get_number_of_basic_blocks(function *func) {
                // Instantiate counter
                size_t count = 0;

                basic_block bb;
                FOR_EACH_BB_FN(bb, func){
                    count++;
                }

                return count;
            }

            // Get the number of statements inside the function
            size_t get_number_of_gimple_statements(function *func) {
                // Initialize count and basic block
                size_t count = 0;
                basic_block bb;


                FOR_EACH_BB_FN(bb, func) {
                    for (gimple_stmt_iterator gsi = gsi_start_bb(bb);
                        !gsi_end_p(gsi);
                        gsi_next(&gsi)) {

                        gimple *stmt = gsi_stmt(gsi);
                        if (!stmt) continue; 
                        count++;
                    }
                }

                if (dump_file) {
                    fprintf(dump_file, "Number of gimple statements is %zu\n", count);
                }

                return count;
            }
    };
}

// This is used inside the tree-pass.h file
gimple_opt_pass* make_pass_hteli1(gcc::context *ctxt) {
    return new pass_hteli1(ctxt);
}
