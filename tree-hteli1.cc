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
#include <cstdio>



// Namespace <--- This section I learned from SPO600 Week 7 - Class 1 Lecture from Professor Chris Tyler
namespace{
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
            // Constructor
            pass_hteli1(gcc::context *ctxt) : gimple_opt_pass(pass_data_hteli1, ctxt) {

            }

            // The gate function 
            bool gate(function *) final override {
                // return 
                return true;

            }

            // The execute function: this is where the magic happens
            unsigned int execute (function *func) override {

                // Instantiate function name
                /* 
                    Inside function.cc, there's a function_name method that returns
                    the name of a function. Check out line 6454:
                    https://github.com/gcc-mirror/gcc/blob/master/gcc/function.cc

                */
                //const char* functionName = function_name(func);

                // This is where we will get started with identifying the functions that have been cloned
                if (dump_file) {

                    // Lets create a map that holds the functions
                    std::map<std::string, std::string> resolverMap;
                    
                    // Use cgraph node
                    cgraph_node *node;
                    // Lets use FOR_EACH_FUNCTION
                    FOR_EACH_FUNCTION(node) {
                        // Get the function pointer
                        function *current_function_pointer = node->get_fun();

                        // Validate
                        if (!curr_fun)
                            continue;
                        // Get the complete funciton name
                        std::string functionName(function_name(current_function_pointer));

                        // Get the dot
                        size_t dot = functionName.find('.');

                        // Check if the dot is there
                        if (dot== std::string::npos){
                            continue;
                        }
                        
                        

                        // Get the suffix first
                        std::string suffix = functionName.substr(dot + 1);

                        // Now we check that if the function has a resolver suffix, we simply store its base name
                        if (suffix == "resolver") {
                          std::string baseName = functionName.substr(0, dot);
                          resolverMap[baseName] = functionName;

                          // Show an output
                          fprintf(dump_file, "Resolver was found for base function: %s\n", baseName.c_str());
                        }

                        
                    }
                }

                // Return value
                return 0;
            }
    };
}

// This is used inside the tree-pass.h file
gimple_opt_pass* make_pass_hteli1(gcc::context *ctxt) {
    return new pass_hteli1(ctxt);
}
