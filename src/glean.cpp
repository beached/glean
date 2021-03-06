// The MIT License (MIT)
//
// Copyright (c) 2016-2019 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <boost/program_options.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "daw/daw_graph_algorithm.h"
#include "daw/glean/glean_config.h"
#include "daw/glean/glean_file.h"
#include "daw/glean/glean_options.h"
#include "daw/glean/logging.h"
#include "daw/glean/utilities.h"

namespace {
	[[nodiscard]] daw::glean::glean_config setup_config( ) {
		auto config = daw::glean::get_config( );
		if( not exists( config.cache_folder ) ) {
			create_directory( config.cache_folder );
		}
		if( not exists( config.cache_folder ) or
		    not is_directory( config.cache_folder ) ) {
			std::stringstream ss{};
			ss << "Cache root (" << config.cache_folder
			   << ") does not exist or is not a directory";
			throw daw::glean::glean_exception( ss.str( ) );
		}
		return config;
	}
} // namespace

// Embed git version from define into binary.
// This is really cool when you have a working version
// but don't know what tag/commit it was from
extern "C" char const GIT_VERSION[];
#ifndef SOURCE_CONTROL_REVISION
#error "SOURCE_CONTROL_REVSION must be defined"
#endif
char const GIT_VERSION[] = SOURCE_CONTROL_REVISION;

int main( int argc, char **argv ) {
	auto const config = setup_config( );
	auto opts = daw::glean::glean_options( argc, argv );
	log_message << "glean cache: " << opts.glean_cache << '\n';
	log_message << "install prefix: " << opts.install_prefix << '\n';
	auto deps = daw::glean::process_config_file( "./glean.json", opts );

	switch( opts.output_type ) {
	case daw::glean::output_types::process:
		if( opts.build_type == daw::glean::build_types::all ) {
			opts.build_type = daw::glean::build_types::debug;
			daw::glean::process_deps( std::move( deps ), opts );
			opts.build_type = daw::glean::build_types::release;
			daw::glean::process_deps( std::move( deps ), opts );
		} else {
			daw::glean::process_deps( std::move( deps ), opts );
		}
		break;
	case daw::glean::output_types::cmake:
		// Output a CMake External project list with deps
		daw::glean::cmake_deps( std::move( deps ) );
		break;
	default:
		log_error << "Not implemented\n";
		std::abort( );
	}
	return EXIT_SUCCESS;
}
