// The MIT License (MIT)
//
// Copyright (c) 2016 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <boost/filesystem.hpp>
#include <boost/utility/string_ref.hpp>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <thread>

#include <daw/parse_template/daw_parse_template.h>

#include "glean_impl.h"
#include "glean_file.h"
#include "glean_file_parser.h"
#include "templates.h"

namespace daw {
	namespace glean {
		glean_exception::glean_exception( boost::string_ref msg ):
			std::runtime_error{ msg.data( ) } { }

		glean_exception::~glean_exception( ) { }
		namespace {
			bool is_update_needed( glean_item const & item, boost::filesystem::path const & prefix, boost::filesystem::path const & cache_folder ) {
				return true;
			}

			boost::filesystem::path cache_path( glean_item const & item, boost::filesystem::path cache_root ) {
				assert( exists( cache_root ) && is_directory( cache_root ) );
				assert( !item.project_name.empty( ) );
				cache_root /= item.project_name;
				if( item.branch && !item.branch->empty( ) ) {
					cache_root /= *item.branch;
				}
				return cache_root;
			}

			void verify_folder( boost::filesystem::path const & path ) {
				if( !exists( path ) ) {
					create_directories( path );
				}
				if( !exists( path ) || !is_directory( path ) ) {
					std::stringstream ss;
					ss << "Could not create folder (" << path << ") or is not a directory";
					throw glean_exception( ss.str( ) );
				}
			}

			void verify_file( boost::filesystem::path const & f ) {
				if( exists( f ) && !is_regular_file( f ) ) {
					std::stringstream ss;
					ss << "File already exists but isn't a file (" << f << ")";
					throw glean_exception( ss.str( ) );

				}
			}

			struct item_folders {
				boost::filesystem::path cache;
				boost::filesystem::path build;
				boost::filesystem::path src;
				boost::filesystem::path cmakelist_file;
			};	// item_folders

			struct change_directory {
				boost::filesystem::path old_path;
				change_directory( boost::filesystem::path const & new_path ):
					old_path{ boost::filesystem::current_path( ) } {

						boost::filesystem::current_path( new_path );
					}

				~change_directory( ) {
					if( exists( old_path) && is_directory( old_path ) ) {
						boost::filesystem::current_path( old_path );
					}
				}
				change_directory( change_directory && ) = default;
				change_directory & operator=( change_directory && ) = default;

				change_directory( change_directory const & ) = delete;
				change_directory & operator=( change_directory const & ) = delete;
			};	// change_directory

			item_folders create_cmakelist( glean_item const & item, boost::filesystem::path const & prefix, boost::filesystem::path const & cache_root ) {
				static auto const git_template_str = impl::get_git_template( );
				item_folders result;
				result.cache = cache_path( item, cache_root );
				verify_folder( result.cache );
				result.build = result.cache / "build";
				verify_folder( result.build );
				result.src = result.cache / "src";
				verify_folder( result.src );
				result.cmakelist_file = result.cache / "CMakeLists.txt";
				verify_file( result.cmakelist_file );
				auto git_template = daw::parse_template::create_parse_template( git_template_str );
				git_template.add_callback( "project_name", [&]( ) -> std::string { return item.project_name; } );
				git_template.add_callback( "git_repo", [&]( ) -> std::string { return *item.uri; } );
				git_template.add_callback( "source_directory", [&]( ) -> std::string { return result.src.native( ); } );
				if( item.branch && !item.branch->empty( ) ) {
					git_template.add_callback( "git_tag", [&]( ) -> std::string { return *item.branch; } );
				} else {
					git_template.add_callback( "git_tag", []( ) -> std::string { return "master"; } );

				}
				git_template.add_callback( "install_directory", [&]( ) -> std::string { return canonical( prefix ).native( ); } );
				try {
					std::ofstream out_file;
					out_file.open( result.cmakelist_file.native( ), std::ios::out | std::ios::trunc );
					if( !out_file ) {
						throw std::runtime_error( "Could not open file" );
					}
					git_template.process_template( out_file );
					out_file.close( );
				} catch( std::exception const & ex ) {
					std::stringstream ss;
					ss << "Could not write cmake file (" << result.cmakelist_file << "): " << ex.what( );
					throw glean_exception( ss.str( ) );
				}
				return result;
			}

			int build( item_folders const & proj, glean_item const & item ) {
				change_directory chd{ proj.build };
				{
					int result;
					if( EXIT_SUCCESS != (result = system( "cmake .." )) ) {
						return result;
					}
				}
				return system( "make" );
			}

			int install( item_folders const & proj, glean_item const & item ) {
				change_directory chd{ proj.build };
				return system( "make install" );
			}

			void process_item( glean_item const & item, boost::filesystem::path const & prefix, boost::filesystem::path const & cache_root ) {
				auto cml = create_cmakelist( item, prefix, cache_root );
				if( build( cml, item ) != EXIT_SUCCESS ) {
					return;
				}
				//install( cml, item );
			}
		}

		void process_file( boost::filesystem::path const & depend_file, boost::filesystem::path const & prefix, boost::filesystem::path const & cache_root ) {
			auto depends_obj = parse_cmakes_deps( depend_file );
			for( auto const & dependency : depends_obj.dependencies ) {
				std::cout << "Processing: " << dependency.project_name << '\n';
				try {
					process_item( dependency, prefix, cache_root );
				} catch( glean_exception const & ex ) {
					std::cerr << "Error processing: " << dependency.project_name << ":\n" << ex.what( ) << std::endl;
				}
			}
		}
	}	// namespace glean
}    // namespace daw