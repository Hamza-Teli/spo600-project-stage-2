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
#include "function.h"
#include "basic-block.h"
#include <string>
#include <map>
#include <vector>

namespace {

struct FunctionInfo {
    std::string baseName;
    std::string suffix;
    function *funcPtr;
};

std::map<std::string, std::string> resolverMap;
std::map<std::string, std::vector<FunctionInfo>> variantMap;

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
    pass_hteli1(gcc::context *ctxt)
        : gimple_opt_pass(pass_data_hteli1, ctxt) {}

    // The gate function 
    bool gate(function *) final override {
        return true;
    }

    // The execute function: this is where the magic happens
    unsigned int execute(function *func) final override {

        // use the IDENTIFIER_POINTER to get the function name
        std::string functionName =  IDENTIFIER_POINTER(DECL_NAME(func->decl));
        // if (!functionName) {
        //     return 0;
        // }
        
        // Instantiate base name and suffix
        std::string baseName;
        std::string suffix;

        size_t dot = functionName.find('.');
        if (dot == std::string::npos) {
            baseName = functionName;
            suffix = "default";
        } else {
            baseName = functionName.substr(0, dot);
            suffix = functionName.substr(dot + 1);
        }

        if (suffix == "resolver") {
            resolverMap[baseName] = functionName;
            if (dump_file)
                fprintf(dump_file, "[Resolver] Found: %s\n", baseName.c_str());
        } else {
            variantMap[baseName].push_back({baseName, suffix, func});
        }

        // Check if this is the last function in the unit
        static bool done = false;
        if (!done) {
            done = true;
            analyze_variants();
        }

        return 0;
    }

private:

    void analyze_variants() {
        for (const auto &entry : variantMap) {
            const std::string &baseName = entry.first;
            const auto &variants = entry.second;

            if (resolverMap.find(baseName) == resolverMap.end()) continue;
            if (variants.size() != 2) continue;

            function *f1 = variants[0].funcPtr;
            function *f2 = variants[1].funcPtr;

            if (!f1 || !f2) {
                if (dump_file) fprintf(dump_file, "[Error] Null function pointer for base: %s\n", baseName.c_str());
                continue;
            }

            size_t bb1 = get_number_of_basic_blocks(f1);
            size_t bb2 = get_number_of_basic_blocks(f2);
            size_t g1 = count_gimple(f1);
            size_t g2 = count_gimple(f2);
            bool sameGimple = compare_gimple_structure(f1, f2);

            if (dump_file) {
                fprintf(dump_file, "\n[Base] %s\n", baseName.c_str());
                fprintf(dump_file, "BB Count: %zu vs %zu\n", bb1, bb2);
                fprintf(dump_file, "GIMPLE Count: %zu vs %zu\n", g1, g2);
                fprintf(dump_file, "GIMPLE Match: %s\n", sameGimple ? "YES" : "NO");
                fprintf(dump_file, "%s: %s\n", sameGimple && bb1 == bb2 && g1 == g2 ? "PRUNE" : "NOPRUNE", baseName.c_str());
            }
        }
    }

    // This function just counts the basic blocks
    size_t get_number_of_basic_blocks(function *f) {

        // Instantiate counter
        size_t count = 0;

        basic_block bb;
        FOR_EACH_BB_FN(bb, f) {
            count++;
        }
        return count;
    }

    // This function counts the gimple statements
    size_t count_gimple(function *f) {
        size_t count = 0;
        basic_block bb;
        FOR_EACH_BB_FN(bb, f) {
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi)) {
                count++;
            }
        }
        return count;
    }

    bool compare_gimple_structure(function *f1, function *f2) {
        std::vector<gimple *> stmts1, stmts2;
        basic_block bb;

        FOR_EACH_BB_FN(bb, f1)
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
                stmts1.push_back(gsi_stmt(gsi));

        FOR_EACH_BB_FN(bb, f2)
            for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
                stmts2.push_back(gsi_stmt(gsi));

        if (stmts1.size() != stmts2.size()) return false;

        for (size_t i = 0; i < stmts1.size(); ++i) {
            if (gimple_code(stmts1[i]) != gimple_code(stmts2[i]))
                return false;
            if (gimple_num_ops(stmts1[i]) != gimple_num_ops(stmts2[i]))
                return false;
        }
        return true;
    }
};

} // namespace

gimple_opt_pass *make_pass_hteli1(gcc::context *ctxt) {
    return new pass_hteli1(ctxt);
}
