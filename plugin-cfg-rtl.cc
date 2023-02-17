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
#include "rtl.h"
#include "graph.h"

int plugin_is_GPL_compatible;

const char *base_name = "cfg";
const char *file_name = "cfg.dot";

namespace {
const pass_data cfg_rtl_pass_data = {
    RTL_PASS,
    "cfg_rtl_pass_data", /* name */
    OPTGROUP_NONE,          /* optinfo_flags */
    TV_NONE,                /* tv_id */
    0,                      /* properties_required */
    0,                      /* properties_provided */
    0,                      /* properties_destroyed */
    0,                      /* todo_flags_start */
    0                       /* todo_flags_finish */
};

class cfg_rtl_pass : public rtl_opt_pass {
public:
  cfg_rtl_pass(gcc::context *ctx)
      : rtl_opt_pass(cfg_rtl_pass_data, ctx) {}

  virtual unsigned int execute(function *fun) override {
    /*
    graph_t g;
    const char *name = function_name(fun);
    graph_create(&g, name);
    generate_cfg_for_function(&g, fun);
    graph_write(&g, stdout);
    */
    print_graph_cfg(base_name, fun);
    return 0;
  }

  virtual cfg_rtl_pass *clone() override {
    // We do not clone ourselves
    return this;
  }
};

} // namespace

namespace {
void finish_gcc(void *gcc_data, void *user_data) {
  std::ofstream outfile;
  outfile.open(file_name, std::fstream::app);
  outfile << "}\n";
}
} // namespace

static struct plugin_info cfg_rtl_plugin_info = {
    .version = "1.0",
    .help = "Generate CFG for each function using RTL pass.",
};

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version) {

  if (!plugin_default_version_check(version, &gcc_version)) {
    std::cerr << "This GCC plugin version: " << GCCPLUGIN_VERSION_MAJOR << "."
              << GCCPLUGIN_VERSION_MINOR << "\n";
    return 1;
  }

  std::ofstream outfile;
  outfile.open(file_name);
  outfile << "Digraph G {\n";

  struct register_pass_info pass_info;

  pass_info.pass = new cfg_rtl_pass(g);
  pass_info.reference_pass_name = "ira";
  pass_info.ref_pass_instance_number = 0;
  pass_info.pos_op = PASS_POS_INSERT_BEFORE;

  register_callback(plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL,
                    &pass_info);

  register_callback(plugin_info->base_name, PLUGIN_INFO, NULL,
                    &cfg_rtl_plugin_info);
  register_callback(plugin_info->base_name, PLUGIN_FINISH, finish_gcc, NULL);

  return 0;
}
