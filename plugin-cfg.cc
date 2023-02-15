#include <iostream>
#include <fstream>

// This is the first gcc header to be included
#include "gcc-plugin.h"
#include "plugin-version.h"

#include "tree-pass.h"
#include "context.h"
#include "function.h"
#include "tree.h"
#include "tree-ssa-alias.h"
#include "internal-fn.h"
#include "is-a.h"
#include "predict.h"
#include "basic-block.h"
#include "gimple-expr.h"
#include "gimple.h"
#include "gimple-pretty-print.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"

// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;

static struct plugin_info custom_cfg_plugin_info = { "1.0", "Plugin to generate CFG in graphviz format" };
std::ofstream outfile;

namespace
{
    const pass_data custom_cfg_pass_data = 
    {
        GIMPLE_PASS,
        "custom_cfg_pass",        /* name */
        OPTGROUP_NONE,          /* optinfo_flags */
        TV_NONE,                /* tv_id */
        PROP_gimple_any,        /* properties_required */
        0,                      /* properties_provided */
        0,                      /* properties_destroyed */
        0,                      /* todo_flags_start */
        0                       /* todo_flags_finish */
    };

    struct custom_cfg_pass : gimple_opt_pass
    {
        custom_cfg_pass(gcc::context *ctx) : gimple_opt_pass(custom_cfg_pass_data, ctx)
        {
            outfile << "Digraph CFG {\n";
        }

        virtual unsigned int execute(function *fun) override
        {
            basic_block bb;

            outfile << "subgraph func_" << fun << " {\n";

            FOR_ALL_BB_FN(bb, fun)
            {
                gimple_bb_info *bb_info = &bb->il.gimple;

                outfile << "bb_" << fun << "_" << bb->index << "[label=\"";
                if (bb->index == 0)
                {
                    outfile << "ENTRY: "
                        << function_name(fun) << "\n"
                        << (LOCATION_FILE(fun->function_start_locus) ? : "<unknown>") << ":" << LOCATION_LINE(fun->function_start_locus);
                }
                else if (bb->index == 1)
                {
                    outfile << "EXIT: "
                        << function_name(fun) << "\n"
                        << (LOCATION_FILE(fun->function_end_locus) ? : "<unknown>") << ":" << LOCATION_LINE(fun->function_end_locus);
                }
                else
                {
                    print_gimple_seq(stderr, bb_info->seq , 0, static_cast<dump_flags_t>(0));
                }
                outfile << "\"];\n";

                edge e;
                edge_iterator ei;

                FOR_EACH_EDGE (e, ei, bb->succs)
                {
                    basic_block dest = e->dest;
                    outfile << "bb_" << fun << "_" << bb->index << " -> bb_" << fun << "_" << dest->index << ";\n";
                }
            }

            outfile << "}\n";

            // Nothing special todo
            return 0;
        }

        virtual custom_cfg_pass* clone() override
        {
            // We do not clone ourselves
            return this;
        }
    };
}

namespace {

    void finish_gcc(void *gcc_data, void *user_data)
    {
        outfile << "}\n";
    }

}

int plugin_init (struct plugin_name_args *plugin_info,
		struct plugin_gcc_version *version)
{
	outfile.open("cfg.dot");

	if (!plugin_default_version_check (version, &gcc_version))
    {
        outfile << "This GCC plugin version: " << GCCPLUGIN_VERSION_MAJOR << "." << GCCPLUGIN_VERSION_MINOR << "\n";
		return 1;
    }

    register_callback(plugin_info->base_name,
            /* event */ PLUGIN_INFO,
            /* callback */ NULL,
            /* user_data */ &custom_cfg_plugin_info);

    struct register_pass_info pass_info;

    pass_info.pass = new custom_cfg_pass(g);
    pass_info.reference_pass_name = "ssa";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);
    register_callback (plugin_info->base_name, PLUGIN_FINISH, finish_gcc, NULL);

    return 0;
}